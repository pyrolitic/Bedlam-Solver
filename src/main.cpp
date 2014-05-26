#include <iostream>
#include <vector>

#include "solver.h"
//#include "draw.h"

/* TODO:
 * -load the shapes from a file
 * -add a schematic generator
 */
/*
using namespace std;

const struct {
	int w, h, d;
	const char* def;
} defs[] = {
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
	Solver solver(4, 4, 4);

	for (int i = 0; i < 13; i++)
		solver.addPiece(defs[i].w, defs[i].h, defs[i].d, defs[i].def, (i < 10) ? '0' + i : 'A' + i - 10);

	solver.solve(5);

	//solver.drawImage("tetrisOut.png");
	solver.printSolutionsHorizontalLevels();

	return 0;
}

*/


#include <cstdlib>
#include <cstdio>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "app.h"

//statics
GLuint Texture::boundTexture = 0;
Texture* TextRender::tex = NULL;
App App::app;

//sadly freeglut requires plain function pointers (with no class context) for callbacks, so it's a bit messy
void reshapeCallback(int w, int h){
	App::app.reshape(w, h);
}

void displayCallback(){
	App::app.update();
}

void keyboardCallback(uint8_t key, int x, int y){
	App::app.keyboard(key, x, y);
}

void mouseButtonCallback(int key, int state, int x, int y){
	App::app.mouseButton(key, state, x, y);
}

void mouseMotionCallback(int x, int y){
	App::app.mouseMotion(x, y);
}

void specialCallback(int key, int x, int y){
	App::app.special(key, x, y);
}

void idleCallback(){
	App::app.idle();
}


int main(int argc, char** argv){
	glutInit(&argc, argv);
	//glutInitContextVersion (1, 1);
	//glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	//glutInitContextProfile(GLUT_CORE_PROFILE);
	App::app.init();

	glutReshapeFunc(reshapeCallback);
	glutDisplayFunc(displayCallback);
	glutKeyboardFunc(keyboardCallback);
	glutMouseFunc(mouseButtonCallback);
	glutMotionFunc(mouseMotionCallback);
	glutPassiveMotionFunc(mouseMotionCallback);
	glutSpecialFunc(specialCallback);
	glutIdleFunc(idleCallback);

	glutMainLoop();

	return 0;
}
