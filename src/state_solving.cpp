#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "app.h"

App::listSolution::listSolution(Solver::solutionPiece* block) : Frame(vec2i(0, 0), vec2i(300, 70)){
	raw = block;
	addChild(new Button(vec2i(0, 0), "Show", &App::listSolutionShowButtonCallback));
	addChild(new Button(vec2i(150, 0), "Remove", &App::listSolutionRemoveButtonCallback));
}

App::listSolution::~listSolution(){
	free(raw);
}

void App::listSolution::draw(){
	if (seen){
		setColor(0.4f, 0.8f, 0.4f);
	}
	Frame::draw();
}

void App::listSolutionShowButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		//transition to show solution
	}
}

void App::listSolutionRemoveButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		listSolution* parent = dynamic_cast<listSolution*>(context->getParent());
		assert(parent);

		elemHover = parent->getParent(); //dangling otherwise
		UIElem::focus = elemHover; //likewise
		delete parent;
	}
}

void App::stopSolverButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		solver->kill();
	}
}

void App::solvingInit(){
	solvingSolutionList = new ScrollingFrame(vec2i(10, 10), 500);
	stopSolverButton = new Button(vec2i(LIST_PIECE_FRAME_WIDTH + 20, 20), (char*)"stop solver", &App::stopSolverButtonCallback);
	stateElems[STATE_SOLVE]->addChild(solvingSolutionList);
	stateElems[STATE_SOLVE]->addChild(stopSolverButton);
}

void App::solvingUpdate(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (solver){
		//get all solutions
		Solver::solutionPiece* sol;
		while ((sol = solver->getSolution())){
			solvingSolutionList->addChild(new listSolution(sol));
		}

		if (solver->isDead()){
			printf("solver worker thread died\n");
			delete solver;
			solver = nullptr;
		}
	}
}

void App::solvingOverlay(){

}
