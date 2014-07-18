#ifndef SOLVER_H
#define SOLVER_H

#include <cstring>

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

			//find the limits
			vec3i lowest(std::numeric_limits<int>::max());
			vec3i highest(std::numeric_limits<int>::min());

			for (auto pairs : from.blocks){
				const vec3i& v = pairs.first;
				lowest = minVec(lowest, v);
				highest = maxVec(highest, v);
			}

			size = highest - lowest;
			data = (uint8_t*) malloc(size.x * size.y * size.z * sizeof(uint8_t)); //maybe change this to std::bitset
			memset(data, 0, size.x * size.y * size.z * sizeof(uint8_t));

			for (auto pairs : from.blocks){
				vec3i p = pairs.first - lowest;
				data[LOOKUP(p, size)] = 1;
			}

			orientation = 0; //original
		}

		//these better be optimized a bit when fully expanded
		#define ROTATED_ONCE_X(p) vec3i(p.x, p.z, size.y - p.y - 1) //[x, y, z] => [ x,  z, -y]
		#define ROTATED_ONCE_Y(p) vec3i(size.z - p.z - 1, p.y, p.x) //[x, y, z] => [-z,  y,  x]
		#define ROTATED_ONCE_Z(p) vec3i(size.y - p.y - 1, p.x, p.z) //[x, y, z] => [-y,  x,  z]

		#define ROTATED_TWICE_X(p) ROTATED_ONCE_X(ROTATED_ONCE_X(p))
		#define ROTATED_TWICE_Y(p) ROTATED_ONCE_Y(ROTATED_ONCE_Y(p))
		#define ROTATED_TWICE_Z(p) ROTATED_ONCE_Y(ROTATED_ONCE_Y(p))

		#define ROTATED_THRICE_X(p) ROTATED_ONCE_X(ROTATED_TWICE_X(p))
		#define ROTATED_THRICE_Y(p) ROTATED_ONCE_Z(ROTATED_TWICE_Y(p))
		#define ROTATED_THRICE_Z(p) ROTATED_ONCE_Y(ROTATED_TWICE_Z(p))

		#define ORIENTATIONS 24

		DensePiece(const DensePiece& from, int oriented){
			data = (uint8_t*) malloc(from.size.x * from.size.y * from.size.z * sizeof(uint8_t));
			assert(oriented >= 0 and oriented < 24);
			orientation = oriented;

			//set the right size;
			//this is the same as the rotation macros, but without invertions
			switch(oriented / 4){
				case 0: size = from.size; break;
				case 1: size = vec3i(from.size.x, from.size.z, from.size.y); break;
				case 2: size = vec3i(from.size.z, from.size.y, from.size.x); break;
				case 3: size = vec3i(from.size.x, from.size.z, from.size.y); break;
				case 4: size = vec3i(from.size.z, from.size.y, from.size.x); break;
				case 5: size = from.size; break;
			}

			//and rotate the size about itself on the z axis
			switch(oriented % 4){
				case 0: break; //no rotation
				case 1: size = vec3i(size.y, size.x, size.z); break;
				case 2: size = vec3i(size.x, size.y, size.z); break;
				case 3: size = vec3i(size.y, size.x, size.z); break;
			}

			for (int z = 0; z < from.size.z; z++){
				for (int y = 0; y < from.size.y; y++){
					for (int x = 0; x < from.size.x; x++){
						vec3i src(x, y, z);
						vec3i dst;

						switch(oriented / 4){
							case 0: dst = src; break;
							case 1: dst = ROTATED_ONCE_X(src); break;
							case 2: dst = ROTATED_ONCE_Y(src); break;
							case 3: dst = ROTATED_THRICE_X(src); break;
							case 4: dst = ROTATED_THRICE_Y(src); break;
							case 5: dst = ROTATED_TWICE_X(src); break;
						}

						switch(oriented % 4){
							case 0: break; //no rotation
							case 1: dst = ROTATED_ONCE_Z(dst); break;
							case 2: dst = ROTATED_TWICE_Z(dst); break;
							case 3: dst = ROTATED_THRICE_Z(dst); break;
						}

						data[LOOKUP(dst, size)] = from.data[LOOKUP(src, from.size)];
					}
				}
			}
		}

		bool operator == (const DensePiece& other){
			if (size == other.size){
				return memcmp(data, other.data, size.x * size.y * size.z * sizeof(uint8_t)) == 0;
			}
			return false;
		}

		~DensePiece(){
			free(data);
		}

		uint8_t query(vec3i at){
			return data[LOOKUP(at, size)];
		}

		vec3i size;
		uint8_t* data;
		int orientation;
	};

