#include <vector>
#include <list>

#include <limits>
#include <functional>
#include <iostream>

#include "solver.h"

using namespace std;

void CSolver::addPiece(int w, int h, int d, const char* structure, char alias) {
	pieces.push_back(list<CPiece*>());
	aliases.append(&alias, 1);

	list<CPiece*>& images = *pieces.rbegin();

	//add the original
	images.push_back(new CPiece(w, h, d, structure));
	CPiece& p = **(images.rbegin());

	//this lambda function check if the image already exists.
	//not efficient, but pieces are small enough not to matter too much
	auto addImage = [&images] (CPiece* ptr) {
		for(list<CPiece*>::iterator it = images.begin(); it != images.end(); it++) {
			if (**it == *ptr) {
				return;
			}
		}

		images.push_back(ptr);
	};

	//every permutation
	for (int i = 0; i < 3; i++) {
		CPiece* a = p.rotated(axis_x, i + 1);
		addImage(a);

		for (int j = 0; j < 3; j++) {
			CPiece* b = a->rotated(axis_y, j + 1);
			addImage(b);

			for (int k = 0; k < 3; k++) {
				addImage(b->rotated(axis_z, k + 1));
			}
		}
	}

	cout << "added piece \'" << alias << "\', with " << images.size() << " images" << endl;
}

bool CSolver::solve(int maxSolutions) {
	if (pieces.size() == 0) {
		cout << "puzzle has no pieces" << endl;
		return false;
	}

	//check if problem can be solved and count the number of nodes in the matrix
	int piecesCover = 0;
	for (vector<list<CPiece*> >::iterator it = pieces.begin(); it != pieces.end(); it++)
		piecesCover += it->front()->volume();

	if (piecesCover < spaces) {
		cout << "the pieces have less material than the volume of the space" << endl;
		return false;
	}

	if (piecesCover > spaces) {
		cout << "the pieces have more material than the volume of the space" << endl;
		return false;
	}

	//otherwise the problem can be solved

	//find the piece with most images and remove all its images but the first
	//this eliminates redundant solutions that have different orientations
	//TODO: this only works for cube puzzle boxes like tetris and bedlam; check for this
	int mostId = 0;
	int most = 0;

	for (unsigned i = 0; i < pieces.size(); i++)
		if (pieces[i].size() > most) {
			most = pieces[i].size();
			mostId = i;
		}

	list<CPiece*>& chosen = pieces[mostId];
	list<CPiece*>::iterator deleteStart = chosen.begin();
	deleteStart++; //start with the second one

	for (list<CPiece*>::iterator it = deleteStart; it != chosen.end(); it++)
		delete *it; //avoid memory leak

	chosen.erase(deleteStart, chosen.end());

	//count the amount of nodes and of rows
	//each row is a permutation of an image of a piece
	int rows = 0;
	for (vector<list<CPiece*> >::iterator it = pieces.begin(); it != pieces.end(); it++) {
		int pieceVol = it->front()->volume();
		int locations = 0;

		for (list<CPiece*>::iterator it2 = it->begin(); it2 != it->end(); it2++)
			locations += (width - (*it2)->width + 1) * (height - (*it2)->height + 1) * (depth - (*it2)->depth + 1);

		rows += locations;
		nodeAmount += locations * (pieceVol + 1); // +1 is for the piece flag on the right side of the matrix
	}

	this->maxSolutions = maxSolutions;

	cols = spaces + pieces.size();
	partialSolutionRows = (node**) malloc(cols * sizeof(node*));
	cout << "there are " << cols << " columns in the matrix" << endl;

	//create the header, which starts at the root
	header = (header_node*) malloc(cols * sizeof(header_node));
	memset(header, 0, cols * sizeof(header_node));

	root.size = 0;
	root.right = &header[0];
	root.left = &header[cols - 1];

	for (int i = 0; i < cols; i++) {
		header_node& h = header[i];
		h.right = (i == cols - 1) ? &root : &header[i + 1];
		h.left = (i == 0) ? &root : &header[i - 1];

		if (i >= spaces) {
			h.name = aliases[i - spaces];
			h.endCol = true;
		}

		else {
			h.name = NON_FLAG_NODE_NAME;
			h.endCol = false;
			h.id = i;
		}
	}

	cout << "there are " << rows << " rows in the table" << endl;

	//allocate the exact amount of nodes
	nodes = (node*) malloc(nodeAmount * sizeof(node));
	memset(nodes, 0, nodeAmount * sizeof(node));

	//there is no way to access a node at a specific location directly, so we must build the matrix in a specific way;

	//store a line of the bottom-most processed nodes
	node** rule = (node**) malloc(cols * sizeof(node*));
	memset(rule, 0, cols * sizeof(node*));

	//create the sparse matrix row by row
	int row = 0;

	list<pair<int, list<CPiece*>::iterator> > its;

	for (int i = 0; i < pieces.size(); i++)
		its.push_back(pair<int, list<CPiece*>::iterator>(i, pieces[i].begin()));

	list<pair<int, list<CPiece*>::iterator> >::iterator it = its.begin();

	while (its.size() > 0) {
		int pieceId = it->first;
		CPiece& im = **(it->second);

		//locations
		for (int z = 0; z <= depth - im.depth; z++)
			for (int y = 0; y <= height - im.height; y++)
				for (int x = 0; x <= width - im.width; x++) {

					node* first = NULL;
					node* last = NULL;

					bool ref = false;

					//blocks - iterate in the order z, y, x so that the id increases at each step
					for (int k = 0; k < im.depth; k++)
						for (int j = 0; j < im.height; j++)
							for (int i = 0; i < im.width; i++) {
								if (im.data(i, j, k) or ref) {
									int spaceId = (!ref) ? space(x + i, y + j, z + k) : spaces + pieceId;

									node* cell = newNode();

									cell->colHeader = &header[spaceId];

									//vertical
									if (!rule[spaceId]) { //the first few rows
										header[spaceId].down = cell;
										cell->up = &header[spaceId];
									}

									else {
										rule[spaceId]->down = cell;
										cell->up = rule[spaceId];
									}

									rule[spaceId] = cell;

									//horizontal
									if (!last) first = last = cell;
									else {
										last->right = cell;
										cell->left = last;
										last = cell;
									}
								}

								//prolong the loop by one to account for the ref flag on the right
								if (!ref and k == im.depth - 1 and j == im.height - 1 and i == im.width - 1) {
									ref = true;
									i--;
								}
							}

					first->left = last;
					last->right = first;

					row++; //one row for every location of every image
				}

		it->second++;
		if (it->second == pieces[it->first].end()) it = its.erase(it);

		else if (sType == interleaved) it++;

		//loop over from the start
		if (sType == interleaved) if (it == its.end()) it = its.begin();
	}

	//rule now contains the lowest nodes - connect them to the header nodes
	for (int i = 0; i < cols; i++) {
		rule[i]->down = &header[i];
		header[i].up = rule[i];
	}

	free(rule);

	//count the nodes in each column
	for (header_node* col = (header_node*) root.right; col != &root; col = (header_node*) col->right) {
		col->size = 0;
		for (node* item = col->down; item != col; item = item->down)
			col->size++;
	}

	cout << endl;

	//fire it up
	timer.reset();
	CTimer timer2;
	search(0);
	cout << "took " << timer2.elapsedMilli() << " milliseconds to find " << solutions.size() << " solutions" << endl;

	free(header);
	free(partialSolutionRows);

	return solutions.size() > 0;
}

