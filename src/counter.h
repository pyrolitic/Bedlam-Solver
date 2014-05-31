#ifndef COUNTER_H
#define COUNTER_H

#include <string>

#include <GL/glew.h>

#include "frame.h"
#include "text_render.h"

class Counter : public Frame{
private:
	class CounterButton : public Button{
	public:
		CounterButton(int x, int y, char* text, int valueDelta) : Button(x, y, text, false) {
			delta = valueDelta;
		}

		void onLeftClick(){
			assert(dynamic_cast<Counter*>(UIElem::parent));
			((Counter*)UIElem::parent)->value += delta;
		}

	private:
		int delta;
	};

public:
	#define COUNTER_SEPARATION 5 //pixels

	Counter(int x, int y, int minVal, int maxVal) : Frame(x, y, 0, 0) {
		int textHeight;
		int minValWidth, maxValWidth;
		TextRender::metrics(std::to_string(minVal).c_str(), &minValWidth, &textHeight);
		TextRender::metrics(std::to_string(maxVal).c_str(), &maxValWidth, &textHeight);
		UIElem::setSize(std::max(minValWidth, maxValWidth), textHeight);

		min = minVal;
		max = maxVal;
		value = 0;

		UIElem::addChild(new CounterButton(0, 0, (char*)"+", 1));
		UIElem::addChild(new CounterButton(COUNTER_SEPARATION * 2 + width, 0, (char*)"-", -1));
	}

	~Counter(){
	}

	int getValue(){
		return value;
	}

	void setValue(int newVal){
		value = newVal;
	}

	void onWheel(int delta) {
		value += delta;
	}

	void draw(){
		Frame::draw();//fr, fg, fb); //draw frame and butons
		glColor4f(1.0f, 0.5f, 0.5f, 1.0f);
		std::string str = std::to_string(value).c_str();
		int textWidth, textHeight;
		TextRender::metrics(str.c_str(), &textWidth, &textHeight);

		TextRender::render(str.c_str(), px + FRAME_ROUNDED_RADIUS + (*children.begin())->getWidth() + COUNTER_SEPARATION,
		                                py + FRAME_ROUNDED_RADIUS + Frame::height / 2 - textHeight / 2); //overlay value text
	}

	void addChild(UIElem* child){
		//doesn't make sense
		printf("warning: trying to add child to a counter\n");
	}

private:
	int min, max;
	int value;
};

#endif