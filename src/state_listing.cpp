#include <GL/glew.h>
#include <GL/freeglut.h>

#include "app.h"

listPieceEntry::listPieceEntry(const std::list<Piece>::iterator& pieceIt, const std::list<int>::iterator& copyIt) : Frame(), itemIt(pieceIt), copyIt(copyIt){
	//leaves
	nameEntry = new TextInput(LIST_PIECE_FRAME_WIDTH - 40, "piece name goes here.");

	Button* editButton = new Button((char*)"Edit");
	editButton->bindMouseUp(this, &listPieceEntry::editButtonCallback);
	editButton->setColor(EDIT_BUTTON_COLOR);

	Button* deleteButton = new Button((char*)"Delete");
	deleteButton->bindMouseUp(this, &listPieceEntry::deleteButtonCallback);
	deleteButton->setColor(DELETE_BUTTON_COLOR);

	copiesCounter = new Counter(1, 99, 1);
	copiesCounter->bindValueChange(this, &listPieceEntry::copiesCounterChangeCallback);
	copiesCounter->setColor(COPIES_COUNTER_COLOR);

	statusLabel = new Label((char*)"");
	statusLabel->clearFlag(UI_ACTIVE);

	//layout
	LinearContainer* buttonRow = new LinearContainer(LINEAR_CONTAINER_HORIZONTAL);
	buttonRow->addChild(editButton);
	buttonRow->addChild(deleteButton);
	buttonRow->addChild(copiesCounter);

	LinearContainer* rows = new LinearContainer(LINEAR_CONTAINER_VERTICAL);
	rows->addChild(nameEntry);
	rows->addChild(buttonRow);
	rows->addChild(statusLabel);

	addChild(rows);
	setColor(LIST_PIECE_FRAME_COLOR);
	update(); //resize accordingly
}

listPieceEntry::~listPieceEntry(){
	pieces.erase(itemIt);
	piecesCopies.erase(copyIt);
}

void listPieceEntry::editButtonCallback(UIElem* context, ivec2 at, int button){
	if (button == GLUT_LEFT_BUTTON){
		//transition to editor
		EditingScreen::getInstance().transitionWithPiece(this, false);
	}
}

void listPieceEntry::deleteButtonCallback(UIElem* context, ivec2 at, int button){
	if (button == GLUT_LEFT_BUTTON){
		delete this; //TODO: might be unsafe
		PieceListScreen::getInstance().checkSolvingPossible();
	}
}

void listPieceEntry::copiesCounterChangeCallback(Counter* context, int oldValue, int newValue){
	(*copyIt) = newValue;
	PieceListScreen::getInstance().checkSolvingPossible();
}

PieceListScreen::PieceListScreen(){
	rootUI = new UIElem();

	//piece list on the left side, all the way down
	pieceListFrame = new ScrollingFrame(500);
	pieceListFrame->setFlag(UI_STICK_TOP_LEFT);

	newPieceButton = new Button((char*)"Add new");
	newPieceButton->setColor(ADD_NEW_BUTTON_COLOR);
	newPieceButton->bindMouseUp(this, &PieceListScreen::newPieceButtonCallback);
	newPieceButton->setSize(ivec2(LIST_PIECE_FRAME_WIDTH, newPieceButton->getSize().y));
	pieceListFrame->addChild(newPieceButton);

	rootUI->addChild(pieceListFrame);

	//solving options on the top right
	LinearContainer* solvingBoxRows = new LinearContainer(LINEAR_CONTAINER_VERTICAL);
	LinearContainer* solvingBoxButtonRow = new LinearContainer(LINEAR_CONTAINER_HORIZONTAL);
	solvingOptionsFrame = new Frame();
	solvingOptionsFrame->setFlag(UI_STICK_TOP_RIGHT);

	for (int i = 0; i < 3; i++){
		worldSize.data[i] = 4; //make it match the original counter value
		dimCounters[i] = new Counter(1, 99, 4);
		dimCounters[i]->bindValueChange(this, &PieceListScreen::dimensionCounterChangeCallback);
		dimCounters[i]->setColor(COPIES_COUNTER_COLOR);
		solvingBoxButtonRow->addChild(dimCounters[i]);
	}

	solveButton = new Button((char*)"Solve");
	solveButton->bindMouseUp(this, &PieceListScreen::solveButtonCallback);
	solveButton->setColor(SOLVE_BUTTON_BAD_COLOR);
	solvingBoxButtonRow->addChild(solveButton);

	solvingBoxRows->addChild(solvingBoxButtonRow);

	materialStatusLabel = new Label((char*)"");
	solvingBoxRows->addChild(materialStatusLabel);

	solvingOptionsFrame->addChild(solvingBoxRows);
	solvingOptionsFrame->update();
	
	rootUI->addChild(solvingOptionsFrame);

	checkSolvingPossible(); //to reset the label colors
}

