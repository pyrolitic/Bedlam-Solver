#ifndef SOLVER_H
#define SOLVER_H

#include <cstring>

#include <numeric>
#include <vector>
#include <list>
#include <limits>
#include <string>
#include <mutex>
#include <thread>

#include "piece.h"

//Spawning a solver comes with the assumption that the sum of the number of 
//blocks in each piece (copies considered) is equal to the number of spaces in the world

class Solver {
private:
	class DensePiece{
	public:
		#define LOOKUP(p, s) ((p.x + p.y * s.x) + p.z * s.x * s.y)

		DensePiece(const Piece& from){
			assert(from.blocks.size() > 0);

			size = from.getSize();
			data = (uint8_t*) malloc(size.x * size.y * size.z * sizeof(uint8_t)); //maybe change this to std::bitset
			memset(data, 0, size.x * size.y * size.z * sizeof(uint8_t));

			for (auto& pairs : from.blocks){
				ivec3 p = pairs.first;
				data[LOOKUP(p, size)] = 1;
			}
			orientation = from.orientationId;
		}

		DensePiece(const DensePiece& other) = delete;
		DensePiece& operator =(const DensePiece& other) = delete;

		~DensePiece(){
			free(data);
		}

		bool operator == (const DensePiece& other) const{
			if (size == other.size){
				return memcmp(data, other.data, size.x * size.y * size.z) == 0;
			}
			return false;
		}

		uint8_t query(ivec3 at) const {
			return data[LOOKUP(at, size)];
		}

		ivec3 size;
		uint8_t* data;
		int orientation;
	};

public:
	//a solution is an array of these, one for every piece copy
	struct solutionPiece{
		int pieceId;
		int copyId;
		int orientationId;
		ivec3 position;
	};

	//constructor and destructor(delete) should only be called from the UI thread

	//just copy the pieces, let the worker thread do the real work
	Solver(ivec3 worldSize, const std::list<Piece>& sparsePieces, const std::list<int>& pieceCopies){
		assert(sparsePieces.size() == pieceCopies.size());
		assert(worldSize > ivec3(0, 0, 0));
		dimensions = worldSize;
		nodes = nullptr;
		nodeAmount = 0;
		nodeIt = 0;

		spaces = dimensions.x * dimensions.y * dimensions.z;

		for (const auto& piece : sparsePieces){
			assert((piece.getSize() <= worldSize) and "solver (somehow) got piece that is larger than the world");
			originals.emplace_back(piece);
			pieceImages.emplace_back();
			piecesMass.emplace_back(piece.mass());
		}

		assert(std::accumulate(piecesMass.begin(), piecesMass.end(), 0) == spaces);

		for (const int& pieceCopy : pieceCopies){
			assert((pieceCopy > 0) and "solver somehow got a piece with < 1 instances");
			piecesCopies.emplace_back(pieceCopy);
		}

		dieCommand = false;
		workerAlive = false;

		findHeaderNodeGoes = 0;
		findHeaderNodeOps = 0;

		//start the thread
		workerAlive = true;
		solverThread = std::thread(threadEntry, this);
		solverThread.detach(); //won't be joining it
	}

	//must not be called until the thread is dead
	~Solver() {
		assert(!workerAlive);
	}

	//stop the solver thread
	void kill(){
		dieCommand = true;
	}

	bool isWorkerAlive() const{
		return workerAlive;
	}

	//fills $storage with the last solution that wasn't taken yet, then deletes the solution from the solver's queue
	//returns nullptr if there is no solution left
	//responsibility of the caller to free the buffer returned
	solutionPiece* getSolution(){
		solutionPiece* sol = nullptr;
		accessLock.lock();
		if (solutions.size() > 0){
			sol = solutions.front();
			solutions.pop_front();
		}
		accessLock.unlock();
		return sol;
	}

private:
	std::list<solutionPiece*> solutions; //queue of malloc'd strings of $spaces size
	std::thread solverThread;
	std::mutex accessLock;

