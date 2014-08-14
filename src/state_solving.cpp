#include <GL/glew.h>
#include <GL/freeglut.h>

#include "app.h"

listSolutionEntry::listSolutionEntry(Solver::solutionPiece* block) : Frame(){
	raw = block;
	
	Button* showButton = new Button("Show");
	showButton->bindMouseUp(this, &listSolutionEntry::showButtonCallback);
	addChild(showButton);

	Button* removeButton = new Button("Remove");
	removeButton->bindMouseUp(this, &listSolutionEntry::removeButtonCallback);
	addChild(removeButton);

	setColor(SHOW_BUTTON_NEW);
}

listSolutionEntry::~listSolutionEntry(){
	free(raw);
}

void listSolutionEntry::showButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		//transition to show solution

		setColor(SHOW_BUTTON_OLD);
	}
}

void listSolutionEntry::removeButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		listSolutionEntry* parent = dynamic_cast<listSolutionEntry*>(getParent());
		assert(parent);
		delete parent;
	}
}

SolvingScreen::SolvingScreen(){
	//UI
	solvingSolutionList = new ScrollingFrame(500);
	solvingSolutionList->setPosition(vec2i(10, 10));

	stopSolverButton = new Button((char*)"stop solver");
	stopSolverButton->bindMouseUp(this, &SolvingScreen::stopSolverButtonCallback);
	stopSolverButton->setPosition(vec2i(LIST_PIECE_FRAME_WIDTH + 20, 20));

	rootUI = new UIElem();
	rootUI->addChild(solvingSolutionList);
	rootUI->addChild(stopSolverButton);
}

SolvingScreen::~SolvingScreen(){
	
}

void SolvingScreen::transition(){
	state = STATE_SOLVE;
}

void SolvingScreen::stopSolverButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		if (solver){
			solver->kill();
		}
	}
}

void SolvingScreen::update(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (solver){
		//get all solutions
		Solver::solutionPiece* sol;
		while ((sol = solver->getSolution())){
			solvingSolutionList->addChild(new listSolutionEntry(sol));
		}

		if (!solver->isWorkerAlive()){
			printf("solver worker thread died\n");
			delete solver;
			solver = nullptr;
		}
	}
}
