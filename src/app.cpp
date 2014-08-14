#include <GL/glew.h>
#include <GL/freeglut.h>

#include "app.h"
#include "graphics/ui_render.h"

UIRender* uiRender = nullptr;

vec2i windowResolution;
int anisoLevel;
int drawingAt;

struct mouseInfo_s mouseInfo;
struct keyboardState_s keyboardState;

int state;
Screen* Screen::screens[NUM_STATES];

std::list<Piece> pieces; //actual piece storage
vec3i worldSize;
Solver* solver;

void cleanInput(){
	//reset mouseInfo.mouse related state
	for (int i = 0; i < 3; i++){
		mouseInfo.mouseButtonDownOn[i] = nullptr;
		mouseInfo.mouseDown[i] = false;
		mouseInfo.lastMouseDown[i] = false;
	}

	UIElem::focus = nullptr;
	UIElem::hover = nullptr;
}

void init(){
	srand(time(NULL));

	windowResolution.set(800, 600);
	glutInitWindowSize(windowResolution.x, windowResolution.y);
	glutInitDisplayMode(GLUT_RGB | GLUT_MULTISAMPLE);
	glutCreateWindow("Bedlam Puzzle Solver");
	printf("created main glut window\n");

	//Let GLEW chase those extension function pointers
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK){
		fprintf(stderr, "could not init glew\n");
		exit(1);
	}
	printf("initialized glew\n");

	if (glutExtensionSupported("GL_EXT_texture_filter_anisotropic")){
		GLfloat aniso_f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso_f);
		anisoLevel = (int) aniso_f;
		printf("largest anisotropic filtering supported: %dx\n", anisoLevel);
	}
	else{
		anisoLevel = 1;
		printf("anisotropic filtering not supported\n");
	}

	TextRender::init();
	TextInput::init();
	Frame::init();
	printf("initialized statics\n");

	uiRender = new UIRender();

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);

	//glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	//get some aa going
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	//glEnable(GL_MULTISAMPLE_ARB);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	//build UI
	Screen::screens[STATE_LIST_PIECES] = new PieceListScreen();
	Screen::screens[STATE_EDIT_PIECE] = new EditingScreen();
	Screen::screens[STATE_SOLVE] = new SolvingScreen();
	Screen::screens[STATE_SHOW_RESULT] = new SolutionScreen();
	printf("created screens\n");

	glutReportErrors();

	//ignition
	state = STATE_LIST_PIECES;

	//schedule the first update
	glutTimerFunc(500, updateCallback, 0);
}

void end(){
	for (int i = 0; i < NUM_STATES; i++){
		delete Screen::screens[i];
	}

	Frame::end();
	TextRender::end();
	TextInput::end();
}

void reshapeCallback(int w, int h){
	windowResolution.set(w, h);
	glViewport(0, 0, w, h);
	glScissor(0,0,w,h);
	if (uiRender) uiRender->windowResized(windowResolution);
}

void displayCallback(){
	int currentTime = glutGet(GLUT_ELAPSED_TIME);

	//update for each state
	Screen::screens[state]->update();

	std::copy(mouseInfo.mouseDown, mouseInfo.mouseDown + 3, mouseInfo.lastMouseDown);

	//UI
	const int labelBufLength = 256;
	char labelBuf[labelBufLength];
	int metricW, metricH;
	//debug strings

	//draw UI

	Screen::screens[state]->rootUI->update();
	Screen::screens[state]->rootUI->draw(0);
	uiRender->render();

	//status label
	/*glColor4f(1.0f, 1.0f, 0.5f, 1.0f);

	//element hovering over
	if (UIElem::hover){
		std::string name = demangledTypeName(UIElem::hover);
		sprintf(labelBuf, "hovering over a %s", name.c_str());
		TextRender::render(labelBuf, 10, windowResolution.y - 20);
	}*/

	glutReportErrors();
	glutSwapBuffers();
}

void keyboardDownCallback(unsigned char key, int x, int y){
	printf("keyboard down %d\n", key);

	keyboardState.keyDown[key] = true;

	if (UIElem::focus){
		int keyMods = glutGetModifiers();
		UIElem::focus->onTextKey(key, keyMods);
	}
}

void keyboardUpCallback(unsigned char key, int x, int y){
	keyboardState.keyDown[key] = false;
}

void specialDownCallback(int key, int x, int y){
	printf("keyboard special down %d\n", key);

	keyboardState.specialDown[key] = true;

	if (UIElem::focus){
		UIElem::focus->onControlKey(key);
	}
}

void specialUpCallback(int key, int x, int y){
	keyboardState.specialDown[key] = false;
}

void mouseButtonCallback(int key, int state, int x, int y){
	printf("mouseInfo.mouse button %d, %d, on %s\n", key, state, demangledTypeName(UIElem::hover).c_str());
	mouseInfo.mouse.set(x, y);

	switch(key){
		case GLUT_LEFT_BUTTON:{
			//place focus on element
			if (state == GLUT_DOWN){
				UIElem::focus = UIElem::hover;
			}

			//note the lack of break
		}
		case GLUT_MIDDLE_BUTTON:
		case GLUT_RIGHT_BUTTON: {
			if (state == GLUT_DOWN){
				mouseInfo.mouseDown[key] = true;
				mouseInfo.mouseButtonDownOn[key] = UIElem::hover;
				mouseInfo.mouseButtonDownAt[key] = mouseInfo.mouse;

				if (UIElem::hover){
					UIElem::hover->onMouseDown(mouseInfo.mouse, key);
				}
			}

			else{ //GLUT_UP
				if (mouseInfo.mouseDown[key]){
					mouseInfo.mouseDown[key] = false;

					if (UIElem::hover and UIElem::hover == mouseInfo.mouseButtonDownOn[key]){
						UIElem::hover->onMouseUp(mouseInfo.mouse, key);
					}

					mouseInfo.mouseButtonDownOn[key] = nullptr;
				}
			}

			break;
		}

		case 3:{ //wheel up
			if (state == GLUT_DOWN){
				if (UIElem::hover){
					UIElem::hover->onWheel(mouseInfo.mouse, 1);
				}
			}

			break;
		}

		case 4:{ //wheel down
			if (state == GLUT_DOWN){
				if (UIElem::hover){
					UIElem::hover->onWheel(mouseInfo.mouse, -1);
				}
			}

			break;
		}
	}
}

//this is used for both active (with button held) and passive mouseInfo.mouse motion
void mouseMotionCallback(int x, int y){
	mouseInfo.oldMouse = mouseInfo.mouse;
	mouseInfo.mouse.set(x, y);

	UIElem::hover = Screen::screens[state]->rootUI->collides(mouseInfo.mouse);
	
	for (int key = 0; key < 3; key++){
		if (mouseInfo.mouseDown[key]){
			if (mouseInfo.mouseButtonDownOn[key]){
				mouseInfo.mouseButtonDownOn[key]->onMouseDrag(mouseInfo.mouse, mouseInfo.mouseButtonDownAt[key], key);
			}
		}
	}
}

void updateCallback(int data){
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	glutPostRedisplay();

	if (keyboardState.keyDown[0x1B]){
		//quit gracefully
		glutLeaveMainLoop();
	}

	//schedule a new update
	glutTimerFunc(500, updateCallback, 0);
}