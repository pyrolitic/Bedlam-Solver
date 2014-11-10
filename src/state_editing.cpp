#include <GL/glew.h>
#include <GL/freeglut.h>

#include "app.h"
#include "ui/linear_container.h"
#include "maths/mat3.h"
#include "maths/mat4.h"

EditingScreen::EditingScreen(){
	//meshes
	blocksVAO = new VertexArrayObject<blockVert>();
	singleBlockVAO = new VertexArrayObject<blockVert>();
	gridVAO = new VertexArrayObject<blockVert>();

	blockVert* v = new blockVert[36];
	memcpy(v, defaultBlock, 36 * sizeof(blockVert));
	for (int i = 0; i < 36; i++){
		const uint8_t c[] = {0xB2, 0x66, 0x66, 0x7F};
		memcpy(v[i].col, c, 4);
	}
	singleBlockVAO->assign(36, v);
	delete [] v;

	v = new blockVert[(FLOATING_GRID_EXTENT + 1) * 2 * 4];
	memset(v, 0, (FLOATING_GRID_EXTENT + 1) * 2 * 4 * sizeof(blockVert));
	for (int i = 0; i < (FLOATING_GRID_EXTENT + 1) * 2; i++){
		v[i * 4 + 0].position.set(i - FLOATING_GRID_EXTENT, 0.0f, -FLOATING_GRID_EXTENT);
		v[i * 4 + 1].position.set(i - FLOATING_GRID_EXTENT, 0.0f, FLOATING_GRID_EXTENT + 1);
		v[i * 4 + 0].normal.set(1.0f, 1.0f, 1.0f);
		v[i * 4 + 1].normal.set(1.0f, 1.0f, 1.0f);
		v[i * 4 + 0].tex.set(0.5f + (float)(i - FLOATING_GRID_EXTENT) / (FLOATING_GRID_EXTENT * 2), 0.0f);
		v[i * 4 + 1].tex.set(0.5f + (float)(i - FLOATING_GRID_EXTENT) / (FLOATING_GRID_EXTENT * 2), 1.0f);
		*((uint32_t*) v[i * 4 + 0].col) = 0xFFFFFFFF; //endianess doesn't matter
		*((uint32_t*) v[i * 4 + 1].col) = 0xFFFFFFFF;

		v[i * 4 + 2].position.set(-FLOATING_GRID_EXTENT,    0.0f, i - FLOATING_GRID_EXTENT);
		v[i * 4 + 3].position.set(FLOATING_GRID_EXTENT + 1, 0.0f, i - FLOATING_GRID_EXTENT);
		v[i * 4 + 2].normal.set(1.0f, 1.0f, 1.0f);
		v[i * 4 + 3].normal.set(1.0f, 1.0f, 1.0f);
		v[i * 4 + 2].tex.set(0.0f, 0.5f + (float)(i - FLOATING_GRID_EXTENT) / (FLOATING_GRID_EXTENT * 2));
		v[i * 4 + 3].tex.set(1.0f, 0.5f + (float)(i - FLOATING_GRID_EXTENT) / (FLOATING_GRID_EXTENT * 2));
		*((uint32_t*) v[i * 4 + 2].col) = 0xFFFFFFFF;
		*((uint32_t*) v[i * 4 + 3].col) = 0xFFFFFFFF;
	}
	gridVAO->assign((FLOATING_GRID_EXTENT + 1) * 2 * 4, v);
	delete [] v;


	//UI
	cameraControl = new CameraControl();
	rootUI = (UIElem*) cameraControl;
	cameraControl->bindMouseMove(this, &EditingScreen::mouseMoveCallback);
	cameraControl->bindMouseDrag(this, &EditingScreen::mouseDragCallback);
	cameraControl->bindMouseWheel(this, &EditingScreen::mouseWheelCallback);
	cameraControl->bindMouseDown(this, &EditingScreen::mouseDownCallback);

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
	buttonFrame->setPosition(ivec2(20, 20));

	LinearContainer* layout = new LinearContainer(LINEAR_CONTAINER_HORIZONTAL);
	layout->addChild(discardButton);
	layout->addChild(saveButton);
	layout->addChild(undoButton);

	buttonFrame->addChild(layout);
	rootUI->addChild(buttonFrame);

	//debug
	LinearContainer* debugList = new LinearContainer(LINEAR_CONTAINER_VERTICAL);
	debugList->setFlag(UI_STICK_BOTTOM_LEFT);
	rootUI->addChild(debugList);

	eyeLabel = new Label();
	mouseRayLabel = new Label();
	collisionLabel = new Label();
	newBlockLabel = new Label();
	cameraAngleLabel = new Label();

	debugList->addChild(eyeLabel);
	debugList->addChild(mouseRayLabel);
	debugList->addChild(collisionLabel);
	debugList->addChild(newBlockLabel);
	debugList->addChild(cameraAngleLabel);

	//state
	parentRef = nullptr;
}