//Knuth's Dancing Links
void CSolver::search(int k) {
	if ((int) solutions.size() == maxSolutions) return;

	//found a solution
	if (root.right == &root or root.left == &root) {
		cout << "found in " << timer.sinceLastMilli() << " milliseconds since last one" << endl;

		//process solution
		for (int i = 0; i < k; i++) {
			node* n = partialSolutionRows[i];

			//go all the way to the end of the row
			while (n->colHeader->name == NON_FLAG_NODE_NAME)
				n = n->left;

			char name = n->colHeader->name;

			n = n->right;

			while (!n->colHeader->endCol) {
				currentSolution[n->colHeader->id] = name;
				n = n->right;
			}
		}

		solutions.push_back(string(currentSolution));
		return;
	}

	//choose a column
	header_node* col = NULL;
	col = (header_node*) root.right;

	/*
	 //take the one with the least nodes in it
	 int s = (int)0x0FFFFFFF;
	 for (header_node* j = (header_node*) root.right; j != &root; j = (header_node*) j->right) {
	 if (j->size < s) s = j->size;
	 col = j;
	 }*/

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

//early debug only, useless
void CSolver::printSolutionsLine(int amount) const {
	if (solutions.size() == 0) {
		cout << "there are no solutions" << endl;
		return;
	}

	if (amount == 0 or amount > solutions.size()) amount = solutions.size();

	for (int i = 0; i < amount; i++)
		cout << solutions[i] << endl;
}

void CSolver::printSolutionsHorizontalLevels(int amount) const {
	if (solutions.size() == 0) {
		cout << "there are no solutions" << endl;
		return;
	}

	if (amount == 0 or amount > solutions.size()) amount = solutions.size();

	for (int item = 0; item < amount; item++) {
		for (int j = 0; j < height; j++) {
			for (int k = 0; k < depth; k++) {
				for (int i = 0; i < width; i++) {
					const string& s = solutions[item];
					cout << s[space(i, j, k)];
				}

				cout << "  ";
			}

			cout << endl;
		}

		cout << endl << endl;
	}
}

void CSolver::printSolutionsVerticalLevels(int perLine, int amount) const {
	assert(perLine > 0);

	if (solutions.size() == 0) {
		cout << "there are no solutions" << endl;
		return;
	}

	if (amount == 0 or amount > solutions.size()) amount = solutions.size();
	if (perLine > amount) perLine = amount;

	int lines = amount / perLine;
	int atLineStart = 0;

	for (int b = 0; b < lines; b++) {
		for (int k = 0; k < depth; k++) {
			for (int j = 0; j < height; j++) {
				for (int item = 0; (item < perLine and b != lines - 1) or (b == lines - 1 and item < amount % perLine); item++) {
					const string& s = solutions[atLineStart + item];

					for (int i = 0; i < width; i++)
						cout << s[space(i, j, k)];

					cout << "  ";
				}

				cout << endl;
			}
		}

		cout << endl << endl;
		atLineStart += perLine;
	}
}
