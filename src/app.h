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

#include "frame.h"
#include "scrolling_frame.h"
#include "button.h"
#include "counter.h"

#include "solver.h"
#include "piece.h"
#include "texture.h"
#include "text_render.h"
#include "vec3.h"

class App{
public:
	App(){}
	~App(){
		delete editorElems;
		delete listingStateElems;
		delete solvingElems;
		delete solutionElems;
	}

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

	//3d view world space
	vec3 cameraEye;
	vec3 mouseRayDir;

	//=====================================
	//Piece listing state
	//=====================================
	struct listPiece : public Frame{
		Piece item;
		std::string name;
		int copies;

		listPiece() : Frame(0, 0, 200, 60){
			UIElem::addChild(new Button(0, 20, (char*)"edit"));
			UIElem::addChild(new Button(30, 20, (char*)"delete"));
			UIElem::addChild(new Counter(50, 20, 0, 10));
		}
	};

	ScrollingFrame* pieceListFrame;

	struct listSolvingOptions : public Frame{
		listSolvingOptions() : Frame(0, 500, 200, 50){
			//UIElem::addChild(new Label((char*)"width x height x depth"));
			//UIElem::addChild(new Button((char*) ));
		}
	};

	listSolvingOptions* solvingOptionsFrame;
	UIElem* listingStateElems; //container of the above

	//=====================================
	//Piece editing state
	//=====================================
	#define FLOATING_GRID_EXTENT 3 //radius from the center
	Piece* editedPiece;
	int editingLayer; //which layer currently editing
	bool blocksLaid; //how many blocks is the peice made of
	bool checkRayCollision; //wether to check for block collision (if the mouse moved or if a block was laid or deleted)

	//keep results of the last collision, so that the collision check won't be done every frame
	Piece::collisionResult lastCollision;
	vec3i extrudedBlock;
	bool editorCollision;

	struct pieceChange{
		bool removal;
		vec3i at;

		pieceChange(bool removal, vec3i at): 
			removal(removal), at(at) {}
	};

	std::list<pieceChange> editingHistory;
	std::list<pieceChange>::iterator editingHistoryCurrentStep;

	Frame* editorFrame;
	Button* discardButton, * saveButton, * undoButton;
	UIElem* editorElems;

	//=====================================
	//Solving state
	//=====================================
	//Solver solver; //TODO: fix the solver
	struct listSolution : public Frame{
		
		int axis;
		bool seen;

		void draw(){
			if (seen){
				setColor(0.4f, 0.8f, 0.4f);
			}
			Frame::draw();
		}
	};

	Frame* solvingFrame;
	UIElem* solvingElems;

	//=====================================
	//Solution animation state
	//=====================================
	Solver::solution* animatedSolution;
	Button* nextPieceButton, * resetButton;
	UIElem* solutionElems;

	void init();

	//callbacks
	void reshape(int w, int h);
	void update();
	void keyboard(uint8_t key, int x, int y);
	void mouseButton(int key, int state, int x, int y);
	void mouseMotion(int x, int y); //this is used for both active (with button held) and passive mouse motion
	void special(int key, int x, int y);
	void idle();

private:
	//helpers
	bool checkButton(int button);
	void drawCube(int x, int y, int z);
	void drawCubeOutlines(int x, int y, int z);
	static unsigned int timeMilli();
};



