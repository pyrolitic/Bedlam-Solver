#include <iostream>
#include <vector>

#include "solver.h"
//#include "draw.h"

/* TODO:
 * -load the shapes from a file
 * -add a schematic generator
 */

using namespace std;

const struct {
	int w, h, d;
	const char* def;
} defs[] =
	{
//0
				{ 3, 3, 1, "\
 x \
xxx\
x  " },

//1
				{ 3, 3, 1, "\
 x \
xxx\
 x " },

//2
				{ 3, 3, 1, "\
x  \
xx \
 xx" },

//3
				{ 3, 2, 2, "\
 xx\
xx \
  x\
   " },

//4
				{ 3, 2, 2, "\
xxx\
 x \
 x \
   " },

//5
				{ 3, 2, 2, "\
 xx\
xx \
 x \
   " },

//6
				{ 3, 2, 2, "\
   \
xxx\
 x \
 x " },

//7
				{ 2, 3, 2, "\
 x\
 x\
xx\
  \
  \
 x" },

//8
				{ 2, 3, 2, "\
xx\
 x\
 x\
  \
  \
 x" },

//9
				{ 2, 3, 2, "\
 x\
 x\
xx\
  \
  \
x " },

//10
				{ 3, 2, 2, "\
 x \
xx \
 xx\
   " },

//11
				{ 3, 2, 2, "\
   \
 x \
x  \
xxx" },

//12
				{ 2, 2, 2, "\
x \
xx\
  \
 x" } };

int main() {
	CSolver solver(4, 4, 4);

	for (int i = 0; i < 13; i++)
		solver.addPiece(defs[i].w, defs[i].h, defs[i].d, defs[i].def, (i < 10) ? '0' + i : 'A' + i - 10);

	solver.solve(5);

	//solver.drawImage("tetrisOut.png");
	solver.printSolutionsHorizontalLevels();

	return 0;
}
