#include <numeric>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "app.h"

listSolutionEntry::listSolutionEntry(Solver::solutionPiece* data, int elapsedMilli) 
		: Frame(){
	metadata = data;

	LinearContainer* rows = new LinearContainer(LINEAR_CONTAINER_VERTICAL);
	LinearContainer* buttonRow = new LinearContainer(LINEAR_CONTAINER_HORIZONTAL);

	int seconds = elapsedMilli / 1000;
	char buf[64];

	if (seconds == 0){
		snprintf(buf, 64, "Took %d ms to find", elapsedMilli);
	}
	else{
		snprintf(buf, 64, "Took %d s to find", seconds);
	}

	Label* timestamp = new Label(buf);
	rows->addChild(timestamp);
	rows->addChild(buttonRow);

	Button* showButton = new Button("Show");
	showButton->bindMouseUp(this, &listSolutionEntry::showButtonCallback);
	buttonRow->addChild(showButton);

	Button* removeButton = new Button("Remove");
	removeButton->bindMouseUp(this, &listSolutionEntry::removeButtonCallback);
	buttonRow->addChild(removeButton);


	addChild(rows);
	setColor(LIST_SOLUTION_ENTRY_NEW_BG);
}

listSolutionEntry::~listSolutionEntry(){
	free(metadata);
}

void listSolutionEntry::showButtonCallback(UIElem* context, ivec2 at, int button){
	if (button == GLUT_LEFT_BUTTON){
		setColor(LIST_SOLUTION_ENTRY_SEEN_BG);

		SolutionScreen::getInstance().transitionWithSolution(metadata);
	}
}

void listSolutionEntry::removeButtonCallback(UIElem* context, ivec2 at, int button){
	if (button == GLUT_LEFT_BUTTON){
		delete this;
	}
}

SolvingScreen::SolvingScreen(){
	lastSolutionFoundAt = 0;

	//UI
	solvingSolutionList = new ScrollingFrame(500);
	solvingSolutionList->setFlag(UI_STICK_TOP_LEFT);

	solutionsIndicator = new Button((char*)"Solutions coming");
	solutionsIndicator->setColor(ADD_NEW_BUTTON_COLOR);
	solutionsIndicator->setSize(ivec2(LIST_PIECE_FRAME_WIDTH, solutionsIndicator->getSize().y));

	stopSolverButton = new Button((char*)"Stop solver");
	stopSolverButton->setColor(DELETE_BUTTON_COLOR);
	stopSolverButton->bindMouseUp(this, &SolvingScreen::stopSolverButtonCallback);
	stopSolverButton->setFlag(UI_AUTO_RESIZE);

	backButton = new Button((char*)"Back");
	backButton->setColor(SOLVE_BUTTON_GOOD_COLOR);
	backButton->bindMouseUp(this, &SolvingScreen::backButtonCallback);

	LinearContainer* buttonCol = new LinearContainer(LINEAR_CONTAINER_VERTICAL);
	buttonCol->setFlag(UI_STICK_BOTTOM_LEFT);
	buttonCol->addChild(stopSolverButton);
	buttonCol->addChild(backButton);

	rootUI = new UIElem();
	rootUI->addChild(solvingSolutionList);
	rootUI->addChild(buttonCol);
}

SolvingScreen::~SolvingScreen(){
	
}

void SolvingScreen::transition(){
	state = STATE_SOLVE;
	cleanInput();

	if (solver){
		printf("fixme: stalling until old solver dies\n");
		while(solver->isWorkerAlive()){
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		delete solver;
	}

	solver = new Solver(worldSize, pieces, piecesCopies);
	lastSolutionFoundAt = glutGet(GLUT_ELAPSED_TIME);
	solvingSolutionList->addChild(solutionsIndicator);
}

void SolvingScreen::backButtonCallback(UIElem* context, ivec2 at, int button){
	if (solvingSolutionList->hasChild(solutionsIndicator)){
		solvingSolutionList->removeChild(solutionsIndicator);
	}

	//remove all solutions
	for (auto it = solvingSolutionList->getChildren().begin(); it != solvingSolutionList->getChildren().end();){
		delete *it++;
	}

	if (solver){
		//get the solver to shut down until we get back to this screen
		solver->kill();
	}

	PieceListScreen::getInstance().transition();
}

void SolvingScreen::stopSolverButtonCallback(UIElem* context, ivec2 at, int button){
	if (button == GLUT_LEFT_BUTTON){
		if (solver){
			solver->kill();
			stopSolverButton->setText("Solver stopping..");
		}
	}
}

void SolvingScreen::update(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (solver){
		//get all solutions
		Solver::solutionPiece* sol;
		while ((sol = solver->getSolution())){
			printf("got solution:\n");
			for (int i = 0; i < std::accumulate(piecesCopies.begin(), piecesCopies.end(), 0); i++){
				const ivec3& pos = sol[i].position;
				printf("piece #%d, copy #%d, orient: #%d, pos: (%d, %d, %d)\n", sol[i].pieceId, sol[i].copyId, sol[i].orientationId, pos.x, pos.y, pos.z);
			}

			int now = glutGet(GLUT_ELAPSED_TIME);
			int elapsedMilli = now - lastSolutionFoundAt;
			solvingSolutionList->addChildBefore(new listSolutionEntry(sol, elapsedMilli), solutionsIndicator);
			lastSolutionFoundAt = now;
		}

		if (!solver->isWorkerAlive()){
			printf("Solver worker thread died.\n");
			delete solver;
			solver = nullptr;
			stopSolverButton->setText("Solver stopped");
		}
	}
	else{
		if (solutionsIndicator){
			//no more solutions coming

			delete solutionsIndicator;
			solutionsIndicator = nullptr;
		}
	}
}
