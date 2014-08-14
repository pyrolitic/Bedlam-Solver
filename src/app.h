#ifndef APP_H
#define APP_H

#include <cmath>
#include <cstdlib>

#include <limits>
#include <list>
#include <vector>
#include <typeinfo>
#include <cxxabi.h> //this and abi::__cxa_demangle might only be available on gcc

#include <GL/glew.h>
#include <GL/freeglut.h>
#if GLUT_LEFT_BUTTON != 0 || GLUT_MIDDLE_BUTTON != 1 || GLUT_RIGHT_BUTTON != 2
#error rework the mouse handling cause the assumptions about the mouse button symbollic constants are wrong; use a hashtable or something instead
#endif

#include "ui/frame.h"
#include "ui/label.h"
#include "ui/scrolling_frame.h"
#include "ui/linear_container.h"
#include "ui/button.h"
#include "ui/counter.h"
#include "ui/text_input.h"
#include "ui/camera_control.h"

#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/text_render.h"
#include "graphics/vertex_array_object.h"

#include "maths/vec3.h"
#include "maths/mat4.h"

#include "solver.h"
#include "piece.h"

//colors
#define EDIT_BUTTON_COLOR        0x6666B2FF
#define DELETE_BUTTON_COLOR      0xB26666FF
#define COPIES_COUNTER_COLOR     0x661499FF
#define ADD_NEW_BUTTON_COLOR     0x66CC66FF
#define SOLVE_BUTTON_BAD_COLOR   0x4C7F99FF
#define SOLVE_BUTTON_GOOD_COLOR  0x5C8FA9FF
#define LIST_PIECE_FRAME_COLOR   0xB2B2B2FF

#define DISCARD_BUTTON_COLOR        0xB27F7FFF
#define SAVE_BUTTON_COLOR           0x7F7FB2FF
#define UNDO_BUTTON_ACTIVE_COLOR    0x667F33FF
#define UNDO_BUTTON_INACTIVE_COLOR  0x4C4C4CFF

#define SHOW_BUTTON_NEW          0x76DC76FF
#define SHOW_BUTTON_OLD          0x66CC66FF

#define NEUTRAL_TEXT_COLOR        0x111111FF
#define YELLOW_TEXT_COLOR         0x302011FF
#define BAD_TEXT_COLOR            0x501111FF

/*
def tohex(string):
	string = string.replace("f", "").replace(" ", "")
	groups = map(lambda x : int(float(x) * 255), string.split(","))
	return '0x' + ''.join(map(lambda x : ("%x" % x).upper().rjust(2, "0"), groups))
*/

template <class T>
inline std::string demangledTypeName(const T* thing){
#ifdef __GNUC__
	if (thing){
		char* demangled = abi::__cxa_demangle(typeid(*thing).name(), nullptr, nullptr, nullptr);
		std::string str(demangled);

		free(demangled);
		return str;
	}
	else{
		return "nullptr";
	}

#else //don't know how
	if (thing){
		return typeid(*thing).name();
	}
	else{
		return "nullptr";
	}
#endif
}

extern vec2i windowResolution;
extern int anisoLevel;
extern int drawingAt;

extern struct mouseInfo_s{
	UIElem* mouseButtonDownOn[3];
	vec2i mouseButtonDownAt[3];
	bool mouseDown[3];
	bool lastMouseDown[3]; 

	vec2i oldMouse;
	vec2i mouse;
} mouseInfo;

extern struct keyboardState_s{
	bool keyDown[256]; //keyboardCallback and keyboardUpCallback
	bool specialDown[256]; //specialCallback and specialUpCallback
} keyboardState;

//states
#define STATE_LIST_PIECES 0
#define STATE_EDIT_PIECE 1
#define STATE_SOLVE 2
#define STATE_SHOW_RESULT 3
#define NUM_STATES 4
extern int state;

//to be called by each state transition function
void cleanInput();

//global state needed across screens
extern std::list<Piece> pieces; //actual piece storage
extern vec3i worldSize;
extern Solver* solver;

class Screen{
public:
	UIElem* rootUI;

	Screen(){
		rootUI = nullptr;
	}

	virtual ~Screen(){
		if (rootUI) delete rootUI;
	}

	virtual void update() = 0;

	static Screen* screens[NUM_STATES];
};

#define INSTANCE_GETTER(X, symbol) static X& getInstance(){ \
		return *((X*)Screen::screens[symbol]); \
	}

//=====================================
//Piece listing state
//=====================================
#define LIST_PIECE_FRAME_WIDTH 250

struct listPieceEntry : public Frame{
	std::list<Piece>::iterator itemIt;
	std::string name;

	TextInput* nameEntry;
	Counter* copiesCounter;
	Label* statusLabel;

	listPieceEntry(const std::list<Piece>::iterator& it);
	~listPieceEntry();

	void editButtonCallback(UIElem* elem, vec2i at, int button);
	void deleteButtonCallback(UIElem* elem, vec2i at, int button);
	void copiesCounterChangeCallback(Counter* context, int oldValue, int newValue);
};

class PieceListScreen : public Screen{
public:
	PieceListScreen();
	~PieceListScreen();

