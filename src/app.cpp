

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "app.h"

void App::transition(int newState){
	//reset mouse related state
	for (int i = 0; i < 3; i++){
		mouseButtonDownOn[i] = nullptr;
		mouseDown[i] = false;
		lastMouseDown[i] = false;
	}

	UIElem::focus = nullptr;

	state = newState;
}

void App::init(){
	srand(time(NULL));

	width = 800;
	height = 600;
	
	glutInitWindowSize(width, height);
	glutInitDisplayMode(GLUT_RGB | GLUT_MULTISAMPLE);
	glutCreateWindow("Bedlam Puzzle Solver");
	printf("created main glut window\n");

	//Let GLEW chase those extension function pointers
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
		anisoLevel = 0;
		printf("anisotropic filtering not supported\n");
	}

	uint8_t* blurred = (uint8_t*) malloc(256 * 256);

	for(int j = 0; j < 256; j++){
		float y = (float)j / 256;
		for (int i = 0; i < 256; i++){
			float x = (float)i / 256;
			float dist = sqrt(x * x + y * y);
			float coeff = cos(M_PI * dist * 0.5f);
			blurred[i + j * 256] = (uint8_t)(std::max(0.0f, coeff * 255));
		}
	}

	blurredBlob = new Texture(GL_ALPHA8, GL_ALPHA, GL_UNSIGNED_BYTE, 256, 256, blurred, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, anisoLevel);
	free(blurred);

	TextRender::init();
	Frame::init();

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);

	//glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	//get some aa going
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_MULTISAMPLE_ARB);

	glEnable(GL_DEPTH_TEST);
	CHECK_GL_ERROR

	//build UI
	//create all-encompassing containers for each state
	for(int i = 0; i < NUM_STATES; i++){
		stateElems[i] = new UIElem(vec2i(0, 0));
	}

	pieceListInit();
	editingInit();
	solvingInit();
	solutionInit();

	stateUpdate[STATE_LIST_PIECES] = &App::pieceListingUpdate;
	stateUpdate[STATE_EDIT_PIECE] = &App::editingUpdate;
	stateUpdate[STATE_SOLVE] = &App::solvingUpdate;
	stateUpdate[STATE_SHOW_RESULT] = &App::solutionUpdate;

	stateOverlay[STATE_LIST_PIECES] = &App::pieceListingOverlay;
	stateOverlay[STATE_EDIT_PIECE] = &App::editingOverlay;
	stateOverlay[STATE_SOLVE] = &App::solvingOverlay;
	stateOverlay[STATE_SHOW_RESULT] = &App::solutionOverlay;

	//ignition
	state = STATE_LIST_PIECES;
}

void App::end(){
	//delete all user interface elements
	for (int i = 0; i < NUM_STATES; i++){
		delete stateElems[i];
	}

	Frame::end();
}

void App::drawCube(int x, int y, int z){
	//top
	glVertex3f(x    , y + 1, z    );
	glVertex3f(x + 1, y + 1, z    );
	glVertex3f(x + 1, y + 1, z + 1);
	glVertex3f(x    , y + 1, z + 1);

	//bottom
	glVertex3f(x    , y,     z);
	glVertex3f(x + 1, y,     z);
	glVertex3f(x + 1, y,     z + 1);
	glVertex3f(x    , y,     z + 1);

	//left
	glVertex3f(x    , y    , z);
	glVertex3f(x    , y    , z + 1);
	glVertex3f(x    , y + 1, z + 1);
	glVertex3f(x    , y + 1, z);

	//right
	glVertex3f(x + 1, y    , z);
	glVertex3f(x + 1, y    , z + 1);
	glVertex3f(x + 1, y + 1, z + 1);
	glVertex3f(x + 1, y + 1, z);

	//front
	glVertex3f(x    , y    , z);
	glVertex3f(x + 1, y    , z);
	glVertex3f(x + 1, y + 1, z);
	glVertex3f(x    , y + 1, z);

	//back
	glVertex3f(x    , y    , z + 1);
	glVertex3f(x + 1, y    , z + 1);
	glVertex3f(x + 1, y + 1, z + 1);
	glVertex3f(x    , y + 1, z + 1);
}

void App::drawCubeOutlines(int x, int y, int z){
	glVertex3f(x    , y    , z    );
	glVertex3f(x + 1, y    , z    );

	glVertex3f(x    , y    , z    );
	glVertex3f(x    , y    , z + 1);

	glVertex3f(x + 1, y    , z    );
	glVertex3f(x + 1, y    , z + 1);

	glVertex3f(x    , y    , z + 1);
	glVertex3f(x + 1, y    , z + 1);


	glVertex3f(x    , y + 1, z    );
	glVertex3f(x + 1, y + 1, z    );

	glVertex3f(x    , y + 1, z    );
	glVertex3f(x    , y + 1, z + 1);

	glVertex3f(x + 1, y + 1, z    );
	glVertex3f(x + 1, y + 1, z + 1);

	glVertex3f(x    , y + 1, z + 1);
	glVertex3f(x + 1, y + 1, z + 1);


	glVertex3f(x    , y    , z    );
	glVertex3f(x    , y + 1, z    );

	glVertex3f(x + 1, y    , z    );
	glVertex3f(x + 1, y + 1, z    );

	glVertex3f(x    , y    , z + 1);
	glVertex3f(x    , y + 1, z + 1);

	glVertex3f(x + 1, y    , z + 1);
	glVertex3f(x + 1, y + 1, z + 1);
}

