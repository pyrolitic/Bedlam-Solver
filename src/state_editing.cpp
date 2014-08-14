#include <GL/glew.h>
#include <GL/freeglut.h>

#include "app.h"
#include "ui/linear_container.h"

static const blockVert defaultBlock[] = {
	//front
	{{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//back
	{{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//left
	{{0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//right
	{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//top
	{{0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	//bottom
	{{0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},

	{{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}},
	{{1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0, 0, 0, 255}}
};

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

EditingScreen::EditingScreen(){
	//texture
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

	blurredBlob = new Texture(1, GL_UNSIGNED_BYTE, 256, 256, blurred, GL_LINEAR, GL_LINEAR, anisoLevel);
	free(blurred);

	//mesh
	blocksVAO = new VertexArrayObject<blockVert>();
	singleBlockVAO = new VertexArrayObject<blockVert>();
	gridVAO = new VertexArrayObject<blockVert>();

	blockVert* v = new blockVert[24];
	memcpy(v, defaultBlock, 24 * sizeof(blockVert));
	for (int i = 0; i < 24; i++){
		const uint8_t c[] = {0xB2, 0x66, 0x66, 0x7F};
		memcpy(v[i].col, c, 4);
	}
	singleBlockVAO->assign(24, v);
	delete [] v;
	
	//UI
	cameraControl = new CameraControl();
	rootUI = (UIElem*) cameraControl;
	cameraControl->bindMouseMove(this, &EditingScreen::mouseMoveCallback);
	cameraControl->bindMouseDrag(this, &EditingScreen::mouseDragCallback);

	discardButton = new Button((char*)"discard");
	discardButton->bindMouseUp(this, &EditingScreen::discardButtonCallback);
	discardButton->setColor(DISCARD_BUTTON_COLOR);

	saveButton = new Button((char*)"save");
	saveButton->bindMouseUp(this, &EditingScreen::saveButtonCallback);
	saveButton->setColor(SAVE_BUTTON_COLOR);

	undoButton = new Button((char*)"undo");
	undoButton->bindMouseUp(this, &EditingScreen::undoButtonCallback);
	undoButton->setColor(UNDO_BUTTON_INACTIVE_COLOR);

	Frame* buttonFrame = new Frame();
	buttonFrame->setPosition(vec2i(20, 20));

	LinearContainer* layout = new LinearContainer(LINEAR_CONTAINER_HORIZONTAL);
	layout->addChild(discardButton);
	layout->addChild(saveButton);
	layout->addChild(undoButton);

	buttonFrame->addChild(layout);
	rootUI->addChild(buttonFrame);

	//debug
	LinearContainer* debugList = new LinearContainer(LINEAR_CONTAINER_VERTICAL);
	rootUI->addChild(debugList);

	eyeLabel = new Label();
	mouseRayLabel = new Label();
	cameraAngleLabel = new Label();

	debugList->addChild(eyeLabel);
	debugList->addChild(mouseRayLabel);
	debugList->addChild(cameraAngleLabel);

	//state
	parentRef = nullptr;
}

EditingScreen::~EditingScreen(){
	delete blurredBlob;
	delete blocksVAO;
}

void EditingScreen::transitionWithPiece(listPieceEntry* ref, bool pieceIsNew){
	parentRef = ref;
	tempPiece = *(ref->itemIt); //copy
	newPiece = pieceIsNew;

	state = STATE_EDIT_PIECE;
	cleanInput();
}


void EditingScreen::discardButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		printf("clicked on the discard button\n");
		//don't change anything, just discard the changes in $editPiece
		history.clear();
		if (newPiece){
			//delete it
			delete parentRef;
		}

		PieceListScreen::getInstance().transition();
	}
}

void EditingScreen::saveButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		printf("clicked on the save button\n");
		//copy the changes and change state
		*(parentRef->itemIt) = tempPiece; //copy back
		history.clear();

		PieceListScreen::getInstance().transition();
	}
}

void EditingScreen::undoButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		printf("clicked on the undo button\n");

		if (!history.empty()){
			const pieceChange& step = history.back();
			if (step.removal){
				tempPiece.insert(step.at);
			}
			else{
				tempPiece.remove(step.at);
			}

			history.pop_back();

			if (history.empty()){
				undoButton->setColor(UNDO_BUTTON_INACTIVE_COLOR);
			}
		}
	}
}

void EditingScreen::mouseUpCallback(UIElem* context, vec2i at, int button){
	if (collision){
		if (button == GLUT_LEFT_BUTTON){
			printf("adding block at (%d %d %d)\n", extrudedBlock.x, extrudedBlock.y, extrudedBlock.z);
			tempPiece.insert(extrudedBlock);
			history.emplace_back(false, extrudedBlock);
			rebuildBlockVAO();

			undoButton->setColor(UNDO_BUTTON_ACTIVE_COLOR);
			checkCollision();
		}

		else if (button == GLUT_RIGHT_BUTTON){
			printf("removing block at (%d %d %d)\n", lastCollision.block.x, lastCollision.block.y, lastCollision.block.z);
			tempPiece.remove(lastCollision.block);
			history.emplace_back(true, lastCollision.block);
			rebuildBlockVAO();

			undoButton->setColor(UNDO_BUTTON_ACTIVE_COLOR);
			checkCollision();
		}
	}
}

void EditingScreen::checkCollision(){
	collision = tempPiece.collisionCheck(cameraEye, mouseRayDir, lastCollision);
	tooFar = false; //limit additions to EDITING_MAX_DIST_BLOCK

	if (collision){
		//offset the block from its source depending on which side it's being extruded coming from
		extrudedBlock = lastCollision.block;
		extrudedBlock.data[lastCollision.side >> 1] += ((lastCollision.side & 1) == 0)? -1 : 1;
	}
	else{
		//no collision, so intersect mouseInfo.mouse ray with the grid
		float h = /*editingLayer*/ - cameraEye.y;
		vec3 gridRayPoint = cameraEye + mouseRayDir * (h / mouseRayDir.y);

		int gpx = floor(gridRayPoint.x);
		int gpz = floor(gridRayPoint.z);

		extrudedBlock.x = gpx;
		extrudedBlock.y = 0;//editingLayer;
		extrudedBlock.z = gpz;
	}

	if (!(extrudedBlock.abs() < vec3i(EDITING_MAX_DIST_BLOCK))){
		tooFar = true;
	}
}

void EditingScreen::mouseMoveCallback(UIElem* context, vec2i to){
	//check for mouse ray collision against the blocks
	mat4 invPVM = (projection * modelView).inverse();
	vec3f far = invPVM * vec3f((2.0f * to.x) / windowResolution.x - 1.0f,
	                           (2.0f * to.y) / windowResolution.y - 1.0f, 
	                           (2 * 1.0f) - 1.0f);
	mouseRayDir = (cameraEye - far).normal();
	checkCollision();
}

void EditingScreen::mouseDragCallback(UIElem* context, vec2i from, vec2i to, int button){
	cameraEye = cameraControl->getEyePosition();
	modelView = mat4::viewLookAt(cameraEye, vec3f(0.0f), vec3f(0.0f, 1.0f, 0.0f));
}

void EditingScreen::rebuildBlockVAO(){
	int count = tempPiece.mass();
	blockVert* buf = new blockVert[count * 6 * 6]; //for every of the 6 faces there must be two triangles
	blockVert* v = buf;

	for (auto block : tempPiece.getBlocks()){
		const vec3i& pos = block.first;
		memcpy(v, defaultBlock, sizeof(defaultBlock));
		for (int i = 0; i < 6; i++){
			v[i].position += vec3f(pos);

			if (pos == lastCollision.block and collision){
				const uint8_t c[] = {0x7F, 0xE5, 0x7F, 0xFF};
				memcpy(v[i].col, c, 4);
			} 
			else{
				const uint8_t c[] = {0x7F, 0x7F, 0xE5, 0xFF};
				memcpy(v[i].col, c, 4);
			}
		}

		v += 6;
	}
}



void EditingScreen::update(){
	//set up perspective projection
	projection = mat4::perspectiveProjection(80.0f, (float)windowResolution.x / (float)windowResolution.y, 1e-2f, 1e3f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //normal
	glDisable(GL_BLEND);

	//3d view
	int gridCenterX, gridCenterZ;
	gridCenterX = extrudedBlock.x;
	gridCenterZ = extrudedBlock.z;

	/*//draw the grid
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(2.0f);

	blurredBlob->bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	if (!collision and !tooFar){
		glEnable(GL_BLEND);
		glBegin(GL_LINES);

		for (int i = -FLOATING_GRID_EXTENT; i < FLOATING_GRID_EXTENT; i++){
			int gx = gridCenterX + i;
			int gz0 = gridCenterZ - FLOATING_GRID_EXTENT;
			int gz1 = gridCenterZ + FLOATING_GRID_EXTENT;

			glTexCoord2f((i + 0.0f) / FLOATING_GRID_EXTENT, -1.0f); glVertex3i(gx, 0, gz0);
			glTexCoord2f((i + 0.0f) / FLOATING_GRID_EXTENT,  1.0f); glVertex3i(gx, 0, gz1);
		}

		for (int i = -FLOATING_GRID_EXTENT; i < FLOATING_GRID_EXTENT; i++){
			int gx0 = gridCenterX - FLOATING_GRID_EXTENT;
			int gx1 = gridCenterX + FLOATING_GRID_EXTENT;
			int gz = gridCenterZ + i;

			glTexCoord2f(-1.0f, (i + 0.0f) / FLOATING_GRID_EXTENT); glVertex3i(gx0, 0, gz);
			glTexCoord2f( 1.0f, (i + 0.0f) / FLOATING_GRID_EXTENT); glVertex3i(gx1, 0, gz);
		}

		glEnd();
	}*/

	//draw current piece, pushing the solid planes back a little so that the lines will show, even at acute angles
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	//draw solid cubes
	glDisable(GL_POLYGON_OFFSET_FILL);
	//draw lines
	glEnd();

	if (!tooFar){
		//also draw the extruded block
		glEnable(GL_BLEND);
		glBegin(GL_QUADS);
		glColor4f(0.7f, 0.4f, 0.4f, 0.5f);
		glTranslatef(extrudedBlock.x, extrudedBlock.y, extrudedBlock.z);
		glutSolidCube(1.0);
		glTranslatef(-extrudedBlock.x, -extrudedBlock.y, -extrudedBlock.z);
		glEnd();
	}


	/*
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
	CHECK_GL_ERROR*/

	//debug info
	const int labelBufLength = 256;
	char labelBuf[labelBufLength];

	//eye position
	snprintf(labelBuf, labelBufLength, "eye at (%.3f, %.3f, %.3f)", cameraEye.x, cameraEye.y, cameraEye.z);
	eyeLabel->setText(labelBuf);

	//mouseInfo.mouse ray dir position
	snprintf(labelBuf, labelBufLength, "mouseInfo.mouse dir (%.3f, %.3f, %.3f)", mouseRayDir.x, mouseRayDir.y, mouseRayDir.z);
	mouseRayLabel->setText(labelBuf);

	//camera rotation
	vec3f turnDiveDist = cameraControl->getTurnDiveDist();
	snprintf(labelBuf, labelBufLength, "turn %.2f, dive %.2f", turnDiveDist.x, turnDiveDist.y);
	cameraAngleLabel->setText(labelBuf);
}
