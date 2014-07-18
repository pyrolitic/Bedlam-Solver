#ifndef LABEL_H
#define LABEL_H

#include <cstdint>

#include <string>

#include <GL/glew.h>

#include "text_render.h"
#include "frame.h"

class Label : public UIElem{
public:
	#define BUTTON_MIN_TEXT_HEIGHT 10 //pixels

	Label(vec2i pos, const char* initialText) : UIElem(pos), text(initialText) {
		TextRender::metrics(text.c_str(), &(size.x), &(size.y));
	}

	virtual ~Label(){
	}

	std::string getText(){
		return text; //copy not reference
	}

	void setText(const char* newText){
		text.assign(newText);
		TextRender::metrics(text.c_str(), &(size.x), &(size.y));
	}

	void draw(){
		glColor4f(r, g, b, a);
		TextRender::render(text.c_str(), pos.x, pos.y);
	}

	void addChild(UIElem* child){
		printf("warning: trying to add child to a label\n");
	}

private:
	std::string text;
};

#endif