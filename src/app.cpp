#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "app.h"

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
	
	/*
	//create some kind of texture for blocks
	const int textureBufferWidth = 256;
	const int textureBufferHeight = 256;
	uint8_t* textureBuffer = (uint8_t*) malloc(textureBufferWidth * textureBufferHeight * 3);
	
	TextureGen::xorGrid(textureBufferWidth, textureBufferHeight, textureBuffer);
	blockTexture = new Texture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, textureBufferWidth, textureBufferHeight, textureBuffer, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, anisoLevel);

	TextureGen::bubbles(textureBufferWidth, textureBufferHeight, 40, 70.0f, textureBuffer);
	bubbleTexture = new Texture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, textureBufferWidth, textureBufferHeight, textureBuffer, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, anisoLevel);
	free(textureBuffer);*/

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
	
	//reset input
	mousePressed[0] = mousePressed[1] = mousePressed[2] = false;
	mousePressedEaten[0] = mousePressedEaten[1] = mousePressedEaten[2] = false;
	keyControl = keyAlt = keyShift = false;
	mouse[0] = mouse[1] = 0;

	//reset state data

	pieces.push_back(new Piece());
	state = 0;
	editedPiece = pieces.back();
	editingLayer = 0;
	blocksLaid = 0;

	//build UI
	pieceListFrame = new ScrollingFrame(10, 10, 500);
	listingStateElems = new UIElem();
	listingStateElems->addChild(pieceListFrame);

	editorFrame = new Frame(600, 10, 180, 50);
	discardButton = new Button(0, 0, (char*)"discard");
	saveButton = new Button(70, 0, (char*)"save");
	undoButton = new Button(120, 0, (char*)"undo");

	editorFrame->addChild(discardButton);
	editorFrame->addChild(saveButton);
	editorFrame->addChild(undoButton);

	editorElems = new UIElem();
	editorElems->addChild(editorFrame);
	
	state = STATE_EDIT_PIECE; //temporary

	//3d camera
	cameraTurn = 0.0f;
	cameraDive = (CAMERA_MIN_DIVE + CAMERA_MAX_DIVE) / 2.0f;
	cameraDist = 15.0f;
}

