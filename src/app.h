#ifndef APP_H
#define APP_H

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
#include <typeinfo>
#include <cxxabi.h> //this and abi::__cxa_demangle might only be available on gcc

#include <GL/glew.h>
#include <GL/glut.h>
#if GLUT_LEFT_BUTTON != 0 || GLUT_MIDDLE_BUTTON != 1 || GLUT_RIGHT_BUTTON != 2
#error rework the mouse handling cause the assumptions about the mouse button symbollic constants are wrong; use a hashtable or something instead
#endif

#include "frame.h"
#include "label.h"
#include "scrolling_frame.h"
#include "button.h"
#include "counter.h"
#include "text_input.h"
#include "camera_control.h"

#include "solver.h"
#include "piece.h"
#include "texture.h"
#include "text_render.h"
#include "vec3.h"


//helper
/*template <class T>
inline std::string demangledTypeName(const T& thing){
	char* demangled = abi::__cxa_demangle(typeid(thing).name(), nullptr, nullptr, nullptr);
	std::string str(demangled);
	free(demangled);
	return str;
}*/

template <class T>
inline std::string demangledTypeName(const T* thing){
	if (thing){
		char* demangled = abi::__cxa_demangle(typeid(*thing).name(), nullptr, nullptr, nullptr);
		std::string str(demangled);
		free(demangled);
		return str;
		//return demangledName(*thing);
	}
	else{
		return "nullptr";
	}
}



class App{
public:
	int width, height; //window dimensions
	int anisoLevel; //[1, 16]
	
	Texture* blockTexture;
	Texture* blurredBlob;
	
	int drawingAt;

	//used for dragging
	UIElem* mouseButtonDownOn[3]; //do not interact with, just compare the pointer, since it could be dangling
	vec2i mouseButtonDownAt[3];
	bool mouseDown[3];
	bool lastMouseDown[3]; 

	//position
	vec2i oldMouse;
	vec2i mouse;

	//states
	#define STATE_LIST_PIECES 0
	#define STATE_EDIT_PIECE 1
	#define STATE_SOLVE 2
	#define STATE_SHOW_RESULT 3
	#define NUM_STATES 4
	int state; //current state
	const char* stateNames[4] = {"Pieces", "Editor", "Solving", "Result"};

	//3d view world space
	vec3 cameraEye;
	vec3 mouseRayDir;
	
	void (App::*stateUpdate[NUM_STATES])(void);
	void (App::*stateOverlay[NUM_STATES])(void);
	UIElem* stateElems[NUM_STATES]; //root container for UI of each state
	//=====================================
	//Piece listing state
	//=====================================
	#define LIST_PIECE_FRAME_WIDTH 250

	struct listPiece : public Frame{
		std::list<Piece>::iterator itemIt;
		std::string name;

		TextInput* nameEntry;
		Counter* copiesCounter;

		listPiece(const std::list<Piece>::iterator& it);
		~listPiece();
	};

	void listPieceEditButtonCallback(UIElem* context, vec2i at, int button);
	void listPieceDeleteButtonCallback(UIElem* context, vec2i at, int button);

	std::list<Piece> pieces; //actual piece storage
	ScrollingFrame* pieceListFrame;

	Button* newPieceButton;
	void newPieceButtonCallback(UIElem* context, vec2i at, int button);

	Frame* solvingOptionsFrame;
	Counter* dimCounters[3];
	Button* solveButton;
	Label* materialStatusLabel;
	Label* solvingPossibleLabel;
	void solveButtonCallback(UIElem* context, vec2i at, int button);

	//for both the piece copy and world size counters
	void counterChangeCallback(Counter* context, int oldValue, int newValue);

	bool solvingPossible;
	void pieceListCheckSolvingPossiblity();

	void pieceListInit();
	void pieceListingUpdate();
	void pieceListingOverlay();

	//=====================================
	//Piece editing state
	//=====================================
	CameraControl* editingCameraControl;
	vec3f editingCameraEye;

	#define FLOATING_GRID_EXTENT 3 //radius from the center
	Piece editedPiece; //a temporary piece separate from the stored ones
	listPiece* editedPieceParentRef; //reference to the actual piece being edited
	bool editingNewPiece; //whether the discard button will cause the piece to be deleted or not
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

	Button* discardButton;
	Button* undoButton;
	Button* saveButton;

	void discardButtonCallback(UIElem* context, vec2i at, int button);
	void saveButtonCallback(UIElem* context, vec2i at, int button);
	void undoButtonCallback(UIElem* context, vec2i at, int button);

	void editingInit();
	void editingUpdate();
	void editingOverlay();

	//=====================================
	//Solving state
	//=====================================
	//Solver solver; //TODO: fix the solver
	struct listSolution : public Frame{
		bool seen;
		Solver::solutionPiece* raw;

		listSolution(Solver::solutionPiece* block);
		~listSolution();
		void draw();
	};

	void listSolutionShowButtonCallback(UIElem* context, vec2i at, int button);
	void listSolutionRemoveButtonCallback(UIElem* context, vec2i at, int button);

	ScrollingFrame* solvingSolutionList;
	Solver* solver;

	Button* stopSolverButton;
	void stopSolverButtonCallback(UIElem* context, vec2i at, int button);

	void solvingInit();
	void solvingUpdate();
	void solvingOverlay();

	//=====================================
	//Solution animation state
	//=====================================
	listSolution* animatedSolution;
	Button* nextPieceButton, * resetButton;


	void solutionInit();
	void solutionUpdate();
	void solutionOverlay();
	
	UIElem* elemHover;

	//other
	void transition(int newState);
	void init();
	void end();

	//callbacks
	void reshape(int w, int h);
	void update();
	void keyboardDown(int key, int x, int y);
	void keyboardUp(int key, int x, int y);
	void specialDown(int key, int x, int y);
	void specialUp(int key, int x, int y);
	void mouseButton(int key, int state, int x, int y);
	void mouseMotion(int x, int y); //this is used for both active (with button held) and passive mouse motion
	void idle();

private:
	//helpers
	void drawCube(int x, int y, int z);
	void drawCubeOutlines(int x, int y, int z);
	unsigned int timeMilli();
};

#endif 