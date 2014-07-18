#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "app.h"
#include "linear_container.h"

void App::discardButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		printf("clicked on the discard button\n");
		//don't change anything, just discard the changes in $editPiece
		editingHistory.clear();
		transition(STATE_LIST_PIECES);
		if (editingNewPiece){
			//delete it
			delete editedPieceParentRef;
		}
		pieceListCheckSolvingPossiblity();
	}
}

void App::saveButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		printf("clicked on the save button\n");
		//copy the changes and change state
		*(editedPieceParentRef->itemIt) = editedPiece; //copy back
		editingHistory.clear();
		transition(STATE_LIST_PIECES);
		pieceListCheckSolvingPossiblity();
	}
}

void App::undoButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		printf("clicked on the undo button\n");
		/*if (editingHistoryCurrentStep != editingHistory.end()){
			const pieceChange& step = *editingHistoryCurrentStep;
			if (step.removal){
				editedPiece.insert(step.at);
			}
			else{
				editedPiece.remove(step.at);
			}

			auto prev = std::prev(editingHistoryCurrentStep);
			if (prev != editingHistory.end()){
				editingHistoryCurrentStep = prev;
			}
		}*/
		if (editingHistory.size() != 0){
			const pieceChange& step = editingHistory.back();
			if (step.removal){
				editedPiece.insert(step.at);
			}
			else{
				editedPiece.remove(step.at);
			}

			editingHistory.pop_back();

			if (editingHistory.size() == 0){
				undoButton->setColor(0.3f, 0.3f, 0.3f, 1.0f);
			}
		}
	}
}

void App::editingInit(){
	//UI
	editingCameraControl = new CameraControl();
	stateElems[STATE_EDIT_PIECE] = editingCameraControl;

	discardButton = new Button(vec2i(0, 0), (char*)"discard", &App::discardButtonCallback);
	saveButton = new Button(vec2i(80, 0), (char*)"save", &App::saveButtonCallback);
	undoButton = new Button(vec2i(130, 0), (char*)"undo", &App::undoButtonCallback);

	LinearContainer* layout = new LinearContainer(vec2i(0, 0), LINEAR_CONTAINER_HORIZONTAL);
	layout->addChild(discardButton);
	layout->addChild(saveButton);
	layout->addChild(undoButton);

	discardButton->setColor(0.7f, 0.5f, 0.5f);
	saveButton->setColor(0.5f, 0.5f, 0.7f);
	undoButton->setColor(0.4f, 0.5f, 0.2f);

	//editorFrame = new Frame(vec2i(560, 10), layout->getSize() + 2 * vec2i(FRAME_ROUNDED_RADIUS));
	//editorFrame->addChild(layout);
	stateElems[STATE_EDIT_PIECE]->addChild(layout);//editorFrame);

	//state
	editedPieceParentRef = nullptr;
}

