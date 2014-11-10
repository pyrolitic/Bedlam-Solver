#include <algorithm>
#include <numeric>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "app.h"
#include "maths/mat3.h"
#include "maths/mat4.h"

SolutionScreen::SolutionScreen(){
	source = nullptr;
	inReps = nullptr;
	instances = 0;
	currentInstance = 0;

	pReps = nullptr;
	blocksVAO = new VertexArrayObject<blockVert>();

	//UI
	nextPieceButton = new Button("Next Piece");
	resetButton = new Button("Reset");
	backButton = new Button("Back");

	LinearContainer* buttonRow = new LinearContainer(LINEAR_CONTAINER_HORIZONTAL);
	buttonRow->addChild(backButton);
	buttonRow->addChild(resetButton);
	buttonRow->addChild(nextPieceButton);

	rootUI = new UIElem();
	rootUI->addChild(buttonRow);

	//debug
	LinearContainer* debugList = new LinearContainer(LINEAR_CONTAINER_VERTICAL);
	debugList->setFlag(UI_STICK_BOTTOM_RIGHT);
	rootUI->addChild(debugList);

	eyeLabel = new Label();
	targetLabel = new Label();
	animTimeLabel = new Label();
	animStateLabel = new Label();

	debugList->addChild(eyeLabel);
	debugList->addChild(targetLabel);
	debugList->addChild(animTimeLabel);
	debugList->addChild(animStateLabel);
}

SolutionScreen::~SolutionScreen(){
	delete pReps;
	delete inReps;
	delete blocksVAO;
}

void SolutionScreen::piecesChanged(){
	if (pReps){
		free(pReps);
		pReps = nullptr;
	}
}

void SolutionScreen::transitionWithSolution(Solver::solutionPiece* metadata){
	state = STATE_SHOW_RESULT;
	cleanInput();

	source = metadata;

	slideCameraDistance = std::max(std::max(worldSize.x, worldSize.y), worldSize.z) * 3.5f;
	pieceSpawnPosition = worldSize / 2.0f + vec3(0.0f, worldSize.y * 2, 0.0f);


	//these don't change when looking at multiple solutions of the same piece set
	if (!pReps){
		//generate a pieceRepr for every piece
		pReps = (pieceRepr*) malloc(pieces.size() * sizeof(pieceRepr));
		int massSum = 0;
		for (const auto& piece : pieces){
			massSum += piece.mass();
		}

		//for every cube, for every of the 6 faces, there must be two triangles
		blockVert* buf = new blockVert[massSum * 36]; 
		const uint8_t defaultColor[] = {0x7F, 0x7F, 0xE5, 0xFF};

		int vi = 0;
		int pieceId = 0;
		for (const auto& piece : pieces){
			int startVi = vi;
			for (const auto& block : piece.getBlocks()){
				const ivec3& pos = block.first;
				memcpy(&buf[vi], defaultBlock, 36 * sizeof(blockVert));

				for (int i = 0; i < 36; i++){
					buf[vi].position += pos;
					memcpy(buf[vi].col, defaultColor, 4);
					vi++;
				}
			}

			new (&pReps[pieceId]) pieceRepr(startVi, vi - startVi, piece.getSize());
			pieceId++;
		}

		assert(vi == massSum * 36);
		blocksVAO->assign(massSum * 36, buf);

		delete [] buf;
		buf = nullptr;
	}

	instances = std::accumulate(piecesCopies.begin(), piecesCopies.end(), 0);

	//sort the instances by pieceId, because there's no (good) way to access a linked list by index
	std::sort(source, source + instances, [](const Solver::solutionPiece& a, const Solver::solutionPiece& b) -> bool {
		return a.pieceId < b.pieceId;
	});

	//generate an instanceRepr for every instance
	inReps = (instanceRepr*) malloc(instances * sizeof(instanceRepr));
	int pieceId = 0;
	auto pieceIt = pieces.begin();
	for (int instId = 0; instId < instances; instId++){
		if (source[instId].pieceId > pieceId){
			pieceId++;
			pieceIt++;
		}
		assert(source[instId].pieceId == pieceId);

		int orientationId = source[instId].orientationId;
		const ivec3& pos = source[instId].position;

		new (&inReps[instId]) instanceRepr(*pieceIt, orientationId);
	}

	//prepare the first piece for animation
	currentInstance = -1;
	animationState = STATE_SHOW_RESULT_ANIMATION_SLIDE;

	animEnd.cameraPosition = vec3(0.0f, 0.0f, slideCameraDistance);
	animEnd.cameraTarget = worldSize / 2.0f;
	animEnd.time = glutGet(GLUT_ELAPSED_TIME) - 1; //make it transition directly

	animation = true;
}

