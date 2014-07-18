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
	//make every image of every piece
	for (std::list<DensePiece> imageList : pieceImages){
		DensePiece& original = imageList.back();

		for (int i = 0; i < ORIENTATIONS; i++){
			imageList.emplace_back(DensePiece(original, i)); //unique rotation $i
		}

		//TODO: if this is too slow, maybe hash everything and compare hashes first
		int amount = original.size.x * original.size.y * original.size.z; 
		imageList.sort([&amount](const DensePiece& one, const DensePiece& other) -> bool{
			return memcmp(one.data, other.data, amount * sizeof(uint8_t)) > 0;
		});

		//now remove duplicates
		auto prev = imageList.begin();
		for (auto it = std::next(prev); it != imageList.end();){
			if (*prev == *it){
				auto next = std::next(it);
				imageList.erase(it);
				it = next;
			}
			else{
				it++;
			}
		}
	}

	//count the amount of nodes and of rows
	//each row is a permutation of an image of a piece
	//the number of permutations per piece is the number of images times the number of possible positions
	nodeAmount = 0;
	int rows = 0;
	pieceInstances = 0;
	for (int i = 0; i < pieceImages.size(); i++){
		auto listImages = pieceImages[i];
		int locations = 0;

		for (auto image : listImages){
			vec3i wiggle = (dimensions - image.size) + vec3i(1, 1, 1);
			locations += wiggle.x * wiggle.y * wiggle.z;
		}

		rows += locations;
		nodeAmount += locations * (piecesMass[i] + 1); // +1 is for the piece flag on the right side of the matrix

		pieceInstances += piecesCopies[i];
	}

	cols = spaces + pieceInstances;
	partialSolutionRows = (node**) malloc(cols * sizeof(node*));
	printf("there are %d columns and %d rows in the sparse matrix\n", cols, rows);

	//create the header, which starts at the root
	header = (header_node*) malloc(cols * sizeof(header_node));
	memset(header, 0, cols * sizeof(header_node));

	root.size = 0;
	root.right = &header[0];
	root.left = &header[cols - 1];

	//make header horizontal links
	for (int i = 0; i < cols; i++) {
		header_node& h = header[i];
		h.right = (i == cols - 1) ? &root : &header[i + 1];
		h.left = (i == 0) ? &root : &header[i - 1];
	}

	for (int i = 0; i < spaces; i++){
		header_node& h = header[i];
		h.symbol = i;
		h.copy = SOLVER_SPACE_NODE_COPY_VALUE;
	}

	int nodeId = 0;
	for (int i = 0; i < pieceImages.size(); i++){
		for (int c = 0; c < piecesCopies[i]; c++){
			for (auto im : pieceImages[i]){
				header_node& h = header[nodeId++];
				h.symbol = i;
				h.copy = c;
				h.orientation = im.orientation;
			}
		}
	}

	//allocate the exact amount of nodes
	nodes = (node*) malloc(nodeAmount * sizeof(node));
	memset(nodes, 0, nodeAmount * sizeof(node));

	//there is no way to access a node at a specific location directly, so we must build the matrix in a specific way;

	//store a line of the bottom-most processed nodes, so as to easily provide a top link to new nodes
	node** lowestNodes = (node**) malloc(cols * sizeof(node*));
	for (int i = 0; i < cols; i++){
		lowestNodes[i] = &header[i];
	}

	//create the sparse matrix row by row
	int row = 0;
	int pieceId = 0; //which piece instance are we on. not trivial to compute given the variable number of images and copies per piece, so just increment instead

	for (int pieceId = 0; pieceId < pieceImages.size(); pieceId++){
		auto pieceList = pieceImages[pieceId];

		//as many copies as requested
		for(int copyId = 0; copyId < piecesCopies[pieceId]; copyId++){

			//every unique orientation
			for (auto im : pieceList){

				//all possible locations
				for (int bz = 0; bz <= dimensions.z - im.size.z; bz++) {
					for (int by = 0; by <= dimensions.y - im.size.y; by++) {
						for (int bx = 0; bx <= dimensions.x - im.size.x; bx++) {

							node* first = nullptr; //first in the row
							node* last = nullptr; //last one to be added

							//blocks - iterate in the order bz, by, bx so that the id increases at each step
							for (int k = 0; k < im.size.z; k++){
								for (int j = 0; j < im.size.y; j++){
									for (int i = 0; i < im.size.x; i++) {
										if (im.query(vec3i(i, j, k))) {
											int horizontalId = space(bx + i, by + j, bz + k);

											node* cell = newNode();
											cell->colHeader = &header[horizontalId];

											//vertical
											lowestNodes[horizontalId]->down = cell;
											cell->up = lowestNodes[horizontalId];
											lowestNodes[horizontalId] = cell; //this is now the bottom-most node of this column

											//horizontal
											if (last){
												last->right = cell;
												cell->left = last;
												last = cell;
											}
											else {
												first = cell;
												last = cell;
											}
										}
									}
								}
							}

							//add the ref node to the right of the position nodes
							int horizontalId = spaces + pieceId;
							node* ref = newNode();

							ref->colHeader = &header[horizontalId];
							assert(ref->colHeader->symbol == pieceId);
							assert(ref->colHeader->copy == copyId);

							ref->up = lowestNodes[horizontalId];
							lowestNodes[horizontalId]->down = ref;
							lowestNodes[horizontalId] = ref;

							
							last->right = ref;
							ref->left = last;

							//wrap around the links
							ref->right = first;
							first->left = ref;

							row++; //row done
						}
					}
				}
			}
		}
	}

	//lowestNodes now contains the final lowest nodes - connect them to the header nodes
	for (int i = 0; i < cols; i++) {
		lowestNodes[i]->down = &header[i];
		header[i].up = lowestNodes[i];
	}

	free(lowestNodes);

	//count the nodes in each column
	for (header_node* col = (header_node*) root.right; col != &root; col = (header_node*) col->right) {
		col->size = 0;
		for (node* item = col->down; item != col; item = item->down){
			col->size++;
		}
	}

	//fire it up
	search(0);

	free(header);
	free(partialSolutionRows);
	free(nodes);
}

