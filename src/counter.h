#ifndef COUNTER_H
#define COUNTER_H

#include <string>

#include <GL/glew.h>

#include "frame.h"
#include "text_render.h"

class App;
extern App* app; //from main.cpp

class Counter : public Frame{
public:
	#define COUNTER_INDICATOR_WIDTH 15 //pixels, space for the + and - (individually)

	Counter(vec2i pos, int minVal, int maxVal, int initVal = 0, void (App::*callback)(Counter*, int, int) = nullptr) : Frame(pos, vec2i(0, 0)) {
		int textHeight;
		int minValWidth, maxValWidth;
		TextRender::metrics(std::to_string(minVal).c_str(), &minValWidth, &textHeight);
		TextRender::metrics(std::to_string(maxVal).c_str(), &maxValWidth, &textHeight);
		widestVal = std::max(minValWidth, maxValWidth);

		min = minVal;
		max = maxVal;
		value = initVal;
		this->callback = callback;

		//use a button to display the value, position it centered, but don't make a margin around it
		Button* label = new Button(vec2i(0, 0), std::to_string(value).c_str());
		Frame::setSize(vec2i(FRAME_ROUNDED_RADIUS * 2 + COUNTER_INDICATOR_WIDTH * 2 + label->getSize().x, label->getSize().y));
		UIElem::addChild(label); //don't offset it
		label->setPosition(vec2i(pos.x + size.x / 2 - label->getSize().x / 2, pos.y)); //offset it here
	}

	~Counter(){
	}

	int getValue(){
		return value;
	}

	void setValue(int newVal){
		value = newVal;
		if (value < min) value = min;
		if (value > max) value = max;
	}

	void setLimits(int min, int max){
		this->min = min;
		this->max = max;
	}

	vec2i getLimits(){
		return vec2i(min, max);
	}

	void onMouseDown(vec2i at, int button){
		if (button == GLUT_LEFT_BUTTON){
			int oldValue = value;

			if (at.x > pos.x + size.x / 2){
				if (value > min) value--;
			}
			else{
				if (value < max) value++;
			}

			((Button*)children.front())->setText(std::to_string(value).c_str());
			if (callback and oldValue != value) (app->*callback)(this, oldValue, value);
		}
	}

	void draw(){
		Frame::draw();//fr, fg, fb); //draw frame and label button
		glColor4f(1.0f, 0.5f, 0.5f, 1.0f);

		const char* plus = "+";
		const char* minus = "-";

		int textWidth, textHeight;
		TextRender::metrics(plus, &textWidth, &textHeight);
		vec2i renderAt = pos + vec2i((FRAME_ROUNDED_RADIUS + COUNTER_INDICATOR_WIDTH) / 2 - textWidth / 2, size.y / 2 - textHeight / 2);
		TextRender::render(plus, renderAt.x, renderAt.y); //overlay value text

		TextRender::metrics(minus, &textWidth, &textHeight);
		renderAt = pos + vec2i(FRAME_ROUNDED_RADIUS + COUNTER_INDICATOR_WIDTH + ((Button*)children.front())->getSize().x + (COUNTER_INDICATOR_WIDTH + FRAME_ROUNDED_RADIUS) / 2 - textWidth / 2, size.y / 2 - textHeight / 2);
		TextRender::render(minus, renderAt.x, renderAt.y); //overlay value text
	}

	void addChild(UIElem* child){
		//doesn't make sense
		printf("warning: trying to add child to a counter\n");
	}

protected:
	bool privateOnWheel(vec2i at, int delta) {
		int oldValue = value;
		value += delta;
		if (value < min) value = min;
		if (value > max) value = max;
		((Button*)children.front())->setText(std::to_string(value).c_str());
		if (callback and oldValue != value) (app->*callback)(this, oldValue, value);
		return true;
	}

private:
	void (App::*callback)(Counter*, int, int);
	int min, max;
	int widestVal;
	int value;
};

#endif