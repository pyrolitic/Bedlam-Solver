#ifndef PIECE_H
#define PIECE_H

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <limits>
#include <unordered_map>
#include <string>

#include "vec3.h"

#define axis_x 0
#define axis_y 1
#define axis_z 2

#define fully_symmetric 1
#define two_rotations 2
// 00 00 00
//============================================================================================
class Piece {
public:
	struct collisionResult{
		vec3i block;
		vec3f point;
		int side; //(axis << 1) | (right == 1)
	};

	std::unordered_map<vec3i, int> blocks;
	int copies;

	Piece(){}
	~Piece(){}

	int mass(){
		return blocks.size();
	}

	void insert(vec3i at){
		blocks[at] = 0;
	}

	bool query(vec3i at){
		return blocks.find(at) != blocks.end();
	}

	void remove(vec3i at){
		blocks.erase(at);
	}

	//checks for the closes collision between a ray (start and dir) and any of the blocks in this piece
	//does not change $result if no collision happens
	//O(n), TODO: make it better
	bool collisionCheck(vec3 start, vec3 dir, collisionResult& result){
		const int otherAxes[] = {1, 2, 0, 2, 0, 1}; //the axes inside the plane for each axis that's orthonormal to them

		vec3i collisionBlock;
		float lowestT = std::numeric_limits<float>::max();
		int collisionSide = 0;

		for (int axis = 0; axis < 3; axis++){ //YZ, XZ, XY planes
			for(auto pairs : blocks){
				const vec3i& b = pairs.first;
				int* blockData = (int*)(&b.x);

				//check either side of each axis
				for (int i = 0; i < 2; i++){
					int v = blockData[axis] + i;

					//distance to collision with plane (offset in the axis direction by $v units)
					float t = (v - start.data[axis]) / dir.data[axis];

					if (t > 0.0f && t != std::numeric_limits<float>::max()){ //sanity check
						float h = v - start.data[axis];
						vec3 p = start + dir * (h / dir.data[axis]);
						
						int a2 = otherAxes[axis * 2 + 0];
						int a3 = otherAxes[axis * 2 + 1];

						//check that the hit is within the plane
						if (p.data[a2] >= blockData[a2] && p.data[a2] < blockData[a2] + 1 && p.data[a3] >= blockData[a3] && p.data[a3] < blockData[a3] + 1){
							//hit
							if (t < lowestT){
								collisionBlock = b;
								lowestT = t;
								collisionSide = axis * 2 + i;
							}
						}
					}
				}
			}
		}

		if (lowestT < std::numeric_limits<float>::max()){
			result.block = collisionBlock;
			result.side = collisionSide;
			result.point = start + dir * lowestT;
			return true;
		}

		else return false;
	}
};

#endif