bool App::checkButton(int button){
	if (mousePressed[button] && !mousePressedEaten[button]){
		mousePressedEaten[button] = true;
		return true;
	}

	else return false;
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

	//set up perspective projection
	double projMatrix[16];

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80.0, (double)width / (double)height, 1e-2, 1e3);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //normal

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	//compute eye position
	//dive
	cameraEye.z = 1.0f * sin(cameraDive);
	cameraEye.y = 1.0f * cos(cameraDive);

	//turn
	cameraEye.x = -cameraEye.z * sin(cameraTurn);
	cameraEye.z =  cameraEye.z * cos(cameraTurn);

	//distance
	cameraEye.x *= cameraDist;
	cameraEye.y *= cameraDist;
	cameraEye.z *= cameraDist;

	gluLookAt(cameraEye.x, cameraEye.y, cameraEye.z,
	          0.0, 0.0, 0.0,  //target
	          0.0, 1.0, 0.0); //up

	double modelViewMatrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//compute camera to scene ray
	double rayStartX, rayStartY, rayStartZ;
	double rayEndX, rayEndY, rayEndZ;
	gluUnProject(mouse[0], height - mouse[1], 0.0f, modelViewMatrix, projMatrix, viewport, &rayStartX, &rayStartY, &rayStartZ);
	gluUnProject(mouse[0], height - mouse[1], 1.0f, modelViewMatrix, projMatrix, viewport, &rayEndX, &rayEndY, &rayEndZ);
	mouseRayDir = (vec3(rayEndX, rayEndY, rayEndZ) - vec3(rayStartX, rayStartX, rayStartZ)).normal();

	//3d view
	switch(state){
		case STATE_EDIT_PIECE: {
			//intersect the mouse ray with the piece matter
			if (mouse[0] != oldMouse[0] || mouse[1] != oldMouse[1]) checkRayCollision = true;

			if (checkRayCollision){
				//Piece::block extrudedBlock;
				editorCollision = editedPiece->collisionCheck(cameraEye, mouseRayDir, lastCollision);
				checkRayCollision = false;
			}

			if (checkButton(GLUT_LEFT_BUTTON)){
				
				printf("adding block at (%d %d %d)\n", extrudedBlock.x, extrudedBlock.y, extrudedBlock.z);
				editedPiece->blocks.push_back(extrudedBlock);
			}

			else if (checkButton(GLUT_RIGHT_BUTTON) && lastCollision.blockIt != editedPiece->blocks.end()){
				printf("removing block at (%d %d %d)\n", lastCollision.blockIt->x, lastCollision.blockIt->y, lastCollision.blockIt->z);
				editedPiece->blocks.erase(lastCollision.blockIt);
			}

			int gridCenterX, gridCenterZ;

			if (editorCollision){
				//offset the block from its source depending on which side it's being extruded coming from
				extrudedBlock = *(lastCollision.blockIt);
				extrudedBlock.data[lastCollision.side >> 1] += ((lastCollision.side & 1) == 0)? -1 : 1;
			}
			else{
				//no collision, so intersect mouse ray with the grid
				float h = editingLayer - cameraEye.y;
				vec3 gridRayPoint = cameraEye + mouseRayDir * (h / mouseRayDir.y);

				int gpx = floor(gridRayPoint.x);
				int gpz = floor(gridRayPoint.z);

				extrudedBlock.x = gpx;
				extrudedBlock.y = editingLayer;
				extrudedBlock.z = gpz;
			}

			gridCenterX = extrudedBlock.x;
			gridCenterZ = extrudedBlock.z;

			//draw the grid
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glLineWidth(2.0f);

			blurredBlob->bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

			glEnable(GL_BLEND);
			glBegin(GL_LINES);

			for (int i = -FLOATING_GRID_EXTENT; i < FLOATING_GRID_EXTENT; i++){
				int gx = gridCenterX + i;
				int gz0 = gridCenterZ - FLOATING_GRID_EXTENT;
				int gz1 = gridCenterZ + FLOATING_GRID_EXTENT;

				glTexCoord2f((float)i / FLOATING_GRID_EXTENT, -1.0f); glVertex3i(gx, editingLayer, gz0);
				glTexCoord2f((float)i / FLOATING_GRID_EXTENT,  1.0f); glVertex3i(gx, editingLayer, gz1);
			}

			for (int i = -FLOATING_GRID_EXTENT; i < FLOATING_GRID_EXTENT; i++){
				int gx0 = gridCenterX - FLOATING_GRID_EXTENT;
				int gx1 = gridCenterX + FLOATING_GRID_EXTENT;
				int gz = gridCenterZ + i;

				glTexCoord2f(-1.0f, (float)i / FLOATING_GRID_EXTENT); glVertex3i(gx0, editingLayer, gz);
				glTexCoord2f( 1.0f, (float)i / FLOATING_GRID_EXTENT); glVertex3i(gx1, editingLayer, gz);
			}

			glEnd();

			glDisable(GL_TEXTURE_2D);


			//draw current piece, pushing the solid planes it a little back so that the lines will show, even at acute angles
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.0f, 1.0f);

			glBegin(GL_QUADS);
			for (auto it = editedPiece->blocks.begin(); it != editedPiece->blocks.end(); it++){
				vec3i& b = *it;
				if (it == lastCollision.blockIt) glColor4f(0.5f, 0.9f, 0.5f, 1.0f);
				else glColor4f(0.5f, 0.5f, 0.9f, 1.0f);
				drawCube(b.x, b.y, b.z);
			}
			glEnd();
			glDisable(GL_POLYGON_OFFSET_FILL);

			glLineWidth(3.0f);
			glBegin(GL_LINES);
			for (auto it = editedPiece->blocks.begin(); it != editedPiece->blocks.end(); it++){
				vec3i& b = *it;
				if (it == lastCollision.blockIt) glColor4f(0.7f, 0.9f, 0.7f, 1.0f);
				else glColor4f(0.7f, 0.7f, 0.9f, 1.0f);
				drawCubeOutlines(b.x, b.y, b.z);
			}
			glEnd();

			//also draw the extruded block
			glEnable(GL_BLEND);
			glBegin(GL_QUADS);
			glColor4f(0.7f, 0.4f, 0.4f, 0.5f);
			drawCube(extrudedBlock.x, extrudedBlock.y, extrudedBlock.z);

			glEnd();
			break;
		}

		case STATE_SHOW_RESULT:{
			break;
		}

		//others, no 3d view
	}


	//origin arrows, x-ray
	glDisable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS);
	glLineWidth(4.0f);
	glBegin(GL_LINES);
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 0.0f, 0.0f);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, 1.0f, 0.0f);

	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0f, 0.0f, 1.0f);
	glEnd();
	CHECK_GL_ERROR

	//=======================================================
	// switch to UI, set up ortho projection
	//=======================================================
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

	switch(state){
		case STATE_LIST_PIECES: {
			listingStateElems->draw();
			break;
		}

		case STATE_EDIT_PIECE: {
			editorElems->draw();
			break;
		}

		case STATE_SOLVE: {
			solvingElems->draw();
			break;
		}

		case STATE_SHOW_RESULT:{
			solutionElems->draw(); 
			break;
		}
	}

	//status label
	glColor4f(1.0f, 1.0f, 0.5f, 1.0f);
	snprintf(labelBuf, labelBufLength, "state: %s", stateNames[state]);
	TextRender::render(labelBuf, 10, height - 100);

	//eye position
	sprintf(labelBuf, "eye at (%.3f, %.3f, %.3f)", cameraEye.x, cameraEye.y, cameraEye.z);
	TextRender::render(labelBuf, 10, height - 80);

	//mouse ray dir position
	sprintf(labelBuf, "mouse dir (%.3f, %.3f, %.3f)", mouseRayDir.x, mouseRayDir.y, mouseRayDir.z);
	TextRender::render(labelBuf, 10, height - 60);

	//camera rotation
	sprintf(labelBuf, "turn %.2f, dive %.2f", cameraTurn, cameraDive);
	TextRender::render(labelBuf, 10, height - 40);


	glutSwapBuffers();
	GLerror::list("after swap buffers");
}



