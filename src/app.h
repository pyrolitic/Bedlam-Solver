#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
#define UNIX_LIKE
#endif 

#ifdef UNIX_LIKE
#include <sys/time.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <cmath>
#include <cstdlib>

#include <limits>
#include <list>
#include <vector>

#include <GL/glew.h>

#include "texture.h"
#include "text_render.h"
#include "vec3.h"

class App{
public:
	App(){}
	~App(){}

	App(const App&); //nope
	void operator =(const App&); //nope
	
	static App app; //singleton instance
	
	int width, height; //window dimensions
	int anisoLevel; //[1, 16]
	
	Texture* blockTexture;
	Texture* blurredBlob;
	
	int drawingAt;
	
	//user input
	bool mousePressed[3]; //actual mouse button state
	bool mousePressedEaten[3]; //whether a mouse button down event was acted upon already
	int oldMouse[2], mouse[2]; //pointer position, window pixel space
	bool keyControl, keyAlt, keyShift; //modifiers

	//mouselook
	int xDragBase, yDragBase; //window mouse coords
	float cameraTurn, cameraTurnDragBase; //radians, about the center, angle wrapped
	float cameraDive, cameraDiveDragBase; //radians
	float cameraDist, cameraDistDragBase; //from the center

	//pleasant limits, radians
	#define CAMERA_MIN_DIVE -2.3f
	#define CAMERA_MAX_DIVE -0.3f
	#define CAMERA_DIST_MIN 1.5f

	std::list<Piece*> pieces;

	//states
	#define STATE_LIST_PIECES 0
	#define STATE_EDIT_PIECE 1
	#define STATE_SOLVE 2
	#define STATE_SHOW_RESULT 3
	int state;
	const char* stateNames[4] = {"Pieces", "Editor", "Solving", "Result"};

	//world space
	vec3 cameraEye;
	vec3 mouseRayDir; 

	//

	//piece editing state
	#define FLOATING_GRID_EXTENT 3 //both width and height, in either direction from the center
	Piece* editedPiece;
	int editingLayer; //which layer currently editing
	bool blocksLaid; //how many blocks is the peice made of
	bool checkRayCollision; //wether to check for block collision (if the mouse moved or if a block was laid or deleted)

	std::list<Piece::block>::iterator collidedBlockIt; //I'm told this gets initialized to the list::end() value
	vec3 collisionPoint;
	Piece::block extrudedBlock;
	int collisionAxis;

	
	void init(){
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
	
		//load font texture
		TextRender::init();
		
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
		state = STATE_EDIT_PIECE; //temporary

		//3d camera
		cameraTurn = 0.0f;
		cameraDive = (CAMERA_MIN_DIVE + CAMERA_MAX_DIVE) / 2.0f;
		cameraDist = 15.0f;
	}
	
	void reshape(int w, int h){
		width = w;
		height = h;
		glViewport(0, 0, width, height);
	}

	bool checkButton(int button){
		if (mousePressed[button] && !mousePressedEaten[button]){
			mousePressedEaten[button] = true;
			return true;
		}

		else return false;
	}

	void update(){
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
					//O(n), TODO: make it better
					collidedBlockIt = editedPiece->blocks.end();
					float closestCollisionT = std::numeric_limits<float>::max();

					for (int axis = 0; axis < 3; axis++){ //YZ, XZ, XY planes
						for(auto it = editedPiece->blocks.begin(); it != editedPiece->blocks.end(); it++){
							Piece::block& b = *it;
							int* blockData = (int*)(&b.x);

							//check either side of each axis
							for (int i = 0; i < 2; i++){
								int v = blockData[axis] + i;

								//distance to collision with plane (offset in the axis direction by $v units)
								float t = (v - cameraEye.data[axis]) / mouseRayDir.data[axis];

								if (t > 0.0f && t != std::numeric_limits<float>::max()){ //sanity check
									float h = v - cameraEye.data[axis];
									vec3 p = cameraEye + mouseRayDir * (h / mouseRayDir.data[axis]);
									const int others[] = {1, 2, 0, 2, 0, 1}; //this better be optimized out
									int a2 = others[axis * 2 + 0];
									int a3 = others[axis * 2 + 1];

									//check that the hit is within the plane
									if (p.data[a2] >= blockData[a2] && p.data[a2] < blockData[a2] + 1 && p.data[a3] >= blockData[a3] && p.data[a3] < blockData[a3] + 1){
										//hit
										if (t < closestCollisionT){
											collidedBlockIt = it;
											closestCollisionT = t;

											collisionAxis = axis * 2 + i;
											extrudedBlock.x = it->x;
											extrudedBlock.y = it->y;
											extrudedBlock.z = it->z;

											//offset the block from its source depending on which side it's being extruded coming from
											((int*)&extrudedBlock)[axis] += (i == 0)? -1 : 1;
										}
									}
								}
							}
						}
					}

					checkRayCollision = false;
				}

