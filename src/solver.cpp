#include <cstring>

#include <vector>
#include <list>
#include <limits>
#include <functional>
#include <iostream>
#include <algorithm>

#include "solver.h"

using namespace std;

void Solver::solve() {
	if (pieceImages.empty()){
		printf("solver: nothing to do\n");
		return;
	}

	//make every image of every piece
	for (int pieceId = 0; pieceId < originals.size(); pieceId++){
		auto& imageList = pieceImages[pieceId];

		for (int i = 0; i < ORIENTATIONS; i++){
			imageList.emplace_back(Piece(originals[pieceId], i)); //unique orientation $i
			if (!(imageList.back().size <= dimensions)){
				//printf("discarding image that won't fit\n");
				imageList.pop_back(); //won't fit
			}
		}

		//TODO: if this is too slow, maybe hash everything and compare hashes first
		int amount = imageList.front().size.x * imageList.front().size.y * imageList.front().size.z; 
		imageList.sort([&amount](const DensePiece& one, const DensePiece& other) -> bool{
			if (one.size.x < other.size.x) return true;
			else if (one.size.x == other.size.x){
				if (one.size.y < other.size.y) return true;
				else if (one.size.y == other.size.y){
					if (one.size.z < other.size.z) return true;
					else if (one.size.z == other.size.z){
						return memcmp(one.data, other.data, amount) < 0;
					}
				}
			}

			return false;
		});

		//now remove duplicates
		int removes = 0;
		auto prev = imageList.begin();
		for (auto it = std::next(prev); it != imageList.end();){
			auto next = std::next(it);

			if (*prev == *it){
				imageList.erase(it);
				removes++;
			}
			else{
				prev = it;
			}

			it = next;
		}

		/*for (auto& im : imageList){
			printf("DensePiece(orientation %02d): size = (%d %d %d)\n", im.orientation, im.size.x, im.size.y, im.size.z);

			for (int z = 0; z < im.size.z; z++){
				for (int y = 0; y < im.size.y; y++){
					for (int x = 0; x < im.size.x; x++){
						if (im.data[LOOKUP(ivec3(x, y, z), im.size)]){
							printf("(%d %d %d), ", x, y, z);
						}
					}
				}
			}

			printf("\n\n");
		}*/
		printf("piece #%d: culled %d duplicate images\n", pieceId, removes);
	}

	//in order to prevent rotated duplicates of the same solution, find the image with most distinct images, 
	//and later, for one of its instances, include only the original orientation
	int mostDistinctImages = pieceImages[0].size();
	auto* mostDistinctImagesHolder = &pieceImages[0];
	for (int pieceId = 1; pieceId < originals.size(); pieceId++){
		auto& imageList = pieceImages[pieceId];
		if (imageList.size() > mostDistinctImages){
			mostDistinctImages = imageList.size();
			mostDistinctImagesHolder = &imageList;
		}
	}

	printf("solver: most distinct images: %d, of piece #%d\n", mostDistinctImages, mostDistinctImagesHolder - pieceImages.data());

	//count the amount of nodes and of rows
	//each row is a permutation of an image of a piece (excluding the header row)
	//the number of permutations per piece is the number of images times the number of possible positions
	rows = 0;
	nodeAmount = 0;
	pieceInstances = 0;

	for (int i = 0; i < pieceImages.size(); i++){
		auto& listImages = pieceImages[i];
		int firstOnlyLocations = 0;
		int allLocations = 0;

		//for the first copy of the piece with the most distinct images, only consider the first orientation
		for (auto it = listImages.begin(); it != listImages.end(); it++){
			ivec3 wiggle = (dimensions - it->size) + ivec3(1, 1, 1);
			allLocations += wiggle.x * wiggle.y * wiggle.z;
			if (it == listImages.begin()) firstOnlyLocations = wiggle.x * wiggle.y * wiggle.z;
		}

		if (&listImages == mostDistinctImagesHolder){
			rows += allLocations * (piecesCopies[i] - 1) + firstOnlyLocations;
			nodeAmount += allLocations * piecesMass[i] * (piecesCopies[i] - 1) + firstOnlyLocations * piecesMass[i];
		}
		else{
			rows += allLocations * piecesCopies[i];
			nodeAmount += allLocations * piecesMass[i] * piecesCopies[i];
		}
		pieceInstances += piecesCopies[i];
	}

	cols = spaces + pieceInstances;
	printf("solver: there are %d columns and %d rows in the sparse matrix\n", cols, rows);

	partialSolutionRows = (node**) malloc(cols * sizeof(node*));
	memset(partialSolutionRows, 0, cols * sizeof(node*));

	//create the header top row and the ref nodes
	//keep them each in a separate pool from the meterial nodes, to be able to tell if a neighbour is a header or a ref node
	refNodes = (refNode*) malloc(rows * sizeof(refNode));
	headers = (headerNode*) malloc(cols * sizeof(headerNode));
	memset(refNodes, 0, rows * sizeof(headerNode));
	memset(headers, 0, cols * sizeof(headerNode));

	int nodeId = 0;
	for (int spaceId = 0; spaceId < spaces; spaceId++){
		headerNode& h = headers[nodeId++];
		h.spaceId = spaceId;
		h.copyId = -1;
		h.size = 0;
	}

	for (int pieceId = 0; pieceId < pieceImages.size(); pieceId++){
		for (int copyId = 0; copyId < piecesCopies[pieceId]; copyId++){
			headerNode& h = headers[nodeId++];
			h.pieceId = pieceId;
			h.copyId = copyId;
			h.size = 0;
		}
	}

	assert(nodeId == cols);

	//make header horizontal links
	for (int ci = 0; ci < cols; ci++) {
		headerNode& h = headers[ci];
		h.right = (ci == cols - 1) ? &root : &headers[ci + 1];
		h.left = (ci == 0) ? &root : &headers[ci - 1];
	}

	//attach root to the left of the header row
	root.size = 0;
	root.right = &headers[0];
	root.left = &headers[cols - 1];
	root.down = root.up = nullptr; //won't ever be accessed

	/* matrix is now

	                all null
	                 |             |                |                 |                        |               |                        |
	loop from end - root  --  s0(0, 0, 0)  --  s1(1, 0, 0) -- sN(W-1, H-1, D-1) -- ... -- piece0_copy0 - piece0_copy_1 -- ... -- pieceN-1_copyC-1 -- loop back to root
	                 |             |                |                 |                        |               |                        |
	                also all null (stuff goes here)
	*/

	printf("solver: created the header row\n");

	//allocate the exact amount of nodes required
	nodes = (node*) malloc(nodeAmount * sizeof(node));
	memset(nodes, 0, nodeAmount * sizeof(node));

	//there is no way to access a node at a location directly, so we must build the matrix in a methodical way, row by row;

	//keep a line of the bottom-most processed nodes, so as to easily provide a top link to new nodes
	node** lowestNodes = (node**) malloc(cols * sizeof(node*));
	for (int ci = 0; ci < cols; ci++){
		lowestNodes[ci] = &headers[ci];
	}

	int instanceId = 0; //non-trivial to compute
	int rowId = 0; //also instance ID

	for (int pieceId = 0; pieceId < pieceImages.size(); pieceId++){
		auto& pieceList = pieceImages[pieceId];

		//as many copies as requested
		for (int copyId = 0; copyId < piecesCopies[pieceId]; copyId++){

			//every unique image (orientation);
			//for the first instance of the piece with the most distinct images, only consider the first orientation
			auto imEndIt = (copyId == 0 and &pieceList == mostDistinctImagesHolder)? std::next(pieceList.begin()) : pieceList.end();

			for (auto imIt = pieceList.begin(); imIt != imEndIt; imIt++){
				auto& im = *imIt;
				//printf("processing image %d, with size %d %d %d\n", im.data, im.size.x, im.size.y, im.size.z);

				//all possible placements for this image
				for (int bz = 0; bz <= dimensions.z - im.size.z; bz++) {
					for (int by = 0; by <= dimensions.y - im.size.y; by++) {
						for (int bx = 0; bx <= dimensions.x - im.size.x; bx++) {
							//generate a single row

							node* first = nullptr; //first in the row
							node* last = nullptr; //last one to be added

							//blocks - iterate in the order bz, by, bx so that the id increases at each step
							int nodesInRow = 0; //debug
							for (int k = 0; k < im.size.z; k++){
								for (int j = 0; j < im.size.y; j++){
									for (int i = 0; i < im.size.x; i++) {
										if (im.query(ivec3(i, j, k))) {
											int horizontalId = space(bx + i, by + j, bz + k);

											nodesInRow++;
											node* cell = newNode();

											//vertical links
											lowestNodes[horizontalId]->down = cell;
											cell->up = lowestNodes[horizontalId];

											//horizontal links
											if (last){
												last->right = cell;
												cell->left = last;
												last = cell; //this is the now the right-most node of the column
											}
											else {
												first = cell;
												last = cell;
											}

											lowestNodes[horizontalId] = cell; //this is now the bottom-most node of this column
										}
									}
								}
							}

							assert(nodesInRow > 0);
							assert(last);

							//add the ref node to the right of the position nodes
							int horizontalId = spaces + instanceId;
							refNode* ref = &refNodes[rowId];

							ref->orientation = im.orientation;
							ref->location.set(bx, by, bz);

							assert(headers[horizontalId].pieceId == pieceId);
							assert(headers[horizontalId].copyId == copyId);

							ref->up = lowestNodes[horizontalId];
							lowestNodes[horizontalId]->down = (node*)ref;
							lowestNodes[horizontalId] = (node*)ref;

							last->right = (node*)ref;
							ref->left = last;

							//wrap around the links
							ref->right = first;
							first->left = (node*)ref;

							rowId++; //row done
						}
					}
				}
			}

			instanceId++;
		}
	}

	assert(rowId == rows);

	//lowestNodes now contains the final lowest nodes - connect them to the header nodes
	for (int i = 0; i < cols; i++) {
		lowestNodes[i]->down = &headers[i];
		headers[i].up = lowestNodes[i];
	}

	free(lowestNodes);
	lowestNodes = nullptr;

	//count the nodes in each column
	for (headerNode* col = (headerNode*) root.right; col != &root; col = (headerNode*) col->right) {
		col->size = 0;
		for (node* item = col->down; item != col; item = item->down){
			col->size++;
		}
	}

	/* matrix is now

	               null         everything else loops back to the lowest node on that column
	                 |               |              |                    |                        |               |                         |
	loop from end - root ------  (0, 0, 0) ---  (1, 0, 0) -- ... -- (W-1, H-1, D-1) -- ... -- piece0_copy0 -- piece0_copy_1 - ... -- pieceN-1_copyC-1 -- loop back to root            header row
	                 |               |              |                    |                        |               |                         |
	               null)  (loop - material  --------+------- ... ---  material  ------ ... ---  marker  ----------+---------- ... ----------+----------- loop back to first material  row 0
	                                 |              |                    |                        |               |                         |
	                       loop -----+---------  material -- ... ---  material  ------ ... -------+-----------  marker  ----- ... ----------+----------- loop back to first material  row 1
	                                 |              |                    |                        |               |                         |            
	                                ...            ...                  ...                      ...             ...                       ...
	                                 |              |                    |                        |               |                         |            
	                       loop -----+--------------+------- ... ---  material  ------ ... -------+---------------+---------- ... ------  marker  ------ loop back to first material  row R-1
	                                 |              |                    |                        |               |                         |     
	                             loop every column to back to the corresponding heading node

	where a + is a lack of a node
	*/

	printf("solver: linked all nodes\n");

	//fire it up
	solutionsFound = 0;
	search(0);

	//clean up
	free(nodes);
	free(headers);
	free(refNodes);
	free(partialSolutionRows);

	printf("solver: found %d solutions\n", solutionsFound);
	printf("solver: findHeaderNode performance: %d runs, %f lookups per run\n", findHeaderNodeGoes, (float)findHeaderNodeOps / findHeaderNodeGoes);
}

