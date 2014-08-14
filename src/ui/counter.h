#ifndef COUNTER_H
#define COUNTER_H

#include <string>

#include <GL/glew.h>

#include "../graphics/text_render.h"

#include "frame.h"

class Counter : public Frame{
public:
	#define COUNTER_INDICATOR_WIDTH 15 //pixels, space for the + and - (individually)

	Counter(int minVal, int maxVal, int initVal = 0) : Frame() {
		min = minVal;
		max = maxVal;
		value = initVal;

		//find the longest reprsentable value and make a *printf format string that ensures enough leading spaces
		int minLength = (minVal < 0)? (1 + (int)ceil(log10(-minVal))) : (ceil(log10(minVal)));
		int maxLength = (maxVal < 0)? (1 + (int)ceil(log10(-maxVal))) : (ceil(log10(maxVal)));
		format = (char*) malloc(16);
		snprintf(format, 10, "%%%dd", std::max(minLength, maxLength));

		char buf[256];
		snprintf(buf, 256, format, value);

		//use a button to display the value, position it centered, but don't make a margin around it
		Button* displayButton = new Button(buf);
		setSize(vec2i(FRAME_ROUNDED_RADIUS * 2 + COUNTER_INDICATOR_WIDTH * 2 + displayButton->getSize().x, displayButton->getSize().y));
		UIElem::addChild(displayButton); //don't offset it
		displayButton->setPosition(vec2i(pos.x + size.x / 2 - displayButton->getSize().x / 2, pos.y)); //offset it here

		Label* plusIndicator = new Label("+");
		Label* minusIndicator = new Label("-");
		int textWidth = plusIndicator->getSize().x;
		int textHeight = plusIndicator->getSize().y;
		plusIndicator->setPosition(pos + vec2i((FRAME_ROUNDED_RADIUS + COUNTER_INDICATOR_WIDTH) / 2 - textWidth / 2, size.y / 2 - textHeight / 2));
		
		textWidth = minusIndicator->getSize().x;
		textHeight = minusIndicator->getSize().y;
		minusIndicator->setPosition(pos + vec2i(FRAME_ROUNDED_RADIUS + COUNTER_INDICATOR_WIDTH + ((Button*)children.front())->getSize().x + (COUNTER_INDICATOR_WIDTH + FRAME_ROUNDED_RADIUS) / 2 - textWidth / 2, size.y / 2 - textHeight / 2));
		
		UIElem::addChild(plusIndicator);
		UIElem::addChild(minusIndicator);

		plusIndicator->setColor(0xFF7F7FFF);
		minusIndicator->setColor(0xFF7F7FFF);
	}

	~Counter(){
		free(format);
	}

	void update(){
		//don't do the Frame resizing
		//also don't update the child button since it gets updated on setValue
	}

	int getValue(){
		return value;
	}

	void setValue(int newVal){
		int oldValue = value;
		value = newVal;
		if (value < min) value = min;
		if (value > max) value = max;

		char buf[256];
		snprintf(buf, 256, format, value);

		((Button*)children.front())->setText(buf);
		if (onValueChangeCallback and oldValue != value) onValueChangeCallback(this, oldValue, value);
	}

	void setLimits(int min, int max){
		this->min = min;
		this->max = max;
	}

	vec2i getLimits(){
		return vec2i(min, max);
	}

	//use Frame::draw()
	/*void draw(int depth)
		Frame::draw();//draw frame and displayButton button

		const char* plus = "+";
		const char* minus = "-";

		int textWidth, textHeight;
		TextRender::metrics(plus, &textWidth, &textHeight);
		vec2i renderAt = pos + vec2i((FRAME_ROUNDED_RADIUS + COUNTER_INDICATOR_WIDTH) / 2 - textWidth / 2, size.y / 2 - textHeight / 2);
		TextRender::render(plus, renderAt.x, renderAt.y); //overlay value text

		TextRender::metrics(minus, &textWidth, &textHeight);
		renderAt = pos + vec2i(FRAME_ROUNDED_RADIUS + COUNTER_INDICATOR_WIDTH + ((Button*)children.front())->getSize().x + (COUNTER_INDICATOR_WIDTH + FRAME_ROUNDED_RADIUS) / 2 - textWidth / 2, size.y / 2 - textHeight / 2);
		TextRender::render(minus, renderAt.x, renderAt.y); //overlay value text
	}*/

	template <class X>
	inline void bindValueChange(X* instance, void(X::*function)(Counter*, int, int)) {
		onValueChangeCallback.bind(instance, function);
	}

	void addChild(UIElem* child){
		//doesn't make sense
		printf("warning: trying to add child to a counter\n");
	}

protected:
	void privateOnMouseUp(vec2i at, int button){
		if (button == GLUT_LEFT_BUTTON){
			int v = value;

			if (at.x > pos.x + size.x / 2) v--;
			else v++;

			setValue(v);
		}
	}

	bool privateOnWheel(vec2i at, int delta) {
		setValue(value + delta);
		return true;
	}

private:
	char* format;
	int min, max;
	int widestVal;
	int value;

	fastdelegate::FastDelegate3<Counter*, int, int> onValueChangeCallback;
};

#endif