PieceListScreen::~PieceListScreen(){

}

void PieceListScreen::transition(){
	state = STATE_LIST_PIECES;
	cleanInput();
	checkSolvingPossible(); //piece likely got updated
}

void PieceListScreen::newPieceButtonCallback(UIElem* context, ivec2 at, int button){
	if (button == GLUT_LEFT_BUTTON){
		//create a new piece ui element
		pieces.emplace_front();
		piecesCopies.emplace_front(1);
		listPieceEntry* p = new listPieceEntry(pieces.begin(), piecesCopies.begin());
		pieceListFrame->addChildBefore(p, newPieceButton);

		//transition to editor
		EditingScreen::getInstance().transitionWithPiece(p, true);
	}
}

void PieceListScreen::solveButtonCallback(UIElem* context, ivec2 at, int button){
	if (button == GLUT_LEFT_BUTTON){
		checkSolvingPossible(); //might not be needed

		if (solvingPossible){
			SolvingScreen::getInstance().transition();
		}
	}
}

void PieceListScreen::dimensionCounterChangeCallback(Counter* context, int oldValue, int newValue){
	//update worldSize
	for (int i = 0; i < 3; i++){
		worldSize.data[i] = dimCounters[i]->getValue();
	}

	checkSolvingPossible();
}

void PieceListScreen::checkSolvingPossible(){
	solvingPossible = false; //many ways to fail
	bool pieceTooLarge = false;

	int worldSpace = worldSize.x * worldSize.y * worldSize.z;
	int material = 0;

	for (auto child : pieceListFrame->getChildren()){
		listPieceEntry* lp;
		if ((lp = dynamic_cast<listPieceEntry*>(child))){
			//update copies amount in pieces

			int mass = lp->itemIt->mass();
			material += *(lp->copyIt) * mass;

			if (mass == 0){
				lp->statusLabel->setFlag(UI_ACTIVE);
				lp->statusLabel->setText("No material.");
				lp->statusLabel->setColor(BAD_TEXT_COLOR);
			}
			else if (!(lp->itemIt->getSize() <= worldSize)){
				pieceTooLarge = true;
				lp->statusLabel->setFlag(UI_ACTIVE);
				lp->statusLabel->setText("Larger than the world.");
				lp->statusLabel->setColor(BAD_TEXT_COLOR);
			}
			else{
				lp->statusLabel->clearFlag(UI_ACTIVE);
			}
		}
	}

	const char* extraInfo;

	if (material == worldSpace and !pieceTooLarge){
		extraInfo = ".";
		solvingPossible = true; //only one to succeed
		solveButton->setColor(SOLVE_BUTTON_GOOD_COLOR);
		materialStatusLabel->setColor(NEUTRAL_TEXT_COLOR);
	}
	else{
		if (material < worldSpace){
			materialStatusLabel->setColor(YELLOW_TEXT_COLOR);
			extraInfo = ", too little.";
		}
		else{
			materialStatusLabel->setColor(BAD_TEXT_COLOR);
			extraInfo = ", too much.";
		}
		solveButton->setColor(SOLVE_BUTTON_BAD_COLOR);
	}

	char buf[256];
	snprintf(buf, 256, "%d material out of %d space%s", material, worldSpace, extraInfo);
	materialStatusLabel->setText(buf);
}

void PieceListScreen::update(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