//Knuth's Dancing Links
void Solver::search(int k) {
	//found a solution
	if (root.right == &root or root.left == &root) {
		printf("solver: found solution, k = %d\n", k);

		//process solution
		solutionPiece* solution = (solutionPiece*) malloc(pieceInstances * sizeof(solutionPiece));
		assert(k == pieceInstances);

		for (int i = 0; i < k; i++) {
			node* n = partialSolutionRows[i];

			//find the ref node
			while (!isRefNode(n)){
				n = n->right;
			}

			refNode* ref = (refNode*) n;
			headerNode* refHead = findHeaderNode(ref);

			solution[i].pieceId = refHead->pieceId;
			solution[i].copyId = refHead->copyId;
			solution[i].orientationId = ref->orientation;
			solution[i].position = ref->location;
		}

		accessLock.lock();
		solutions.emplace_back(solution);
		solutionsFound++;
		accessLock.unlock();

		return;
	}

	//choose a column
	headerNode* col = nullptr;
	col = (headerNode*) root.right;

	//take the one with the least nodes in it
	/*int s = (int)0x0FFFFFFF;
	for (headerNode* j = (headerNode*) root.right; j != &root; j = (headerNode*) j->right) {
		if (j->size < s){
			s = j->size;
			col = j;
		}
	}*/

	if (dieCommand) return;
	cover(col);

	for (node* row = col->down; row != col; row = row->down) {
		partialSolutionRows[k] = row;

		for (node* j = row->right; j != row; j = j->right){
			cover(findHeaderNode(j));
		}

		search(k + 1);

		for (node* j = row->left; j != row; j = j->left){
			uncover(findHeaderNode(j));
		}
	}

	uncover(col);
}