//probably should move this to a file in src/maths/
static float smoothstep(float x){
	return x * x * (3 - 2 * x); //3x^2 - 2x^3
}

void SolutionScreen::update(){
	int now = glutGet(GLUT_ELAPSED_TIME);

	if (animation){
		if (now >= animEnd.time){
			//transition to next state
			if (animationState == STATE_SHOW_RESULT_ANIMATION_WARP and source[currentInstance].orientationId == 0){
				//skip rotation, since it would do nothing
				animationState = STATE_SHOW_RESULT_ANIMATION_SLIDE;
			}
			else{
				animationState = (animationState + 1) % STATE_SHOW_RESULT_ANIMATION_STATES;
			}
			printf("transitioning to animation state %d\n", animationState);

			switch(animationState){
			case STATE_SHOW_RESULT_ANIMATION_WARP:
				if (currentInstance < instances - 1){
					currentInstance++;
					inReps[currentInstance].draw = true;
					printf("transitioning to next piece instance, #%d\n", currentInstance);

					currentInstanceTranslation[0] = pReps[source[currentInstance].pieceId].originalSize / -2.0f;
					currentInstanceTranslation[1] = inReps[currentInstance].material.getSize() / 2.0f;

					//focus the camera on the new piece
					animStart = animEnd; //copy
					animStart.pieceRotation = animEnd.pieceRotation = quat();
					animStart.piecePosition = animEnd.piecePosition = pieceSpawnPosition;

					vec3 pSize = pReps[source[currentInstance].pieceId].originalSize;
					animEnd.cameraTarget = animEnd.piecePosition + pSize * 0.5f;
					animEnd.cameraPosition = animEnd.cameraTarget + vec3(1.0f) * pSize.length();
					animEnd.time = animStart.time + 5000; //5 seconds
				}

				else{
					printf("finished animating solution\n");
					animation = false;
				}
				break;

			case STATE_SHOW_RESULT_ANIMATION_PIECE_ROTATE:
				//keep camera and focus in the same place
				//rotate piece in place

				animStart = animEnd; //copy

				animEnd.pieceRotation = inReps[currentInstance].material.getRotationFromOriginal();
				animEnd.time = animStart.time + 5000; //5 seconds
				break;

			case STATE_SHOW_RESULT_ANIMATION_SLIDE:
				//bring the piece to its final resting place

				animStart = animEnd; //copy

				//find the best angle to view from
				vec3 viewDir = bestViewDirection(currentInstance);

				animEnd.cameraTarget = source[currentInstance].position + (pReps[source[currentInstance].pieceId].originalSize / 2.0f);
				animEnd.cameraPosition = animEnd.cameraTarget + viewDir * slideCameraDistance;
				
				animEnd.piecePosition = source[currentInstance].position;
				animEnd.pieceRotation = inReps[currentInstance].material.getRotationFromOriginal();
				animEnd.time = animStart.time + 5000; //5 seconds

				//and cement the piece
				for (const auto& block : inReps[currentInstance].material.getBlocks()){
					ivec3 pos = source[currentInstance].position + block.first;
					assert(!addedMaterial.query(pos) and "overlapping blocks");
					addedMaterial.insert(pos);
				}

				break;
			}
		}

		//compute current values, based on how far along the animation cycle we're at
		assert(animEnd.time > animStart.time);

		//interpolate translation and rotation
		float animCoeff = (float)(now - animStart.time) / (animEnd.time - animStart.time);
		float along = smoothstep(animCoeff);

		cameraPosition = animStart.cameraPosition * (1.0f - along) + animEnd.cameraPosition * along;
		cameraTarget = animStart.cameraTarget * (1.0f - along) + animEnd.cameraTarget * along;

		vec3 moving = animStart.piecePosition * (1.0f - along) + animEnd.piecePosition * along;
		quat rot = quat::lerpRot(animStart.pieceRotation, animEnd.pieceRotation, along);

		inReps[currentInstance].transformation = mat4::translation(moving + currentInstanceTranslation[1]) * 
			mat4::quaternionRotation(rot) * mat4::translation(currentInstanceTranslation[0]);

		char buf[256];
		snprintf(buf, 256, "camera position: (%f %f %f)", cameraPosition.x, cameraPosition.y, cameraPosition.z);
		eyeLabel->setText(buf);

		snprintf(buf, 256, "camera target: (%f %f %f)", cameraTarget.x, cameraTarget.y, cameraTarget.z);
		targetLabel->setText(buf);

		snprintf(buf, 256, "animation progression: %f (%f)", animCoeff, along);
		animTimeLabel->setText(buf);

		const char* stateNames[] = {"new", "rotating", "sliding"};
		snprintf(buf, 256, "animation state: %s, piece #%d, instance #%d", stateNames[animationState], source[currentInstance].pieceId, currentInstance);
		animStateLabel->setText(buf);
	}

	//draw
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projection = mat4::perspectiveProjection(80.0f, (float)windowResolution.x / (float)windowResolution.y, 1e-2f, 1e4f);
	view = mat4::viewLookAt(cameraPosition, cameraTarget, vec3(0.0f, 1.0f, 0.0f));

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthFunc(GL_GREATER); //normal
	glActiveTexture(GL_TEXTURE0);

	blockShader->use();
	glUniform1i(blockShader->getUniformLocation("texture"), 0);

	vec3 pointLight(0.0f);
	glUniform3f(blockShader->getUniformLocation("pointLightPosition"), pointLight.x, pointLight.y, pointLight.z);

	vec3 dirLight = vec3(0.5f, 0.2f, 0.8f).normal();
	glUniform3f(blockShader->getUniformLocation("directionalLightDirection"), dirLight.x, dirLight.y, dirLight.z);

	//draw already placed pieces
	blocksVAO->bind();
	for (int instId = 0; instId < instances; instId++){
		if (inReps[instId].draw){
			mat4& model = inReps[instId].transformation;
			mat4 mvp = projection * view * model;
			mat3 texMatrix = mat3::identity();
			bool inverseSuccess;
			mat3 norMatrix = mat3::fromMat4(model).inverse(inverseSuccess).transpose();

			if (!inverseSuccess){
				fprintf(stderr, "show solution: piece %d, has no well defined normal matrix\n", instId);
			}

			glUniformMatrix4fv(blockShader->getUniformLocation("modelViewProjectionMatrix"), 1, GL_FALSE, (const GLfloat*)mvp.data);
			glUniformMatrix4fv(blockShader->getUniformLocation("modelMatrix"), 1, GL_FALSE, (const GLfloat*)model.data);
			glUniformMatrix3fv(blockShader->getUniformLocation("textureMatrix"), 1, GL_FALSE, (const GLfloat*)texMatrix.data);
			glUniformMatrix3fv(blockShader->getUniformLocation("normalMatrix"), 1, GL_FALSE, (const GLfloat*)norMatrix.data);

			int pieceId = source[instId].pieceId;
			glDrawArrays(GL_TRIANGLES, pReps[pieceId].firstVert, pReps[pieceId].vertCount);
		}
	}


	//bounding box, x-ray
	lineShader->use();
	//glDisable(GL_DEPTH_TEST);
	glLineWidth(4.0f);

	axesVAO->bind();
	blurredBlob->bind();

	for (int ci = 0; ci < 8; ci++){
		vec3 pos, scale;
		for (int ai = 0; ai < 3; ai++){
			int far = (ci >> ai) & 0x1;
			pos.data[ai] = far * worldSize.data[ai];
			scale.data[ai] = 1 - 2 * far;
		}

		mat4 model = mat4::translation(pos) * mat4::scale(scale);
		mat4 mvp = projection * view * model;
		mat3 texMatrix = mat3::identity();

		glUniformMatrix4fv(lineShader->getUniformLocation("modelViewProjectionMatrix"), 1, GL_FALSE, (const GLfloat*)mvp.data);
		glUniformMatrix4fv(lineShader->getUniformLocation("modelMatrix"), 1, GL_FALSE, (const GLfloat*)model.data);
		glUniformMatrix3fv(lineShader->getUniformLocation("texMatrix"), 1, GL_FALSE, (const GLfloat*)texMatrix.data);

		glDrawArrays(GL_LINES, 0, 6);
	}
}