unsigned int App::timeMilli() {
	#ifdef UNIX_LIKE
	struct timeval t;
	gettimeofday(&t, NULL);
	return (int) (t.tv_sec * 1000 + t.tv_usec / 1000);
	
	#else //windoze
	return (int) GetTickCount();
	#endif
}

void App::reshape(int w, int h){
	width = w;
	height = h;
	glViewport(0, 0, width, height);
}

void App::update(){
	int currentTime = timeMilli();

	//update for each state
	(this->*stateUpdate[state])();

	std::copy(mouseDown, mouseDown + 3, lastMouseDown);

	//UI, set up ortho projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1.0, 1.0);
	CHECK_GL_ERROR

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	CHECK_GL_ERROR
	
	const int labelBufLength = 256;
	char labelBuf[labelBufLength];
	int metricW, metricH;
	//debug strings

	//draw UI
	glDisable(GL_DEPTH_TEST);
	stateElems[state]->draw();

	(this->*stateOverlay[state])();

	//status label
	glColor4f(1.0f, 1.0f, 0.5f, 1.0f);
	snprintf(labelBuf, labelBufLength, "state: %s", stateNames[state]);
	TextRender::render(labelBuf, 10, height - 100);

	//element hovering over
	if (elemHover){
		std::string name = demangledTypeName(elemHover);
		sprintf(labelBuf, "hovering over a %s", name.c_str());
		TextRender::render(labelBuf, 10, height - 20);
	}

	glutSwapBuffers();
	GLerror::list("after swap buffers");
}

void App::keyboardDown(int key, int x, int y){
	printf("keyboard down %d\n", key);

	if (UIElem::focus){
		int keyMods = glutGetModifiers();
		UIElem::focus->onTextKey(key, keyMods);
	}
}

void App::keyboardUp(int key, int x, int y){
	//nothing so far
}

void App::specialDown(int key, int x, int y){
	printf("keyboard special down %d\n", key);

	if (UIElem::focus){
		UIElem::focus->onControlKey(key);
	}
}

void App::specialUp(int key, int x, int y){
}

void App::mouseButton(int key, int state, int x, int y){
	printf("mouse button %d, %d, on %s\n", key, state, demangledTypeName(elemHover).c_str());
	mouse.set(x, y);

	switch(key){
		case GLUT_LEFT_BUTTON:{
			//place focus on element
			if (state == GLUT_DOWN){
				UIElem::focus = elemHover;
			}

			//note the lack of break
		}
		case GLUT_MIDDLE_BUTTON:
		case GLUT_RIGHT_BUTTON: {
			if (state == GLUT_DOWN){
				mouseDown[key] = true;
				mouseButtonDownOn[key] = elemHover;
				mouseButtonDownAt[key] = mouse;

				if (elemHover){
					elemHover->onMouseDown(mouse, key);
				}
			}

			else{ //GLUT_UP
				if (mouseDown[key]){
					mouseDown[key] = false;
					mouseButtonDownOn[key] = nullptr;

					if (elemHover){
						elemHover->onMouseUp(mouse, key);
					}
				}
			}

			break;
		}

		case 3:{ //wheel up
			if (state == GLUT_DOWN){
				if (elemHover){
					elemHover->onWheel(mouse, 1);
				}
			}

			break;
		}

		case 4:{ //wheel down
			if (state == GLUT_DOWN){
				if (elemHover){
					elemHover->onWheel(mouse, -1);
				}
			}

			break;
		}
	}
}

//this is used for both active (with button held) and passive mouse motion
void App::mouseMotion(int x, int y){
	oldMouse = mouse;
	mouse.set(x, y);

	elemHover = stateElems[state]->collides(mouse);
	
	for (int key = 0; key < 3; key++){
		if (mouseDown[key]){
			if (mouseButtonDownOn[key]){
				mouseButtonDownOn[key]->onMouseDrag(mouse, mouseButtonDownAt[key], key);
			}
		}
	}
}

void App::idle(){
	//cap framerate to 60
	unsigned int start = timeMilli();
	glutPostRedisplay();
	unsigned int drawDuration = timeMilli() - start;
	
	if (drawDuration < 17){
		#ifdef UNIX_LIKE
		usleep
		#else
		Sleep
		#endif
		((17 - drawDuration) * 1000);
	}
}

