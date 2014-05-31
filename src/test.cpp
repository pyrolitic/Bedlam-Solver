#if 0
#include "ui_elem.h"
#include "frame.h"
#include "scrolling_frame.h"
#include "button.h"
#include "counter.h"

int main(){
	Frame f(0, 0, 10, 10);
	ScrollingFrame sf(0, 0, 100);
	Button b(0, 0, (char*)"stuff", false);
	Counter c(0, 0, -10, 10);
	return 0;
}
#endif

#if 0
#include "vec3.h"

int main(){
	vec3 f;
	vec3i i;
	return 0;
}
#endif

#if 1
#include "piece.h"
#include "solver.h"
#include "solver_control.h"

int main(){
	Piece p;
	Solver s(vec3i(10, 10, 10), std::list<Piece>());
	return 0;
}
#endif