EditingScreen::~EditingScreen(){
	delete blurredBlob;

	delete blocksVAO;
	delete singleBlockVAO;
	delete gridVAO;
}

void EditingScreen::transitionWithPiece(listPieceEntry* ref, bool pieceIsNew){
	parentRef = ref;
	tempPiece = *(ref->itemIt); //copy
	newPiece = pieceIsNew;

	state = STATE_EDIT_PIECE;
	cleanInput();
	rebuildBlockVAO();
}


void EditingScreen::discardButtonCallback(UIElem* context, ivec2 at, int button){
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

void EditingScreen::saveButtonCallback(UIElem* context, ivec2 at, int button){
	if (button == GLUT_LEFT_BUTTON){
		printf("clicked on the save button\n");
		//copy the changes and change state
		tempPiece.cancelOffset();
		*(parentRef->itemIt) = tempPiece; //copy back
		history.clear();

		PieceListScreen::getInstance().transition();
	}
}

void EditingScreen::undoButtonCallback(UIElem* context, ivec2 at, int button){
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

			rebuildBlockVAO();
			history.pop_back();

			if (history.empty()){
				undoButton->setColor(UNDO_BUTTON_INACTIVE_COLOR);
			}
		}
	}
}

void EditingScreen::mouseDownCallback(UIElem* context, ivec2 at, int button){
	if (!tooFar){
		if (button == GLUT_LEFT_BUTTON){
			printf("adding block at (%d %d %d)\n", extrudedBlock.x, extrudedBlock.y, extrudedBlock.z);
			tempPiece.insert(extrudedBlock);
			history.emplace_back(false, extrudedBlock);
			rebuildBlockVAO();

			undoButton->setColor(UNDO_BUTTON_ACTIVE_COLOR);
			checkCollision();
		}

		else if (collision and (button == GLUT_RIGHT_BUTTON)){
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
	ivec3 oldBlock = lastCollision.block;
	int oldSide = lastCollision.side;
	bool oldCollision = collision;

	collision = tempPiece.collisionCheck(cameraEye, mouseRayDir, lastCollision);
	tooFar = false; //limit additions to EDITING_MAX_DIST_BLOCK

	if (collision){
		//offset the block from its source depending on which side it's being extruded coming from
		extrudedBlock = lastCollision.block;
		extrudedBlock.data[lastCollision.side >> 1] += ((lastCollision.side & 1) == 0)? -1 : 1;

		if (!oldCollision or oldBlock != lastCollision.block or oldSide != lastCollision.side){
			rebuildBlockVAO(); //move green tile
		}
	}
	else{
		//no collision, so intersect mouseInfo.mouse ray with the grid
		float h = /*editingLayer*/ - cameraEye.y;
		vec3 gridRayPoint = cameraEye + mouseRayDir * (h / mouseRayDir.y);
		lastCollision.point = gridRayPoint; //TODO: a little confusing, but it's for the grid offset

		int gpx = floor(gridRayPoint.x);
		int gpz = floor(gridRayPoint.z);

		extrudedBlock.x = gpx;
		extrudedBlock.y = 0;//editingLayer;
		extrudedBlock.z = gpz;

		if (oldCollision){
			rebuildBlockVAO(); //remove green tile
		}
	}

	if (!(extrudedBlock.abs() < ivec3(EDITING_MAX_DIST_BLOCK))){
		tooFar = true;
	}
}

void EditingScreen::mouseMoveCallback(UIElem* context, ivec2 to){
	updateViewMatrix();
	updateMouseRay(to);
}

void EditingScreen::mouseDragCallback(UIElem* context, ivec2 from, ivec2 to, int button){
	updateViewMatrix();
	updateMouseRay(mouseInfo.mouse);
}

void EditingScreen::mouseWheelCallback(UIElem* context, ivec2 at, int delta){
	updateViewMatrix();
	updateMouseRay(mouseInfo.mouse);
}

void EditingScreen::updateViewMatrix(){
	cameraEye = cameraControl->getEyePosition();
	view = mat4::viewLookAt(cameraEye, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
}

void EditingScreen::updateMouseRay(ivec2 to){
	//check for mouse ray collision against the blocks
	bool success;
	mat4 invPVM = dmat4(projection * view).inverse(success);

	vec4 farHomogenous = invPVM * vec4((2.0f * to.x) / windowResolution.x - 1.0f,
	                         (2.0f * (windowResolution.y - to.y)) / windowResolution.y - 1.0f,
	                         (2.0f) - 1.0f,
	                          1.0f);
	vec3 far = farHomogenous.xyz / farHomogenous.w;

	vec4 nearHomogenous = invPVM * vec4((2.0f * to.x) / windowResolution.x - 1.0f,
	                          (2.0f * (windowResolution.y - to.y)) / windowResolution.y - 1.0f,
	                          (-2.0f) - 1.0f,
	                           1.0f);
	vec3 near = nearHomogenous.xyz / nearHomogenous.w;

	mouseRayDir = (far - near).normal();

	checkCollision();
}

void EditingScreen::rebuildBlockVAO(){
	int count = tempPiece.mass();
	blockVert* buf = new blockVert[count * 6 * 6]; //for every of the 6 faces there must be two triangles
	blockVert* v = buf;

	const uint8_t defaultColor[] = {0x7F, 0x7F, 0xE5, 0xFF};
	const uint8_t collidedColor[] = {0x7F, 0xE5, 0x7F, 0xFF};

	for (auto& block : tempPiece.getBlocks()){
		const ivec3& pos = block.first;
		memcpy(v, defaultBlock, 36 * sizeof(blockVert));

		for (int i = 0; i < 6 * 6; i++){
			v[i].position += pos;
			memcpy(v[i].col, defaultColor, 4);
		}

		if (pos == lastCollision.block and collision){
			int collidedFaceId = lastCollision.side * 6;
			for (int i = 0; i < 6; i++){
				memcpy(v[collidedFaceId + i].col, collidedColor, 4);
			}
		}

		v += 6 * 6;
	}

	blocksVAO->assign(count * 6 * 6, buf);
	delete [] buf;
}

void EditingScreen::update(){
	//set up perspective projection
	projection = mat4::perspectiveProjection(80.0f, (float)windowResolution.x / (float)windowResolution.y, 1e-2f, 1e4f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LESS); //normal
	glActiveTexture(GL_TEXTURE0);

	blockShader->use();
	glUniform3f(blockShader->getUniformLocation("directionalLightDirection"), 0.7f, 0.7f, 0.7f);
	glUniform1i(blockShader->getUniformLocation("texture"), 0);

	//draw current piece, pushing the solid planes back a little so that the lines will show, even at acute angles
	mat4 model = mat4::identity();
	mat4 mvp = projection * view * model;
	mat3 texMatrix = mat3::identity();

	bool inverseSuccess;
	mat3 norMatrix = mat3::fromMat4(model).inverse(inverseSuccess).transpose();

	if (!inverseSuccess){
		fprintf(stderr, "piece editor: no well defined normal matrix for the blocks\n");
	}

	glUniformMatrix4fv(blockShader->getUniformLocation("modelViewProjectionMatrix"), 1, GL_FALSE, (const GLfloat*)mvp.data);
	glUniformMatrix4fv(blockShader->getUniformLocation("modelMatrix"), 1, GL_FALSE, (const GLfloat*)model.data);
	glUniformMatrix3fv(blockShader->getUniformLocation("textureMatrix"), 1, GL_FALSE, (const GLfloat*)texMatrix.data);
	glUniformMatrix3fv(blockShader->getUniformLocation("normalMatrix"), 1, GL_FALSE, (const GLfloat*)norMatrix.data);

	vec3 pointLight(0.0f);
	if (!tooFar){
		pointLight = lastCollision.point;
	}
	glUniform3f(blockShader->getUniformLocation("pointLightPosition"), pointLight.x, pointLight.y, pointLight.z);

	blocksVAO->bind();
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(1.0f, 1.0f);

	//draw solid cubes
	glDrawArrays(GL_TRIANGLES, 0, blocksVAO->getVerticesAssigned());

	//draw lines
	//glDisable(GL_POLYGON_OFFSET_FILL);
	//glDrawArrays(GL_LINES, 0, blocksVAO->getVerticesAssigned());

	if (!tooFar){
		//also draw the extruded block
		model = mat4::translation(extrudedBlock);
		mvp = projection * view * model;
		texMatrix = mat3::identity();
		norMatrix = mat3::fromMat4(model).inverse(inverseSuccess).transpose();

		if (!inverseSuccess){
			fprintf(stderr, "piece editor: no well defined normal matrix for the extruded block\n");
		}

		glUniformMatrix4fv(blockShader->getUniformLocation("modelViewProjectionMatrix"), 1, GL_FALSE, (const GLfloat*)mvp.data);
		glUniformMatrix4fv(blockShader->getUniformLocation("modelMatrix"), 1, GL_FALSE, (const GLfloat*)model.data);
		glUniformMatrix3fv(blockShader->getUniformLocation("textureMatrix"), 1, GL_FALSE, (const GLfloat*)texMatrix.data);
		glUniformMatrix3fv(blockShader->getUniformLocation("normalMatrix"), 1, GL_FALSE, (const GLfloat*)norMatrix.data);

		singleBlockVAO->bind();
		glEnable(GL_BLEND);
		glDrawArrays(GL_TRIANGLES, 0, singleBlockVAO->getVerticesAssigned());
	}


	glEnable(GL_BLEND);
	lineShader->use();
	glUniform1i(lineShader->getUniformLocation("texture"), 0);

	int gridCenterX, gridCenterZ;
	gridCenterX = extrudedBlock.x;
	gridCenterZ = extrudedBlock.z;

	if (!collision and !tooFar){
		//draw the grid
		blurredBlob->bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		model = mat4::translation(vec3(gridCenterX, 0.0f, gridCenterZ));
		mvp = projection * view * model;

		vec2 modOffset = vec2(fmod(lastCollision.point.x, 1.0f), fmod(lastCollision.point.z, 1.0f));
		if (modOffset.x < 0.0f) modOffset.x += 1.0f;
		if (modOffset.y < 0.0f) modOffset.y += 1.0f;
		modOffset /= (FLOATING_GRID_EXTENT + 1) * 2;

		texMatrix = mat3::translation(-modOffset);

		glUniformMatrix4fv(lineShader->getUniformLocation("modelViewProjectionMatrix"), 1, GL_FALSE, (const GLfloat*)mvp.data);
		glUniformMatrix4fv(lineShader->getUniformLocation("modelMatrix"), 1, GL_FALSE, (const GLfloat*)model.data);
		glUniformMatrix3fv(lineShader->getUniformLocation("textureMatrix"), 1, GL_FALSE, (const GLfloat*)texMatrix.data);

		gridVAO->bind();
		glLineWidth(2.0f);
		glDrawArrays(GL_LINES, 0, gridVAO->getVerticesAssigned());
	}

	//origin arrows, x-ray
	model = mat4::identity();
	mvp = projection * view * model;
	texMatrix = mat3::identity();
	glUniformMatrix4fv(lineShader->getUniformLocation("modelViewProjectionMatrix"), 1, GL_FALSE, (const GLfloat*)mvp.data);
	glUniformMatrix4fv(lineShader->getUniformLocation("modelMatrix"), 1, GL_FALSE, (const GLfloat*)model.data);
	glUniformMatrix3fv(lineShader->getUniformLocation("textureMatrix"), 1, GL_FALSE, (const GLfloat*)texMatrix.data);

	glDisable(GL_DEPTH_TEST);
	glLineWidth(4.0f);

	axesVAO->bind();
	blurredBlob->bind();
	glDrawArrays(GL_LINES, 0, 6);


	//debug info
	const int labelBufLength = 256;
	char labelBuf[labelBufLength];

	//eye position
	snprintf(labelBuf, labelBufLength, "eye at (%.3f, %.3f, %.3f)", cameraEye.x, cameraEye.y, cameraEye.z);
	eyeLabel->setText(labelBuf);

	//mouse ray dir position
	snprintf(labelBuf, labelBufLength, "mouse dir (%.3f, %.3f, %.3f)", mouseRayDir.x, mouseRayDir.y, mouseRayDir.z);
	mouseRayLabel->setText(labelBuf);

	//mouse ray collision with grid or material
	if (!tooFar){
		snprintf(labelBuf, labelBufLength, "collision at (%d, %d, %d), (%.3f, %.3f, %.3f)",
			lastCollision.block.x, lastCollision.block.y, lastCollision.block.z,
			lastCollision.point.x, lastCollision.point.y, lastCollision.point.z);
	}
	else{
		snprintf(labelBuf, labelBufLength, "no collision in reasonable range");
	}
	collisionLabel->setText(labelBuf);

	//extruded block
	if (!tooFar){
		snprintf(labelBuf, labelBufLength, "collision with %s, leading to new block at (%d, %d, %d)",
			collision? "block" : "floor",
			extrudedBlock.x, extrudedBlock.y, extrudedBlock.z);
	}
	else{
		snprintf(labelBuf, labelBufLength, "");
	}
	newBlockLabel->setText(labelBuf);

	//camera rotation
	vec3 turnDiveDist = cameraControl->getTurnDiveDist();
	snprintf(labelBuf, labelBufLength, "turn %.2f, dive %.2f", turnDiveDist.x, turnDiveDist.y);
	cameraAngleLabel->setText(labelBuf);
}
