#ifndef BUTTON_H
#define BUTTON_H

#include <cstdint>

#include <string>

#include "frame.h"
#include "label.h"

class Button : public Frame{
public:
	Button(const char* text, bool toggleable = false) : Frame() {
		Label* label = new Label(text);
		Frame::addChild(label);
		Frame::update(); //resize to fit the label exactly, once
		
		value = false;
		toggle = toggleable;
	}

	virtual ~Button(){
	}

	void update(){
		if (flags & UI_AUTO_RESIZE){
			Frame::update();
		}
	}

	bool getValue() const{
		return value;
	}

	std::string getText() const{
		Label* label = (Label*)children.front();
		return label->getText();
	}

	void setText(const char* newText){
		Label* label = (Label*)children.front();
		label->setText(newText);
		update(); //resize
	}

	//use Frame::draw()

	void addChild(UIElem* child){
		printf("warning: trying to add child to a button\n");
	}

protected:
	void privateOnMouseUp(ivec2 at, int button){
		if (button == GLUT_LEFT_BUTTON){
			if (toggle){
				value = !value;
			}
		}
	}

private:
	bool value;
	bool toggle;
};

#endif