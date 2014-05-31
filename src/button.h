#ifndef BUTTON_H
#define BUTTON_H

#include <cstdint>

#include <string>

#include <GL/glew.h>

#include "frame.h"
#include "text_render.h"

class Button : public Frame{
public:
	#define BUTTON_HEIGHT 30 //pixels

	Button(int x, int y, char* text, bool toggleable = false) : Frame(x, y, 0, 0), text(text) {
		int textWidth;
		TextRender::metrics(text, &textWidth, &textHeight);
		Frame::setSize(textWidth, std::max(BUTTON_HEIGHT - 2 * FRAME_ROUNDED_RADIUS, textHeight));

		value = false;
		toggle = toggleable;
	}

	~Button(){
	}

	std::string getText(){
		return text; //copy not reference
	}

	void setText(char* newText){
		text.assign(newText);
		int textWidth;
		TextRender::metrics(text.c_str(), &textWidth, &textHeight);
		Frame::setSize(textWidth, std::max(BUTTON_HEIGHT, textHeight));
	}

	void draw(){//float tr, float tg, float tb, float fr, float fg, float fb){
		Frame::draw();//fr, fg, fb); //draw frame underneath
		glColor4f(1.0f, 0.5f, 0.5f, 1.0f);//glColor4f(tr, tg, tb, 1.0f);
		TextRender::render(text.c_str(), px + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + Frame::height / 2 - textHeight / 2); //overlay text, centered
	}

	void addChild(UIElem* child){
		printf("warning: trying to add child to a button\n");
	}

private:
	std::string text;
	int textHeight;
	bool value;
	bool toggle;
};

#endif