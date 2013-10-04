#ifndef PIECE_H
#define PIECE_H

#include <cstdlib>
#include <cstring>

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
class CPiece {
public:
	int width;
	int height;
	int depth;

	CPiece(int w, int h, int d);
	CPiece(int w, int h, int d, const char* data);
	~CPiece();

	bool& data(int x, int y, int z);

	CPiece* copy();
	CPiece* rotated(int axis, int turns);

	int volume();

	bool axialSymmetry(int axis, int turns);

	void checkAxialSymmetry();

	bool operator ==(CPiece& other) {
		if (width != other.width or height != other.height or depth != other.depth) return false;
		for (int i = 0; i < width * height * depth; i++)
			if (solid[i] != (&other.data(0, 0, 0))[i]) return false;

		return true;
	}

	void print() {
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
	}

private:
	bool* solid;
	char symmetry;
};
//============================================================================================
inline CPiece::CPiece(int w, int h, int d) :
		width(w), height(h), depth(d), symmetry(0) {
	solid = (bool*) malloc(width * height * depth * sizeof(bool));
}

inline CPiece::CPiece(int w, int h, int d, const char* data) :
		width(w), height(h), depth(d) {

	assert(strlen(data) == w * h * d);

	solid = (bool*) malloc(width * height * depth * sizeof(bool));

	for (int i = 0; i < width * height * depth; i++)
		solid[i] = (data[i] == ' ') ? false : true;

	//checkAxialSymmetry();
}

inline CPiece::~CPiece() {
	free(solid);
}
//============================================================================================
inline bool& CPiece::data(int x, int y, int z) {
	return solid[(x + y * width) + z * width * height];
}

inline bool CPiece::axialSymmetry(int axis, int turns) {
	assert(axis >= 0 and axis < 3);
	turns = abs(turns);
	assert(turns < 4);

	//has to be fully symmetric
	if (turns == 1 or turns == 3) return symmetry & (1 << axis);
	else if (turns == 2) return symmetry & (1 << (axis + 1));
	else return true; //turns == 0
}

inline int CPiece::volume() {
	int v = 0;

	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)
			for (int z = 0; z < depth; z++)
				if (data(x, y, z)) v++;

	return v;
}
//============================================================================================
inline CPiece* CPiece::copy() {
	CPiece* n = new CPiece(width, height, depth);

	memcpy(&(n->data(0, 0, 0)), solid, width * height * depth * sizeof(bool));
	//n->checkAxialSymmetry();
	return n;
}

inline CPiece* CPiece::rotated(int axis, int turns) {
	assert(turns > 0);
	assert(turns < 4);

	bool dimSwap = turns & 1;

	CPiece* n;

	switch (axis) {
	case axis_x:
		n = dimSwap ? new CPiece(width, depth, height) : new CPiece(width, height, depth);

		for (int z = 0; z < depth; z++)
			for (int y = 0; y < height; y++)
				for (int x = 0; x < width; x++) {
					if (turns == 1) n->data(x, z, height - y - 1) = data(x, y, z);
					else if (turns == 2) n->data(x, height - y - 1, depth - z - 1) = data(x, y, z);
					else if (turns == 3) n->data(x, depth - z - 1, y) = data(x, y, z);
				}
		break;

	case axis_y:
		n = dimSwap ? new CPiece(depth, height, width) : new CPiece(width, height, depth);

		for (int z = 0; z < depth; z++)
			for (int y = 0; y < height; y++)
				for (int x = 0; x < width; x++) {
					if (turns == 1) n->data(depth - z - 1, y, x) = data(x, y, z);
					else if (turns == 2) n->data(width - x - 1, y, depth - z - 1) = data(x, y, z);
					else if (turns == 3) n->data(z, y, width - x - 1) = data(x, y, z);
				}
		break;

	case axis_z:
		n = dimSwap ? new CPiece(height, width, depth) : new CPiece(width, height, depth);

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
}
//============================================================================================
inline void CPiece::checkAxialSymmetry() {

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
			for (int j = 0; j < b; j++)
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
//============================================================================================
#endif /* PIECE_H */