void App::keyboard(uint8_t key, int x, int y){
	int keyMods = glutGetModifiers();
	keyControl = keyMods & GLUT_ACTIVE_CTRL;
	keyAlt = keyMods & GLUT_ACTIVE_ALT;
	keyShift = keyMods & GLUT_ACTIVE_SHIFT;

	switch(key){
		case 27: //escape
		case 'q': 
			glutLeaveMainLoop();
			break;
	}
}

void App::mouseButton(int key, int state, int x, int y){
	switch(key){
		case GLUT_LEFT_BUTTON: 
			mousePressed[0] = (state == GLUT_DOWN); 
			mousePressedEaten[0] = false;
			break;
			
		case GLUT_MIDDLE_BUTTON: 
			mousePressed[1] = (state == GLUT_DOWN);
			mousePressedEaten[1] = false;
			break;
			
		case GLUT_RIGHT_BUTTON: 
			mousePressed[2] = (state == GLUT_DOWN);
			mousePressedEaten[2] = false;

			if (state == GLUT_DOWN){
				//drag start
				xDragBase = x;
				yDragBase = y;
				cameraTurnDragBase = cameraTurn;
				cameraDiveDragBase = cameraDive;
				cameraDistDragBase = cameraDist;
			}
			else{
				//drag end, angle wrap $cameraTurn
				cameraTurn = fmodf(cameraTurn, M_PI * 2);
			}
			break;

		case 3: //wheel up
			if (state == GLUT_DOWN){
				cameraDist -= log10(cameraDist);

				if (cameraDist < CAMERA_DIST_MIN) cameraDist = CAMERA_DIST_MIN;
			}

			break;

		case 4: //wheel down
			if (state == GLUT_DOWN)
				cameraDist += log10(cameraDist);

			break;
	}
}

//this is used for both active (with button held) and passive mouse motion
void App::mouseMotion(int x, int y){
	oldMouse[0] = mouse[0];
	oldMouse[1] = mouse[1];

	mouse[0] = x;
	mouse[1] = y;
	
	if (mousePressed[2]){// && mousePressedEaten[2]){
		float dx = x - xDragBase;
		float dy = y - yDragBase;

		cameraTurn = cameraTurnDragBase + dx * 0.01f;
		cameraDive = cameraDiveDragBase + dy * 0.01f;

		if (cameraDive < CAMERA_MIN_DIVE) cameraDive = CAMERA_MIN_DIVE;
		if (cameraDive > CAMERA_MAX_DIVE) cameraDive = CAMERA_MAX_DIVE;
	}
}

void App::special(int key, int x, int y){
	printf("special callback: key = %d\n", key);
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

