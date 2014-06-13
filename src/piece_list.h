#ifndef PIECE_H
#define PIECE_H

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <limits>
#include <list>
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
		std::list<vec3i>::iterator blockIt;
		vec3 point;
		int side; //(axis << 1) | (right == 1)
	};

	std::list<vec3i> blocks;

	Piece(){}
	~Piece(){}

	int mass(){
		return blocks.size();
	}

	void insert(int x, int y, int z){
		blocks.emplace(blocks.begin(), x, y, z);
	}

	//checks for the closes collision between a ray (start and dir) and any of the blocks in this piece
	//does not change $result if no collision happens
	//O(n), TODO: make it better
	bool collisionCheck(vec3 start, vec3 dir, collisionResult& result){
		const int otherAxes[] = {1, 2, 0, 2, 0, 1}; //the axes inside the plane for each axis that's orthonormal to them

		auto blockIt = blocks.end();
		float lowestT = std::numeric_limits<float>::max();
		int collisionSide = 0;

		for (int axis = 0; axis < 3; axis++){ //YZ, XZ, XY planes
			for(auto it = blocks.begin(); it != blocks.end(); it++){
				vec3i& b = *it;
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
								blockIt = it;
								lowestT = t;
								collisionSide = axis * 2 + i;
							}
						}
					}
				}
			}
		}

		if (blockIt != blocks.end()){
			result.blockIt = blockIt;
			result.side = collisionSide;
			result.point = start + dir * lowestT;
			return true;
		}

		else return false;
	}
};

#endif
