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
		CounterButton(vec2i pos, char* text, int valueDelta) : Button(pos, text, false) {
			delta = valueDelta;
		
}
		void onLeftClick(){
			Counter* counter = dynamic_cast<Counter*>(UIElem::parent);
			assert(counter);

			counter->setValue(counter->getValue() + delta);
		}

	private:
		int delta;
	};

public:
	#define COUNTER_SEPARATION 5 //pixels

	Counter(vec2i pos, int minVal, int maxVal, int initVal = 0) : Frame(pos, vec2i(0, 0)) {
		int textHeight;
		int minValWidth, maxValWidth;
		TextRender::metrics(std::to_string(minVal).c_str(), &minValWidth, &textHeight);
		TextRender::metrics(std::to_string(maxVal).c_str(), &maxValWidth, &textHeight);
		widestVal = std::max(minValWidth, maxValWidth);

		min = minVal;
		max = maxVal;
		value = initVal;

		CounterButton* more = new CounterButton(vec2i(0, 0), (char*)"+", 1);
		CounterButton* fewer = new CounterButton(vec2i(more->getSize().x + COUNTER_SEPARATION * 2 + widestVal, 0), (char*)"-", -1);
		Frame::setSize(vec2i(fewer->getPosition().x + fewer->getSize().x + 2 * FRAME_ROUNDED_RADIUS, textHeight + 2 * FRAME_ROUNDED_RADIUS));

		UIElem::addChild(more);
		UIElem::addChild(fewer);
	}

	~Counter(){
	}

	int getValue(){
		return value;
	}

	void setValue(int newVal){
		value = newVal;
	}

	void setLimits(int min, int max){
		this->min = min;
		this->max = max;
	}

	vec2i getLimits(){
		return vec2i(min, max);
	}

	void onWheel(int delta) {
		value += delta;
	}

	void draw(){
		printf("(%d %d) (%d %d)\n", pos.x, pos.y, size.x, size.y);
		Frame::draw();//fr, fg, fb); //draw frame and butons
		glColor4f(1.0f, 0.5f, 0.5f, 1.0f);
		std::string str = std::to_string(value).c_str();
		int textWidth, textHeight;
		TextRender::metrics(str.c_str(), &textWidth, &textHeight);
		vec2i renderAt = pos + vec2i(size.x / 2 + textWidth / 2, //FRAME_ROUNDED_RADIUS + (*children.begin())->getSize().x + COUNTER_SEPARATION, 
		                             size.y / 2 - textHeight / 2);
		TextRender::render(str.c_str(), renderAt.x, renderAt.y); //overlay value text
	}

	void addChild(UIElem* child){
		//doesn't make sense
		printf("warning: trying to add child to a counter\n");
	}

private:
	int min, max;
	int widestVal;
	int value;
};

#endif