#ifndef SOLVER_H
#define SOLVER_H

#include "piece.h"
#include "timer.h"

#include <vector>
#include <list>
#include <limits>

#include <iostream>
#include <string>

#define NON_FLAG_NODE_NAME '\0'

class Solver {
private:
	class DensePiece{
	public:
		#define LOOKUP(p) ((p.x + p.y * size.x) + p.z * size.x * size.y)

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
			data = (bool*)malloc(size.x * size.y * size.z * sizeof(bool)); //maybe change this to std::bitset
			memset(data, 0, size.x * size.y * size.z * sizeof(bool));

			for (auto pairs : from.blocks){
				vec3i p = pairs.first - lowest;
				data[LOOKUP(p)] = true;
			}
		}

		~DensePiece(){
			free(data);
		}

		#define ROTATED_ONCE_X(p) vec3i(p.x, p.z, size.y - p.y - 1)
		#define ROTATED_ONCE_Y(p) vec3i(size.z - p.z - 1, p.y, p.x)
		#define ROTATED_ONCE_Z(p) vec3i(p.z, p.y, size.x - p.x - 1)

		#define ROATE_TWICE_X(p) vec3i(p.x, size.y - p.y - 1, size.z - p.z - 1)
		#define ROATE_TWICE_Y(p) vec3i(size.x - p.x - 1, p.y, size.z - p.z - 1)
		#define ROATE_TWICE_Z(p) vec3i(size.x - p.x - 1, size.y - p.y - 1, p.z)

		#define ROTATE_THRICE_X(p) vec3i(p.x, size.z - p.z - 1, p.y)
		#define ROTATE_THRICE_Y(p) vec3i(p.z, p.y, size.x - p.x - 1)
		#define ROTATE_THRICE_Z(p) vec3i(p.y, size.x - p.x - 1, p.z)


		bool query(vec3i at, int mode){
			switch(mode){
				case 0: return data[LOOKUP(at)];
				case 1: return data[LOOKUP(ROTATED_ONCE_X(at))];
				/*case 2: return data[(p.x + p.y * size.x) + p.z * size.x * size.y];
				case 3: return data[(p.x + p.y * size.x) + p.z * size.x * size.y];
				case 4: return data[(p.x + p.y * size.x) + p.z * size.x * size.y];
				case 5: return data[(p.x + p.y * size.x) + p.z * size.x * size.y];
				case 6: return data[(p.x + p.y * size.x) + p.z * size.x * size.y];
				case 7: return data[(p.x + p.y * size.x) + p.z * size.x * size.y];*/
			}
			assert(false);
			return false;
		}

	private:
		vec3i size;
		bool* data;
	};
public:
	struct solution{
		struct piecePosition{
			int px, py, pz;
			int side;
		};

		piecePosition* positions;
	};

	Solver(vec3i worldSize, const std::list<Piece>& sparsePieces){
		assert(worldSize > vec3i(0, 0, 0));
		dimensions = worldSize;
		nodes = NULL;
		nodeAmount = 0;
		nodeIt = 0;

		spaces = dimensions.x * dimensions.y * dimensions.z;
		currentSolution = (char*) malloc(spaces * sizeof(char));
	}

	~Solver() {
		for (std::vector<std::list<Piece*> >::iterator it = pieces.begin(); it != pieces.end(); it++) {
			for (std::list<Piece*>::iterator it2 = it->begin(); it2 != it->end(); it2++) {
				if (*it2) delete *it2;
			}
		}

		free(currentSolution);
	}

	//void addPiece(int w, int h, int d, const char* structure, char alias);
	bool solve(int maxSolutions);

private:
	std::vector<std::list<Piece*> > pieces; //pieces with all their images
	vec3i dimensions;

	int maxSolutions;
	char* currentSolution;
	std::vector<std::string> solutions;

	struct header_node;

	//a 1 in the matrix
	struct node {
		node* left;
		node* right;
		node* up;
		node* down;

		header_node* colHeader;
	};

	struct header_node: public node {
		int size;
		short id;

		bool endCol;
		char name;
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

	header_node* header;
	header_node root;

	int nodeAmount; //amount of nodes, not matrix size
	int nodeIt; //index of last allocated node
	node* nodes; //storage

	node* newNode() {
		assert(nodeIt < nodeAmount);
		return &(nodes[nodeIt++]);
	}

	//where all the magic happens
	void search(int k);

	CTimer timer;
};

#endif