ivec3 SolutionScreen::slideDirection(int instId, int& distance){
	const ivec3& place = source[instId].position; //distination
	const ivec3& size = addedMaterial.getSize();

	//ray-cast in every direction to the edge of the cube
	struct sideRec{
		int side; //(axis << 1) + right
		int dist; //to edge of the cube, -1 if something was hit
	} sides[6];
	
	for (int i = 0; i < 6; i++){
		sides[i].side = i;
		sides[i].dist = 0;
	}

	//go through every block in the final position
	for (const auto& block : inReps[instId].material.getBlocks()){
		ivec3 start = place + block.first; //final position of that block

		//check if there's a clear path to the edge of the cube on each axis,
		for (int axis = 0; axis < 3; axis++){
			//both directions
			for (int right = 0; right < 2; right++){
				int side = (axis << 1) + right;
				int dist = 0;

				//walk along that direction to the edge of the cube
				vec3 pos = start;
				while (pos <= size and pos > ivec3(0)){
					if (addedMaterial.query(pos)){
						//hit something
						sides[side].dist = -1;
						break;
					}

					else{
						//clear
						pos.data[axis] += -1 + 2 * right;
						dist++;
					}
				}

				if (sides[side].dist != -1){
					sides[side].dist = std::max(sides[side].dist, dist);
				}
			}
		}
	}

	//sort by distance, ascending order
	std::sort(sides, sides + 6, [](const sideRec& a, const sideRec& b) -> bool{
		return a.dist < b.dist;
	});

	ivec3 ret(0, 0, 0);

	int maskedDirs = 0;
	for (;sides[maskedDirs].dist == -1 and maskedDirs < 6; maskedDirs++);

	if (maskedDirs == 6){
		printf("show solution: no slide direction for piece #%d\n", instId);
		distance = 0;
		ret = ivec3(1, 0, 0);
	}
	else {
		//randomly pick one of the non-masked dirs
		int one = maskedDirs;
		if (maskedDirs < 5) one += rand() % (6 - maskedDirs);
		assert(one < 6);

		ret.data[sides[one].side >> 1] += -1 + (sides[one].side & 1) * 2;
		distance = sides[one].dist;
	}

	return ret;
}

