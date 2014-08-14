#include <GL/glew.h>
#include <GL/freeglut.h>

#include "app.h"

SolutionScreen::SolutionScreen(){
	rootUI = new UIElem();
}

SolutionScreen::~SolutionScreen(){
	
}

void SolutionScreen::update(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}
