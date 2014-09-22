#ifndef PIECE_H
#define PIECE_H

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <limits>
#include <unordered_map>
#include <string>

#include "maths/vec.h"
#include "maths/quat.h"

class Piece {
public:
	struct collisionResult{
		ivec3 block;
		vec3 point;
		int side; //(axis << 1) | (right == 1)
	};

	Piece(){
		orientationId = 0; //original
	}

	//these better be optimized a bit when fully expanded
	#define ROTATED_ONCE_X(p, size) ivec3(p.x, p.z, size.y - (p.y) - 1) //[x, y, z] => [ x,  z, -y]
	#define ROTATED_ONCE_Y(p, size) ivec3(size.z - (p.z) - 1, p.y, p.x) //[x, y, z] => [-z,  y,  x]
	#define ROTATED_ONCE_Z(p, size) ivec3(size.y - (p.y) - 1, p.x, p.z) //[x, y, z] => [-y,  x,  z]

	#define ROTATED_TWICE_X(p, size) ROTATED_ONCE_X(ROTATED_ONCE_X(p, size), ivec3(size.x, size.z, size.y))
	#define ROTATED_TWICE_Y(p, size) ROTATED_ONCE_Y(ROTATED_ONCE_Y(p, size), ivec3(size.z, size.y, size.x))
	#define ROTATED_TWICE_Z(p, size) ROTATED_ONCE_Z(ROTATED_ONCE_Z(p, size), ivec3(size.y, size.x, size.z))

	#define ROTATED_THRICE_X(p, size) ROTATED_ONCE_X(ROTATED_TWICE_X(p, size), size)
	#define ROTATED_THRICE_Y(p, size) ROTATED_ONCE_Y(ROTATED_TWICE_Y(p, size), size)
	#define ROTATED_THRICE_Z(p, size) ROTATED_ONCE_Z(ROTATED_TWICE_Z(p, size), size)

	#define ORIENTATIONS 24 // 6 * 4

	//rotating constructor; it is assumed that the $from piece is already offset-cancelled
	Piece(const Piece& from, int oriented){
		assert(oriented >= 0 and oriented < ORIENTATIONS);
		orientationId = oriented;

		ivec3 fromSize = from.getSize();

		//set the right size;
		//this is the same as the rotation macros, but without invertions
		ivec3 firstSize;
		switch(oriented / 4){
			case 0: firstSize = fromSize; break; //no rotation, front
			case 1: firstSize = ivec3(fromSize.x, fromSize.z, fromSize.y); break; //X, top
			case 2: firstSize = ivec3(fromSize.z, fromSize.y, fromSize.x); break; //Y, right
			case 3: firstSize = ivec3(fromSize.x, fromSize.z, fromSize.y); break; //X, bottom
			case 4: firstSize = ivec3(fromSize.z, fromSize.y, fromSize.x); break; //Y, left
			case 5: firstSize = fromSize; break; //X, back
		}

		//printf("non-z rotation to size %d %d %d\n", firstSize.x, firstSize.y, firstSize.z);

		/*//and rotate the size about itself on the z axis
		switch(oriented % 4){
			case 0: size = firstSize; break; //no rotation
			case 1: size = ivec3(firstSize.y, firstSize.x, firstSize.z); break; //once counterclockwise
			case 2: size = ivec3(firstSize.x, firstSize.y, firstSize.z); break; //twice counterclockwise
			case 3: size = ivec3(firstSize.y, firstSize.x, firstSize.z); break; //thrice counterclockwise
		}*/

		//printf("z rotation to size %d %d %d\n", size.x, size.y, size.z);

		for (int z = 0; z < fromSize.z; z++){
			for (int y = 0; y < fromSize.y; y++){
				for (int x = 0; x < fromSize.x; x++){
					ivec3 src(x, y, z);
					ivec3 dst;

					//X or Y axis rotations
					switch(oriented / 4){
						case 0: dst = src; break;
						case 1: dst = ROTATED_ONCE_X(src, fromSize); break;
						case 2: dst = ROTATED_ONCE_Y(src, fromSize); break;
						case 3: dst = ROTATED_THRICE_X(src, fromSize); break;
						case 4: dst = ROTATED_THRICE_Y(src, fromSize); break;
						case 5: dst = ROTATED_TWICE_X(src, fromSize); break;
					}

					//Z axis rotations
					switch(oriented % 4){
						case 0: break; //no rotation
						case 1: dst = ROTATED_ONCE_Z(dst, firstSize); break;
						case 2: dst = ROTATED_TWICE_Z(dst, firstSize); break;
						case 3: dst = ROTATED_THRICE_Z(dst, firstSize); break;
					}

					//printf("get from %d %d %d, put at %d %d %d\n", src.x, src.y, src.z, dst.x, dst.y, dst.z);
					if (from.query(src)){
						insert(dst);
					}
				}
			}
		}
	}

