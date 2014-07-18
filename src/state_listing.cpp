#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "app.h"

extern App* app; //in main.cpp

App::listPiece::listPiece(const std::list<Piece>::iterator& it) : Frame(vec2i(0, 0), vec2i(0, 0)), itemIt(it){
	setColor(0.7f, 0.7f, 0.7f);

	nameEntry = new TextInput(vec2i(0, 0), LIST_PIECE_FRAME_WIDTH - 40, "piece name");
	const int separation = 10;
	int secondLine = nameEntry->getSize().y + separation;

	Button* editButton = new Button(vec2i(0, secondLine), (char*)"edit", &App::listPieceEditButtonCallback);
	Button* deleteButton = new Button(vec2i(60, secondLine), (char*)"delete", &App::listPieceDeleteButtonCallback);
	copiesCounter = new Counter(vec2i(150, secondLine), 1, 99, 1, &App::counterChangeCallback);

	editButton->setColor(0.4f, 0.7f, 0.7f);
	deleteButton->setColor(0.8f, 0.4f, 0.4f);
	copiesCounter->setColor(0.4f, 0.1f, 0.6f);

	addChild(nameEntry);
	addChild(editButton);
	addChild(deleteButton);
	addChild(copiesCounter);

	setSize(vec2i(LIST_PIECE_FRAME_WIDTH, editButton->getSize().y * 2 + separation + 2 * FRAME_ROUNDED_RADIUS));
}

App::listPiece::~listPiece(){
	app->pieces.erase(itemIt);
}

void App::listPieceEditButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		listPiece* parent = dynamic_cast<listPiece*>(context->getParent());
		assert(parent);

		//transition to editor
		editedPieceParentRef = parent;
		editedPiece = *(parent->itemIt); //copy data before editing
		transition(STATE_EDIT_PIECE);
		editingNewPiece = false;
	}
}

void App::listPieceDeleteButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		listPiece* parent = dynamic_cast<listPiece*>(context->getParent());
		assert(parent);

		elemHover = parent->getParent(); //dangling otherwise
		UIElem::focus = elemHover; //likewise
		delete parent;

		pieceListCheckSolvingPossiblity();
	}
}

void App::newPieceButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		//create a new piece ui element
		pieces.emplace_front();
		listPiece* p = new listPiece(pieces.begin());

		pieceListFrame->addChildBefore(p, newPieceButton);

		//transition to editor
		editedPieceParentRef = p;
		editedPiece = *(p->itemIt); //copy data before editing
		transition(STATE_EDIT_PIECE);
		editingNewPiece = true;
	}
}

void App::solveButtonCallback(UIElem* context, vec2i at, int button){
	if (button == GLUT_LEFT_BUTTON){
		pieceListCheckSolvingPossiblity(); //might not be needed
		if (solvingPossible){
			vec3i worldSize(dimCounters[0]->getValue(), dimCounters[1]->getValue(), dimCounters[2]->getValue());
			solver = new Solver(worldSize, pieces);
			transition(STATE_SOLVE);
		}
	}
}

void App::counterChangeCallback(Counter* context, int oldValue, int newValue){
	pieceListCheckSolvingPossiblity();
}

void App::pieceListCheckSolvingPossiblity(){
	solvingPossible = false; //assume until proven untrue

	vec3i worldSize(dimCounters[0]->getValue(), dimCounters[1]->getValue(), dimCounters[2]->getValue());
	int worldSpace = worldSize.x * worldSize.y * worldSize.z;
	int material = 0;

	for (auto child : pieceListFrame->getChildren()){
		listPiece* lp;
		if ((lp = dynamic_cast<listPiece*>(child))){
			//update copies amount in pieces
			lp->itemIt->copies = lp->copiesCounter->getValue();
			material += lp->itemIt->copies * lp->itemIt->mass();
		}
	}

	char buf[256];
	snprintf(buf, 256, "%d out of %d material", material, worldSpace);
	materialStatusLabel->setText(buf);

	if (material == worldSpace){
		solvingPossible = true;
	}
}

void App::pieceListInit(){
	pieceListFrame = new ScrollingFrame(vec2i(10, 10), 500);
	newPieceButton = new Button(vec2i(0, 0), (char*)"Add new", &App::newPieceButtonCallback);
	newPieceButton->setColor(0.4f, 0.8f, 0.4f);
	newPieceButton->setSize(vec2i(LIST_PIECE_FRAME_WIDTH, newPieceButton->getSize().y));
	pieceListFrame->addChild(newPieceButton);

	solvingOptionsFrame = new Frame(pieceListFrame->getPosition() + vec2i(LIST_PIECE_FRAME_WIDTH + 50, 0), vec2i(5 * 70, newPieceButton->getSize().y + 2 * FRAME_ROUNDED_RADIUS));
	for (int i = 0; i < 3; i++){
		dimCounters[i] = new Counter(vec2i(i * 70, 0), 1, 99, 4);
		solvingOptionsFrame->addChild(dimCounters[i]);
		dimCounters[i]->setColor(0.4f, 0.1f, 0.6f);
	}
	solveButton = new Button(vec2i(3 * 70 + 30, 0), "Solve", &App::solveButtonCallback);
	solveButton->setColor(0.3f, 0.5f, 0.6f);

	materialStatusLabel = new Label(vec2i(0, 50), (char*)"");
	solvingPossibleLabel = new Label(vec2i(0, 50), (char*)"");

	solvingOptionsFrame->addChild(solveButton);
	solvingOptionsFrame->addChild(materialStatusLabel);
	
	stateElems[STATE_LIST_PIECES]->addChild(pieceListFrame);
	stateElems[STATE_LIST_PIECES]->addChild(solvingOptionsFrame);
}

void App::pieceListingUpdate(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	pieceListFrame->update();
}

void App::pieceListingOverlay(){

}