public:
	//a solution is an array of these, one for every piece copy
	struct solutionPiece{
		int pieceId;
		int copyId;
		int orientationId;
	};

	//just copy the pieces, let the worker thread do the real work
	Solver(vec3i worldSize, const std::list<Piece>& sparsePieces){
		assert(worldSize > vec3i(0, 0, 0));
		dimensions = worldSize;
		nodes = nullptr;
		nodeAmount = 0;
		nodeIt = 0;

		spaces = dimensions.x * dimensions.y * dimensions.z;

		for (auto piece : sparsePieces){
			pieceImages.emplace_back(std::list<DensePiece>());
			pieceImages.back().emplace_back(piece);
			piecesCopies.emplace_back(piece.copies);
			piecesMass.emplace_back(piece.mass());
		}

		//start the thread
		solverThread = std::thread(threadEntry, this);
	}

	bool isWorkerAlive(){
		return workerAlive;
	}

	//must not be called until the thread is dead
	~Solver() {
		assert(!workerAlive);
	}

	//stop the solver thread
	void kill(){
		dieCommand = true;
	}

	bool isDead(){
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

	std::vector<std::list<DensePiece>> pieceImages; //original and every unique rotation image
	std::vector<int> piecesCopies; //number of copies
	std::vector<int> piecesMass; //number of blocks per piece
	vec3i dimensions; //world size

	//a 1 in the matrix
	struct header_node;
	struct node {
		node* left;
		node* right;
		node* up;
		node* down;

		header_node* colHeader;
	};

	#define SOLVER_SPACE_NODE_COPY_VALUE -1
	struct header_node: public node {
		int size; //height of the column
		int symbol; //1d space index or piece index
		int copy; //copy index or SOLVER_SPACE_NODE_COPY_VALUE if symbol is a space index
		int orientation; //orientation index
	};

	void cover(header_node* column) {
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
	void uncover(header_node* column) {
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

	int space(int x, int y, int z) const {
		return (x + y * dimensions.x) + z * dimensions.x * dimensions.y; //consistent with DensePiece addressing
	}

	vec3i location(int cell) const {
		vec3i p;

		int slice = cell % dimensions.y * dimensions.z;
		p.x = slice % dimensions.x;
		p.y = slice / dimensions.x;
		p.z = cell / (dimensions.x * dimensions.z);

		return p;
	}

	//this is a stack of the chosen row to cover at each level of search(k)
	//all the rows up to the last level can be accessed instantly when it's time to print a solution
	node** partialSolutionRows;

	int spaces;
	int cols;
	int pieceInstances;

	header_node* header;
	header_node root;

	int nodeAmount; //amount of nodes, not matrix size
	int nodeIt; //index of last allocated node
	node* nodes; //storage

	node* newNode() {
		assert(nodeIt < nodeAmount);
		return &(nodes[nodeIt++]);
	}

	//costly preparation, calls the initial search
	void solve();

	//where all the magic happens
	void search(int k);

	static void threadEntry(Solver* solver){
		solver->workerAlive = true;
		solver->solve();
		solver->workerAlive = false;
	}
};

#endif