//Knuth's Dancing Links
void Solver::search(int k) {
	if (dieCommand) return;

	//found a solution
	if (root.right == &root or root.left == &root) {
		printf("found solution\n");

		//process solution
		solutionPiece* solution = (solutionPiece*) malloc(pieceInstances * sizeof(solutionPiece));
		assert(k == pieceInstances);

		for (int i = 0; i < k; i++) {
			node* n = partialSolutionRows[i];

			//go all the way to the end of the row, to the ref node
			while (n->colHeader->copy == SOLVER_SPACE_NODE_COPY_VALUE){
				n = n->left;
			}

			header_node* ref = (header_node*) n;
			solution[i].pieceId = ref->symbol;
			solution[i].copyId = ref->copy;
			solution[i].orientationId = ref->orientation;
		}

		accessLock.lock();
		solutions.emplace_back(solution);
		accessLock.unlock();
		return;
	}

	//choose a column
	header_node* col = nullptr;
	col = (header_node*) root.right;

	
	//take the one with the least nodes in it
	/*int s = (int)0x0FFFFFFF;
	for (header_node* j = (header_node*) root.right; j != &root; j = (header_node*) j->right) {
		if (j->size < s){
			s = j->size;
			col = j;
		}
	}*/

	if(dieCommand) return;
	cover(col);

	for (node* row = col->down; row != col; row = row->down) {
		partialSolutionRows[k] = row;

		for (node* j = row->right; j != row; j = j->right)
			cover(j->colHeader);

		search(k + 1);
		//and reverse
		for (node* j = row->left; j != row; j = j->left)
			uncover(j->colHeader);
	}

	uncover(col);
}