	~Piece(){}

	const std::unordered_map<ivec3, int>& getBlocks() const{
		return blocks;
	}

	int mass() const{
		return blocks.size();
	}

	void insert(const ivec3& at){
		blocks[at] = 0;
	}

	bool query(const ivec3& at) const{
		return blocks.find(at) != blocks.end();
	}

	void remove(const ivec3& at){
		blocks.erase(at);
	}

	//make it so that its bottom-left-back bound is at (0, 0, 0)
	//the top-right-front bound is returned
	ivec3 cancelOffset(){
		ivec3 end(0, 0, 0);
		if (!blocks.empty()){
			ivec3 start = ivec3(std::numeric_limits<int>::max());
			end = ivec3(std::numeric_limits<int>::min());

			for (const auto& pairs : blocks){
				const ivec3& v = pairs.first;
				start = minVec(start, v);
				end = maxVec(end, v);
			}

			decltype(blocks) copy;
			for (const auto& pairs : blocks){
				const ivec3& v = pairs.first;
				copy[v - start] = 0;
			}

			blocks = copy;

			end -= start;
			end += ivec3(1);
		}
		return end;
	}

	ivec3 getSize() const {
		ivec3 end(0, 0, 0);
		if (!blocks.empty()){
			end = ivec3(std::numeric_limits<int>::min());

			for (auto& pairs : blocks){
				const ivec3& v = pairs.first;
				end = maxVec(end, v);
			}

			end += ivec3(1);
		}
		return end;
	}

	//This solves the problem that once a solution is found, the actual position of each individual block is not directly known.
	//Apply to a mesh of the original piece (located at the origin, going positive on every axis), as pv = 2nd * rot * 1st * v
	quat getRotationFromOriginal() const{
		quat rotation;

		switch(orientationId / 4){
			case 0: rotation = quat(); break; //no rotation, front
			case 1: rotation = quat::axisRotation(vec3(1, 0, 0), M_PI_2); break; //X, top
			case 2: rotation = quat::axisRotation(vec3(0, 1, 0), -M_PI_2); break; //Y, right
			case 3: rotation = quat::axisRotation(vec3(1, 0, 0), -M_PI_2); break; //X, bottom
			case 4: rotation = quat::axisRotation(vec3(0, 1, 0), M_PI_2); break; //Y, left
			case 5: rotation = quat::axisRotation(vec3(1, 0, 0), M_PI); break; //X, back
		}

		//and rotate the size about itself on the z axis
		switch(orientationId % 4){
			case 0: break; //no rotation
			case 1: rotation = quat::axisRotation(vec3(0, 0, 1), M_PI_2) * rotation; break; //once counterclockwise
			case 2: rotation = quat::axisRotation(vec3(0, 0, 1), M_PI) * rotation; break; //twice counterclockwise
			case 3: rotation = quat::axisRotation(vec3(0, 0, 1), M_PI * 1.5) * rotation; break; //thrice counterclockwise
		}

		return rotation;
	}

	//checks for the closes collision between a ray (start and dir) and any of the blocks in this piece
	//does not change $result if no collision happens
	//O(n), but it's ok for small pieces
	bool collisionCheck(const vec3& start, const vec3& dir, collisionResult& result) const{
		const int otherAxes[] = {1, 2, 0, 2, 0, 1}; //the axes inside the plane for each axis that's orthonormal to them

		ivec3 collisionBlock;
		float lowestT = std::numeric_limits<float>::max();
		int collisionSide = 0;

		for (int axis = 0; axis < 3; axis++){ //YZ, XZ, XY planes
			for (auto& pairs : blocks){
				const ivec3& b = pairs.first;
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

	std::unordered_map<ivec3, int> blocks;
	int orientationId;
};

#endif