	void transition();
	void update();

	void checkSolvingPossible(); //sets solvingPossible

	INSTANCE_GETTER(PieceListScreen, STATE_LIST_PIECES)

private:
	bool solvingPossible;

	ScrollingFrame* pieceListFrame;
	Button* newPieceButton;

	Frame* solvingOptionsFrame;
	Counter* dimCounters[3];

	Button* solveButton;
	Label* materialStatusLabel;

	void newPieceButtonCallback(UIElem* context, vec2i at, int button);
	void solveButtonCallback(UIElem* context, vec2i at, int button);
	void dimensionCounterChangeCallback(Counter* context, int oldValue, int newValue);
};
//=====================================
//Piece editing state
//=====================================
struct blockVert{
	vec3f position;
	vec3f normal;
	vec2f tex;
	uint8_t col[4];
};

class EditingScreen : public Screen{
public:
	#define EDITING_MAX_DIST_BLOCK 25
	#define FLOATING_GRID_EXTENT 3 //radius from the center

	struct pieceChange{
		bool removal;
		vec3i at;

		pieceChange(bool removal, vec3i at): 
			removal(removal), at(at) {}
	};

	EditingScreen();
	~EditingScreen();

	//transitions to editor with this piece reference
	//newPiece refers to whether discard will cause the piece to be deleted or not
	//the rationale is that since clicking the new piece button will transition to the editor,
	//clicking discard should discard the piece entirely rather than just the changes,
	//since the piece has no content or name if the changes are discarded
	void transitionWithPiece(listPieceEntry* ref, bool pieceIsNew);
	void update();

	INSTANCE_GETTER(EditingScreen, STATE_EDIT_PIECE)

private:
	CameraControl* cameraControl;
	vec3f cameraEye;
	vec3f mouseRayDir;

	mat4 projection; //ideally redone on every resize, but updated on update()
	mat4 modelView; //updated by mouseDragCallback

	Piece tempPiece; //a temporary piece separate from the stored ones
	listPieceEntry* parentRef; //reference to the actual piece being edited
	bool newPiece; //whether the discard button will cause the piece to be deleted or not

	//keep results of the last collision, so that the collision check won't be done every frame
	Piece::collisionResult lastCollision; //only valid when $collision is true
	vec3i extrudedBlock;
	bool collision;
	bool tooFar;

	std::list<pieceChange> history;

	//UI
	Button* discardButton;
	Button* undoButton;
	Button* saveButton;

	Label* eyeLabel;
	Label* mouseRayLabel;
	Label* cameraAngleLabel;

	Texture* blockTexture;
	Texture* blurredBlob;

	//graphics
	VertexArrayObject<blockVert>* blocksVAO;
	VertexArrayObject<blockVert>* singleBlockVAO;
	VertexArrayObject<blockVert>* gridVAO;

	void checkCollision();

	//for the camera control, 
	void mouseUpCallback(UIElem* context, vec2i at, int button); //to place or remove blocks
	void mouseMoveCallback(UIElem* context, vec2i to); //to ckeck for collisions
	void mouseDragCallback(UIElem* context, vec2i from, vec2i to, int button); //to change the modelview matrix

	void discardButtonCallback(UIElem* context, vec2i at, int button);
	void saveButtonCallback(UIElem* context, vec2i at, int button);
	void undoButtonCallback(UIElem* context, vec2i at, int button);

	void rebuildBlockVAO();
};

//=====================================
//Solving state
//=====================================
struct listSolutionEntry : public Frame{
	bool seen;
	Solver::solutionPiece* raw;

	listSolutionEntry(Solver::solutionPiece* block);
	~listSolutionEntry();

	void showButtonCallback(UIElem* elem, vec2i at, int button);
	void removeButtonCallback(UIElem* elem, vec2i at, int button);
};

class SolvingScreen : public Screen{
public:
	SolvingScreen();
	~SolvingScreen();

	void transition();
	void update();

	INSTANCE_GETTER(SolvingScreen, STATE_SOLVE)

private:
	ScrollingFrame* solvingSolutionList;
	Button* stopSolverButton;
	void stopSolverButtonCallback(UIElem* context, vec2i at, int button);
};

//=====================================
//Solution animation state
//=====================================
class SolutionScreen : public Screen{
public:
	SolutionScreen();
	~SolutionScreen();

	void transitionWithSolution(listSolutionEntry& ref);
	void update();

	INSTANCE_GETTER(SolutionScreen, STATE_SHOW_RESULT)

private:
	listSolutionEntry* animatedSolution;
	Button* nextPieceButton, * resetButton;
};

void init();
void end();

void reshapeCallback(int w, int h);
void displayCallback();
void keyboardDownCallback(unsigned char key, int x, int y);
void keyboardUpCallback(unsigned char key, int x, int y);
void specialDownCallback(int key, int x, int y);
void specialUpCallback(int key, int x, int y);
void mouseButtonCallback(int key, int state, int x, int y);
void mouseMotionCallback(int x, int y); //this is used for both active (with button held) and passive mouse motion
void updateCallback(int data); //called on a timer

#endif 