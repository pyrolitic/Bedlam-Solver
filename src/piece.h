#ifndef PIECE_H
#define PIECE_H

#include <cstdlib>
#include <cstring>

#include <list>
#include <string>

#include <iostream>
#include <cassert>

#define axis_x 0
#define axis_y 1
#define axis_z 2

#define fully_symmetric 1
#define two_rotations 2
// 00 00 00
//============================================================================================
class Piece {
public:
	struct block{
		int x, y, z;
		block(int x, int y, int z):
			x(x), y(y), z(z) {}

		block() {
			x = y = z = 0; 
		}
	};

	int width;
	int height;
	int depth;
	std::list<block> blocks;

	Piece(); //empty piece
	Piece(int w, int h, int d, const char* data); //piece from (layer then row major) dense matrix
	~Piece();

	//bool& data(int x, int y, int z);

	Piece* copy();
	//Piece* rotated(int axis, int turns);
	int mass(); //the number of solid blocks

	void insert(int x, int y, int z){
		blocks.push_back(block(x, y, z));
	}

	//TODO: fix; find a way to do this in at most O(n)
	bool operator == (Piece& other) {
		if (width != other.width or height != other.height or depth != other.depth) return false;

		/*for (blocks::iterator it1 = blocks.begin(), other.blocks::iterator it2 = other.begin(); it1 != blocks.end() && it2 != other.blocks.end(); it1++, it2++){
			block& a = *it1;
			block& b = *it2;
			if (a.x != b.x || a.y != b.y || a.z != b.z) return false;
		}

		return true;*/

		return false;
	}

	/*void print() {
		std::cout << "-----" << std::endl;
		for (int z = 0; z < depth; z++) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					if (data(x, y, z)) std::cout << 'x';
					else std::cout << ' ';
				}
				std::cout << std::endl;
			}

			std::cout << "-----" << std::endl;
		}
	}*/

	
};
//============================================================================================
inline Piece::Piece() {
	width = height = depth = 0;
}

inline Piece::Piece(int w, int h, int d, const char* data) :
		width(w), height(h), depth(d) {

	assert(strlen(data) == w * h * d);

	for (int z = 0; z < depth; z++) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (data[(x + y * width) + z * width * height] != ' '){
					blocks.push_back(block(x, y, z));
				}
			}
		}
	}
}

inline Piece::~Piece() {
}
//============================================================================================
inline int Piece::mass() {
	return blocks.size();
}
//============================================================================================
inline Piece* Piece::copy() {
	Piece* n = new Piece();
	n->blocks.insert(n->blocks.begin(), blocks.begin(), blocks.end());
	return n;
}
/*
inline Piece* Piece::rotated(int axis, int turns) {
	assert(turns > 0);
	assert(turns < 4);

	bool dimSwap = turns & 1;
	Piece* n;

	switch (axis) {
	case axis_x:
		n = dimSwap ? new Piece(width, depth, height) : new Piece(width, height, depth);

		for (int z = 0; z < depth; z++)
			for (int y = 0; y < height; y++)
				for (int x = 0; x < width; x++) {
					if (turns == 1) n->data(x, z, height - y - 1) = data(x, y, z);
					else if (turns == 2) n->data(x, height - y - 1, depth - z - 1) = data(x, y, z);
					else if (turns == 3) n->data(x, depth - z - 1, y) = data(x, y, z);
				}
		break;

	case axis_y:
		n = dimSwap ? new Piece(depth, height, width) : new Piece(width, height, depth);

		for (int z = 0; z < depth; z++)
			for (int y = 0; y < height; y++)
				for (int x = 0; x < width; x++) {
					if (turns == 1) n->data(depth - z - 1, y, x) = data(x, y, z);
					else if (turns == 2) n->data(width - x - 1, y, depth - z - 1) = data(x, y, z);
					else if (turns == 3) n->data(z, y, width - x - 1) = data(x, y, z);
				}
		break;

	case axis_z:
		n = dimSwap ? new Piece(height, width, depth) : new Piece(width, height, depth);

		for (int z = 0; z < depth; z++)
			for (int y = 0; y < height; y++)
				for (int x = 0; x < width; x++) {
					if (turns == 1) n->data(height - y - 1, x, z) = data(x, y, z);
					else if (turns == 2) n->data(width - x - 1, height - y - 1, z) = data(x, y, z);
					else if (turns == 3) n->data(y, width - x - 1, z) = data(x, y, z);
				}
		break;
	}

	return n;
}*/
//============================================================================================
/*inline void Piece::checkAxialSymmetry() {

	symmetry = 0x3F; //assume all is true

	for (int axis = axis_x; axis <= axis_z; axis++) {

		int a, b, c;
		if (axis == axis_x) {
			a = width;
			b = height;
			c = depth;
		}

		if (axis == axis_y) {
			a = height;
			b = depth;
			c = width;
		}

		if (axis == axis_z) {
			a = depth;
			b = height;
			c = width;
		}

		for (int i = 0; i < a; i++) {
			bool last = (axis == axis_x) ? data(i, 0, 0) : (axis == axis_y) ? data(0, i, 0) : data(0, 0, i); //the first

			//slice by slice
			for (int j = 0; j < b; j++){
				for (int k = 0; k < c; k++) {
					bool& cell = (axis == axis_x) ? data(i, j, k) : (axis == axis_y) ? data(k, i, j) : data(k, j, i);

					if (cell != last) symmetry &= ~(1 << (axis * 2)); //not turn symmetric, but there's still hope for flip symmetry

					if (cell != data(i, height - i - 1, depth - k - 1)) {
						symmetry &= ~(2 << (axis * 2));

						//this implies not fully symmetric either, so skip to the end
						k = c;
						j = b;
						i = a;
					}

					last = cell;
				}
			}
		}
	}
}*/

//============================================================================================
#endif /* PIECE_H */
