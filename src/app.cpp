#include <GL/glew.h>
#include <GL/freeglut.h>

#include "app.h"
#include "graphics/ui_render.h"

UIRender* uiRender = nullptr;

ivec2 windowResolution;
int anisoLevel;
int drawingAt;

struct mouseInfo_s mouseInfo;
struct keyboardState_s keyboardState;

Texture* blockTexture;
Texture* blurredBlob;

Shader* blockShader;
Shader* lineShader;

VertexArrayObject<blockVert>* axesVAO;

int state;
Screen* Screen::screens[NUM_STATES];

std::list<Piece> pieces; //actual piece storage
std::list<int> piecesCopies;
ivec3 worldSize;
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
	srand(time(nullptr));

	windowResolution.set(800, 600);
	glutInitWindowSize(windowResolution.x, windowResolution.y);
	glutInitDisplayMode(GLUT_RGB | GLUT_MULTISAMPLE);
	glutCreateWindow("Bedlam Puzzle Solver");
	printf("created main glut window\n");

	//Let GLEW chase those extension function pointers
	glewExperimental = GL_TRUE; //this is needed 
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
	//glFrontFace(GL_CCW);

	//get some aa going
	/*glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);*/
	//glEnable(GL_MULTISAMPLE_ARB);

	glEnable(GL_DEPTH_TEST);

	//initialize common textures
	uint8_t* blurred = (uint8_t*) malloc(256 * 256);

	for(int j = 0; j < 256; j++){
		float y = (float)(j - 127) / 256;
		for (int i = 0; i < 256; i++){
			float x = (float)(i - 127) / 256;
			float dist = sqrt(x * x + y * y); //goes from 0 to 0.5
			float coeff = cos(M_PI * dist); //1.0 to 0.0, flat then steep
			blurred[i + j * 256] = (uint8_t)(std::max(0.0f, coeff * 255));
		}
	}

	blurredBlob = new Texture(1, GL_UNSIGNED_BYTE, 256, 256, blurred, GL_LINEAR, GL_LINEAR, anisoLevel);
	free(blurred);

	blockTexture = nullptr;

	const Shader::attribLocationPair blockShaderAttribs[] = {
		{0, "pos"},
		{1, "nor"},
		{2, "tex"},
		{3, "col"},
		{0, nullptr}
	};
	blockShader = new Shader("shaders/block", blockShaderAttribs);
	lineShader = new Shader("shaders/line", blockShaderAttribs);

	axesVAO = new VertexArrayObject<blockVert>();
	blockVert* v = new blockVert[6];
	memset(v, 0, 6 * sizeof(blockVert));
	for (int i = 0; i < 3; i++){
		v[i * 2 + 1].position.data[i] = 1.0f;

		v[i * 2 + 0].tex.set(0.5f, 0.5f);
		v[i * 2 + 1].tex.set(1.0f, 0.5f);

		v[i * 2 + 0].col[i] = 255;
		v[i * 2 + 1].col[i] = 255;

		v[i * 2 + 0].col[3] = 255;
		v[i * 2 + 1].col[3] = 255;
	}
	axesVAO->assign(6, v);
	delete [] v;

	//build UI
	Screen::screens[STATE_LIST_PIECES] = new PieceListScreen();
	printf("UI elements alive: %d\n", UIElem::elemsAlive);

	Screen::screens[STATE_EDIT_PIECE] = new EditingScreen();
	printf("UI elements alive: %d\n", UIElem::elemsAlive);

	Screen::screens[STATE_SOLVE] = new SolvingScreen();
	printf("UI elements alive: %d\n", UIElem::elemsAlive);

	Screen::screens[STATE_SHOW_RESULT] = new SolutionScreen();
	printf("UI elements alive: %d\n", UIElem::elemsAlive);

	printf("created screens\n");

	glutReportErrors();

	//ignition
	//state = STATE_LIST_PIECES;

	//debugging the solution showing screen;
	pieces.emplace_back();
	piecesCopies.emplace_back(1);
	pieces.back().insert(ivec3(0, 0, 0));
	pieces.back().insert(ivec3(1, 0, 0));
	pieces.back().insert(ivec3(0, 1, 0));
	pieces.back().insert(ivec3(0, 1, 1));
	
	pieces.emplace_back();
	piecesCopies.emplace_back(1);
	pieces.back().insert(ivec3(0, 0, 0));
	pieces.back().insert(ivec3(1, 0, 0));
	pieces.back().insert(ivec3(0, 1, 0));
	pieces.back().insert(ivec3(0, 1, 1));

	worldSize.set(2, 2, 2);
	SolvingScreen::getInstance().transition();

	//schedule the first update
	glutTimerFunc(500, updateCallback, 0);
}

void end(){
	delete blockShader;
	delete lineShader;
	delete axesVAO;

	for (int i = 0; i < NUM_STATES; i++){
		delete Screen::screens[i];
		printf("UI elements alive: %d\n", UIElem::elemsAlive);
	}

	Frame::end();
	TextRender::end();
	TextInput::end();

	delete uiRender;
}

void reshapeCallback(int w, int h){
	windowResolution.set(w, h);
	glViewport(0, 0, w, h);
	if (uiRender) uiRender->windowResized(windowResolution);
}

void displayCallback(){
	//int currentTime = glutGet(GLUT_ELAPSED_TIME);

	//update for each state
	Screen::screens[state]->update();

	std::copy(mouseInfo.mouseDown, mouseInfo.mouseDown + 3, mouseInfo.lastMouseDown);

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
	//printf("keyboard down %d\n", key);

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
	//printf("keyboard special down %d\n", key);

	keyboardState.specialDown[key] = true;

	if (UIElem::focus){
		UIElem::focus->onControlKey(key);
	}
}

void specialUpCallback(int key, int x, int y){
	keyboardState.specialDown[key] = false;
}

void mouseButtonCallback(int key, int state, int x, int y){
	if (mouseInfo.mouse != ivec2(x, y)){
		mouseInfo.mouse.set(x, y);
		UIElem::hover = Screen::screens[state]->rootUI->collides(mouseInfo.mouse);
	}

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
						printf("mouse button %d up at (%d, %d), on %s\n", key, x, y, demangledTypeName(UIElem::hover).c_str());
						UIElem::hover->onMouseUp(mouseInfo.mouse, key);
					}

					mouseInfo.mouseButtonDownOn[key] = nullptr;
				}
			}

			break;
		}

		case 3: //wheel up
		case 4:{ //wheel down
			if (state == GLUT_DOWN){
				if (UIElem::hover){
					UIElem::hover->onWheel(mouseInfo.mouse, key == 3? -1 : 1);
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
				return; //don't move and drag at the same time
			}
		}
	}

	if (UIElem::hover){
		UIElem::hover->onMouseMove(mouseInfo.mouse);
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
	glutTimerFunc(16, updateCallback, 0);
}


template<>
void VertexArrayObject<blockVert>::setAttribPointers(){
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(blockVert), (void*) (offsetof(blockVert, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(blockVert), (void*) (offsetof(blockVert, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(blockVert), (void*) (offsetof(blockVert, tex)));
	glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(blockVert), (void*) (offsetof(blockVert, col)));
}

//In the x, y, z order, negative, then positive, right hand rule, consistent with collision side bitmask
const blockVert defaultBlock[] = {
	//left -x
	{{0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//right +x
	{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//bottom -y
	{{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//top +y
	{{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//back -z
	{{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//front +z
	{{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}}
};