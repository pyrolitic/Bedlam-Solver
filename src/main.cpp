#include "app.h"

//statics
GLuint Texture::boundTexture = 0;
GLuint Shader::boundProgram = 0;

Texture* TextRender::fontTexture = nullptr;

uiVert Frame::verts[36];
Texture* Frame::roundedBox = nullptr;

std::vector<uiVert> TextInput::caretVerts;

UIElem* UIElem::hover = nullptr;
UIElem* UIElem::focus = nullptr;
int UIElem::elemsAlive = 0;

int main(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitContextVersion (2, 1);

	//create the main window
	init();

	//now associate these callbacks with the main window
	glutReshapeFunc(reshapeCallback);
	glutDisplayFunc(displayCallback);
	glutKeyboardFunc(keyboardDownCallback);
	glutKeyboardUpFunc(keyboardUpCallback);
	glutSpecialFunc(specialDownCallback);
	glutMouseFunc(mouseButtonCallback);
	glutMotionFunc(mouseMotionCallback);
	glutPassiveMotionFunc(mouseMotionCallback);

	//clicking on the close button on the window bar will simply end the main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	//pass control to freeglut
	glutMainLoop();

	printf("glut main loop ended\n");
	end();

	printf("UI elements still alive: %d\n", UIElem::elemsAlive);
	printf("exited cleanly (but can't promise no leaks or corruption)\n");
	return 0;
}