vec3 SolutionScreen::bestViewDirection(int instId){
	ivec3 place = source[instId].position;

	//Try to find the best viewing angle:
	//Every block is checked for occlusion from view (by the already placed blocks),
	//from every cardinal direction, and two vertical inclination angles, 30 and 60 degrees.
	//The angles that do not cause occludion are summed up and normalized, 
	//resulting in what is hopefully a good view.

	vec3 quadrantSum[4];
	int quadrantHits[4];
	for (int i = 0; i < 4; i++){
		quadrantSum[i] = vec3(0.0f);
		quadrantHits[i] = 0;
	}

	const float inclinations[2] = {30.0f / 180.0f * M_PI, 60.0f / 180.0f * M_PI};
	Piece::collisionResult hit;

	for (const auto& block : inReps[instId].material.getBlocks()){
		vec3 pos = vec3(place + block.first) + vec3(0.5f);

		for (int ii = 0; ii < 2; ii++){
			for (int ri = 0; ri < 4; ri++){
				vec3 dir = mat4::flightRotation((float)ri / 2.0f * M_PI, inclinations[ii], 0.0f).multVec3(vec3(1.0f, 0.0f, 0.0f));
				if (addedMaterial.collisionCheck(pos, dir, hit)){
					quadrantHits[ri]++;
				}
				/*if (addedMaterial.collisionCheck(pos, dir, hit)){
					dirOccluded[ri] |= 1 << hit.side;
				}*/
				else{
					quadrantSum[ri] += dir;
				}
			}
		}
	}

	/*const char* axes[] = {"x", "y", "z"};
	for (int ii = 0; ii < 2; ii++){
		printf("inclination %s\n", (ii == 0)? "30" : "60");
		for (int a = 0; a < 3; a++){
			printf("%s:%d ", axes[a], (dirOccluded[ii] >> (a * 2)) & 0x2);
		}
		printf("\n");
	}*/

	int bestQ = 0;
	int fewest = quadrantHits[0];
	for (int qi = 1; qi < 4; qi++){
		if (quadrantHits[qi] < fewest){
			fewest = quadrantHits[qi];
			bestQ = qi;
		}
	}

	vec3 dir = quadrantSum[bestQ].normal();
	printf("view dir: (%.2f %.2f %.2f)\n", dir.x, dir.y, dir.z);
	return dir;
}