void App::editingUpdate(){
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

	editingCameraEye = editingCameraControl->getEyePosition();
	gluLookAt(editingCameraEye.x, editingCameraEye.y, editingCameraEye.z,
	          0.0, 0.0, 0.0,  //target
	          0.0, 1.0, 0.0); //up

	double modelViewMatrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	//compute camera to scene ray in world space
	double rayStartX, rayStartY, rayStartZ;
	double rayEndX, rayEndY, rayEndZ;
	gluUnProject(mouse.x, height - mouse.y, 0.0f, modelViewMatrix, projMatrix, viewport, &rayStartX, &rayStartY, &rayStartZ);
	gluUnProject(mouse.x, height - mouse.y, 1.0f, modelViewMatrix, projMatrix, viewport, &rayEndX, &rayEndY, &rayEndZ);
	mouseRayDir = (vec3(rayEndX, rayEndY, rayEndZ) - vec3(rayStartX, rayStartX, rayStartZ)).normal();

	//3d view
	//intersect the mouse ray with the piece matter
	if (mouse != oldMouse) checkRayCollision = true;

	if (checkRayCollision){
		editorCollision = editedPiece.collisionCheck(editingCameraEye, mouseRayDir, lastCollision);
		checkRayCollision = false;
	}

	if (editorCollision){
		//offset the block from its source depending on which side it's being extruded coming from
		extrudedBlock = lastCollision.block;
		extrudedBlock.data[lastCollision.side >> 1] += ((lastCollision.side & 1) == 0)? -1 : 1;
	}
	else{
		//no collision, so intersect mouse ray with the grid
		float h = /*editingLayer*/ - editingCameraEye.y;
		vec3 gridRayPoint = editingCameraEye + mouseRayDir * (h / mouseRayDir.y);

		int gpx = floor(gridRayPoint.x);
		int gpz = floor(gridRayPoint.z);

		extrudedBlock.x = gpx;
		extrudedBlock.y = 0;//editingLayer;
		extrudedBlock.z = gpz;
	}

	if (mouseDown[GLUT_LEFT_BUTTON] and !lastMouseDown[GLUT_LEFT_BUTTON]){
		printf("adding block at (%d %d %d)\n", extrudedBlock.x, extrudedBlock.y, extrudedBlock.z);
		editedPiece.insert(extrudedBlock);
		editingHistory.emplace_back(false, extrudedBlock);

		if (editingHistory.size() == 1){
			undoButton->setColor(0.4f, 0.5f, 0.2f);
		}
	}

	else if (mouseDown[GLUT_RIGHT_BUTTON] and !lastMouseDown[GLUT_RIGHT_BUTTON] and editorCollision){
		printf("removing block at (%d %d %d)\n", lastCollision.block.x, lastCollision.block.y, lastCollision.block.z);
		editedPiece.remove(lastCollision.block);
		editingHistory.emplace_back(true, lastCollision.block);
	}

	int gridCenterX, gridCenterZ;
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

		glTexCoord2f((i + 0.0f) / FLOATING_GRID_EXTENT, -1.0f); glVertex3i(gx, 0 /*editingLayer*/, gz0);
		glTexCoord2f((i + 0.0f) / FLOATING_GRID_EXTENT,  1.0f); glVertex3i(gx, 0 /*editingLayer*/, gz1);
	}

	for (int i = -FLOATING_GRID_EXTENT; i < FLOATING_GRID_EXTENT; i++){
		int gx0 = gridCenterX - FLOATING_GRID_EXTENT;
		int gx1 = gridCenterX + FLOATING_GRID_EXTENT;
		int gz = gridCenterZ + i;

		glTexCoord2f(-1.0f, (i + 0.0f) / FLOATING_GRID_EXTENT); glVertex3i(gx0, 0 /*editingLayer*/, gz);
		glTexCoord2f( 1.0f, (i + 0.0f) / FLOATING_GRID_EXTENT); glVertex3i(gx1, 0 /*editingLayer*/, gz);
	}

	glEnd();

	glDisable(GL_TEXTURE_2D);


	//draw current piece, pushing the solid planes it a little back so that the lines will show, even at acute angles
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);

	glBegin(GL_QUADS);
	for (auto pair : editedPiece.blocks){
		const vec3i& b = pair.first;
		if (b == lastCollision.block and editorCollision) glColor4f(0.5f, 0.9f, 0.5f, 1.0f);
		else glColor4f(0.5f, 0.5f, 0.9f, 1.0f);
		drawCube(b.x, b.y, b.z);
	}
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);

	glLineWidth(3.0f);
	glBegin(GL_LINES);
	for (auto pair : editedPiece.blocks){
		const vec3i& b = pair.first;
		if (b == lastCollision.block and editorCollision) glColor4f(0.7f, 0.9f, 0.7f, 1.0f);
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
}

void App::editingOverlay(){
	const int labelBufLength = 256;
	char labelBuf[labelBufLength];
	int metricW, metricH;

	glColor4f(1.0f, 1.0f, 0.5f, 1.0f);

	//eye position
	sprintf(labelBuf, "eye at (%.3f, %.3f, %.3f)", editingCameraEye.x, editingCameraEye.y, editingCameraEye.z);
	TextRender::render(labelBuf, 10, height - 80);

	//mouse ray dir position
	sprintf(labelBuf, "mouse dir (%.3f, %.3f, %.3f)", mouseRayDir.x, mouseRayDir.y, mouseRayDir.z);
	TextRender::render(labelBuf, 10, height - 60);

	//camera rotation
	vec3f turnDiveDist = editingCameraControl->getTurnDiveDist();
	sprintf(labelBuf, "turn %.2f, dive %.2f", turnDiveDist.x, turnDiveDist.y);
	TextRender::render(labelBuf, 10, height - 40);
}