				if (checkButton(GLUT_LEFT_BUTTON)){
					printf("adding block at (%d %d %d)\n", extrudedBlock.x, extrudedBlock.y, extrudedBlock.z);
					editedPiece->blocks.push_back(extrudedBlock);
				}

				else if (checkButton(GLUT_RIGHT_BUTTON) && collidedBlockIt != editedPiece->blocks.end()){
					printf("removing block at (%d %d %d)\n", collidedBlockIt->x, collidedBlockIt->y, collidedBlockIt->z);
					editedPiece->blocks.erase(collidedBlockIt);
				}

				int gridCenterX, gridCenterZ;

				if (collidedBlockIt == editedPiece->blocks.end()){
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


				//draw current piece
				glBegin(GL_QUADS);
				for (auto it = editedPiece->blocks.begin(); it != editedPiece->blocks.end(); it++){
					Piece::block& b = *it;

					if (it == collidedBlockIt) glColor4f(0.5f, 0.9f, 0.5f, 1.0f);
					else glColor4f(0.5f, 0.5f, 0.9f, 1.0f);
					drawCube(b.x, b.y, b.z);
				}
				glEnd();

				glDepthFunc(GL_LEQUAL);
				glBegin(GL_LINES);
				for (auto it = editedPiece->blocks.begin(); it != editedPiece->blocks.end(); it++){
					Piece::block& b = *it;

					if (it == collidedBlockIt) glColor4f(0.7f, 0.9f, 0.7f, 1.0f);
					else glColor4f(0.7f, 0.7f, 0.9f, 1.0f);
					drawCubeOutlines(b.x, b.y, b.z);
				}
				glEnd();
				glDepthFunc(GL_LESS);

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
		//glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
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

		//status label
		glPushMatrix();
		glColor4f(1.0f, 1.0f, 0.5f, 1.0f);
		glTranslatef(10, 10, 0);
		snprintf(labelBuf, labelBufLength, "%s", stateNames[state]);
		TextRender::render(labelBuf);
		glPopMatrix();

		//eye position
		glPushMatrix();
		glTranslatef(10, 25, 0);
		sprintf(labelBuf, "eye at (%f, %f, %f)", cameraEye.x, cameraEye.y, cameraEye.z);
		TextRender::render(labelBuf);
		glPopMatrix();

		//mouse ray dir position
		glPushMatrix();
		glTranslatef(10, 40, 0);
		sprintf(labelBuf, "mouse dir (%f, %f, %f)", mouseRayDir.x, mouseRayDir.y, mouseRayDir.z);
		TextRender::render(labelBuf);
		glPopMatrix();

		//camera rotation
		glPushMatrix();
		glTranslatef(10, 55, 0);
		sprintf(labelBuf, "turn %f, dive %f", cameraTurn, cameraDive);
		TextRender::render(labelBuf);
		glPopMatrix();

		glutSwapBuffers();
		GLerror::list("after swap buffers");
	}

	void drawCube(int x, int y, int z){
		//top
		glVertex3f(x    , y + 1, z    );
		glVertex3f(x + 1, y + 1, z    );
		glVertex3f(x + 1, y + 1, z + 1);
		glVertex3f(x    , y + 1, z + 1);

		//bottom
		glVertex3f(x    , y,     z    );
		glVertex3f(x + 1, y,     z    );
		glVertex3f(x + 1, y,     z + 1);
		glVertex3f(x    , y,     z + 1);

		//left
		glVertex3f(x    , y,     z    );
		glVertex3f(x    , y,     z + 1);
		glVertex3f(x    , y + 1, z + 1);
		glVertex3f(x    , y + 1, z    );

		//right
		glVertex3f(x + 1, y,     z    );
		glVertex3f(x + 1, y,     z + 1);
		glVertex3f(x + 1, y + 1, z + 1);
		glVertex3f(x + 1, y + 1, z    );

		//front
		glVertex3f(x    , y,     z    );
		glVertex3f(x + 1, y,     z    );
		glVertex3f(x + 1, y + 1, z    );
		glVertex3f(x    , y + 1, z    );

		//back
		glVertex3f(x    , y,     z + 1);
		glVertex3f(x + 1, y,     z + 1);
		glVertex3f(x + 1, y + 1, z + 1);
		glVertex3f(x    , y + 1, z + 1);
	}

	void drawCubeOutlines(int x, int y, int z){
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

	void keyboard(uint8_t key, int x, int y){
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

	void mouseButton(int key, int state, int x, int y){
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
	void mouseMotion(int x, int y){
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

	void special(int key, int x, int y){
		printf("special callback: key = %d\n", key);
	}

	void idle(){
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
	
	static unsigned int timeMilli() {
		#ifdef UNIX_LIKE
		struct timeval t;
		gettimeofday(&t, NULL);
		return (int) (t.tv_sec * 1000 + t.tv_usec / 1000);
		
		#else //windoze
		return (int) GetTickCount();
		#endif
	}
};



