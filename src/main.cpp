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

int main(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitContextVersion (3, 3);
	//glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

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

	printf("exited cleanly\n");
	return 0;
}
