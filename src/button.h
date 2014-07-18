#ifndef BUTTON_H
#define BUTTON_H

#include <cstdint>

#include <string>

#ifndef TEST_BUILD //remove all references to gl when doing unit tests
#include <GL/glew.h>

#include "text_render.h"
#endif
#include "frame.h"

class App;
extern App* app; //from main.cpp

class Button : public Frame{
public:
	#define BUTTON_MIN_TEXT_HEIGHT 10 //pixels

	Button(vec2i pos, const char* text, void (App::*callback)(UIElem*, vec2i, int) = nullptr, bool toggleable = false) : Frame(pos, vec2i(0, 0)), text(text) {
		this->callback = callback;
		int textWidth;
#ifndef TEST_BUILD //remove all references to gl when doing unit tests
		TextRender::metrics(text, &textWidth, &textHeight);
		Frame::setSize(vec2i(textWidth + 2 * FRAME_ROUNDED_RADIUS, 2 * FRAME_ROUNDED_RADIUS + TextRender::getReasonableHeight()));
#endif
		
		value = false;
		toggle = toggleable;
	}

	void onMouseDown(vec2i at, int button){
		if (callback) (app->*callback)(this, at, button);
	}

	//let on wheel return false

	virtual ~Button(){
	}

	std::string getText(){
		return text; //copy not reference
	}

#ifndef TEST_BUILD //remove all references to gl when doing unit tests
	void setText(const char* newText){
		text.assign(newText);
		int textWidth;
		TextRender::metrics(text.c_str(), &textWidth, &textHeight);
		Frame::setSize(vec2i(textWidth + 2 * FRAME_ROUNDED_RADIUS, 2 * FRAME_ROUNDED_RADIUS + TextRender::getReasonableHeight()));
	}

	void draw(){//float tr, float tg, float tb, float fr, float fg, float fb){
		Frame::draw();//fr, fg, fb); //draw frame underneath
		glColor4f(0.1f, 0.1f, 0.1f, 1.0f);//glColor4f(tr, tg, tb, 1.0f);
		vec2i renderAt = pos + vec2i(FRAME_ROUNDED_RADIUS, FRAME_ROUNDED_RADIUS); //overlay text, centered
		TextRender::render(text.c_str(), renderAt.x, renderAt.y);
	}
#endif

	void addChild(UIElem* child){
		printf("warning: trying to add child to a button\n");
	}

private:
	std::string text;
	int textHeight;
	bool value;
	bool toggle;
	void (App::*callback)(UIElem* context, vec2i at, int button);
};

#endif