	bool workerAlive; //read/write for worker, read only for master
	bool dieCommand; //read-only for worker, read/write for master

	std::vector<Piece> originals;
	std::vector<std::list<DensePiece>> pieceImages; //original and every unique rotation image
	std::vector<int> piecesCopies; //number of copies
	std::vector<int> piecesMass; //number of blocks per piece
	ivec3 dimensions; //world size

	//a 1 in the matrix, a material node
	struct node {
		node* left;
		node* right;
		node* up;
		node* down;
	};

	//top row nodes
	struct headerNode: public node {
		int size; //height of the column

		//union{ //debug
			struct{
				//only applies to material columns
				int spaceId; //z * W*H + y * W + x
			};

			struct{
				//only apply to marker columns
				int pieceId;
				int copyId;
			};
		//};
	};

	//marker nodes at the end of each row
	struct refNode : public node{
		int orientation;
		ivec3 location; //bottom-left-back corner
	};

	void cover(headerNode* column) {
		column->right->left = column->left;
		column->left->right = column->right;

		//traverse the column all the way down
		for (node* row = column->down; row != column; row = row->down) {
			//traverse the row to the right and loop to the start
			for (node* col = row->right; col != row; col = col->right) {
				col->up->down = col->down;
				col->down->up = col->up;

				//row->colHeader->size--; //there is one less node in the column
			}
		}
	}

	//reverse of the above
	void uncover(headerNode* column) {
		for (node* col = column->up; col != column; col = col->up) {
			for (node* row = col->left; row != col; row = row->left) {
				//row->colHeader->size++;

				row->up->down = row;
				row->down->up = row;
			}
		}

		column->right->left = column;
		column->left->right = column;
	}

	bool isHeaderNode(node* n){
		assert(n);
		assert(headers);
		return (n >= headers and n < headers + cols);
	}

	bool isRefNode(node* n){
		assert(n);
		assert(refNodes);
		return (n >= refNodes and n < refNodes + rows);
	}

	//warning: assumes there is a link; can get stuck
	//debug
	int findHeaderNodeGoes;
	int findHeaderNodeOps;
	headerNode* findHeaderNode(node* n){
		while(!isHeaderNode(n)){
			n = n->up;
			findHeaderNodeOps++;
		}
		findHeaderNodeGoes++;
		return (headerNode*) n;
	}

	int space(int x, int y, int z) const {
		return (x + y * dimensions.x) + z * dimensions.x * dimensions.y; //consistent with DensePiece addressing
	}

	ivec3 location(int cell) const {
		ivec3 p;

		int slice = cell % dimensions.y * dimensions.z;
		p.x = slice % dimensions.x;
		p.y = slice / dimensions.x;
		p.z = cell / (dimensions.x * dimensions.z);

		return p;
	}

	//this is a stack of the chosen row to cover at each level of search(k)
	//all the rows up to the last level can be accessed instantly when it's time to print a solution
	node** partialSolutionRows;

	headerNode root;

	int spaces; //W * H * Z
	int cols; //spaces + pieceInstances
	headerNode* headers; //storage for header nodes

	int pieceInstances; //sum of piece copies
	int rows; //this is the size of refNodes
	refNode* refNodes; //storage for marker nodes

	int nodeAmount; //amount of nodes, not matrix size
	int nodeIt; //index of last allocated node
	node* nodes; //storage for material nodes

	int solutionsFound;

	node* newNode() {
		assert(nodeIt < nodeAmount);
		return &(nodes[nodeIt++]);
	}

	//costly preparation, calls the initial search
	void solve();

	//where all the magic happens
	void search(int k);

public: //debug
	static void threadEntry(Solver* solver){
		printf("started solver thread\n");
		solver->solve();
		printf("solver thread done\n");
		solver->workerAlive = false; //last thing the thread does, 
		//since the other thread will delete this thread as soon as it notices this
	}
};

#endif
