#include <cstdlib>
#include <cstdio>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "app.h"

//statics
GLuint Texture::boundTexture = 0;
Texture* TextRender::tex = nullptr;
Texture* Frame::roundedBox = nullptr;
UIElem* UIElem::focus = nullptr;
App* app;

//sadly freeglut requires plain function pointers (with no class context) for callbacks, so it's a bit messy
void reshapeCallback(int w, int h){
	app->reshape(w, h);
}

void displayCallback(){
	app->update();
}

void keyboardDownCallback(uint8_t key, int x, int y){
	app->keyboardDown(key, x, y);
}

void keyboardUpCallback(uint8_t key, int x, int y){
	app->keyboardUp(key, x, y);
}

void specialDownCallback(int key, int x, int y){
	app->specialDown(key, x, y);
}

void specialUpCallback(int key, int x, int y){
	app->specialUp(key, x, y);
}

void mouseButtonCallback(int key, int state, int x, int y){
	app->mouseButton(key, state, x, y);
}

void mouseMotionCallback(int x, int y){
	app->mouseMotion(x, y);
}

void idleCallback(){
	app->idle();
}


int main(int argc, char** argv){
	glutInit(&argc, argv);
	//glutInitContextVersion (1, 1);
	//glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	//glutInitContextProfile(GLUT_CORE_PROFILE);
	app = new App();
	app->init();

	glutReshapeFunc(reshapeCallback);
	glutDisplayFunc(displayCallback);
	glutKeyboardFunc(keyboardDownCallback);
	glutKeyboardUpFunc(keyboardUpCallback);
	glutSpecialFunc(specialDownCallback);
	glutMouseFunc(mouseButtonCallback);
	glutMotionFunc(mouseMotionCallback);
	glutPassiveMotionFunc(mouseMotionCallback);
	glutIdleFunc(idleCallback);

	glutMainLoop();
	app->end();

	return 0;
}
