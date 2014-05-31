#ifndef SCROLLING_FRAME_H
#define SCROLLING_FRAME_H

#include <cstdint>

#include <string>

#include <GL/glew.h>

#include "frame.h"
#include "ui_elem.h"
#include "text_render.h"

class ScrollingFrame : public Frame{
public:
	#define SCROLLING_FRAME_ELMENT_SEPARATION 20

	ScrollingFrame(int x, int y, int height) : Frame(x, y, 0, height){
	}

	virtual ~ScrollingFrame() {}

	virtual void update(){
		//compute the dimensions and position each element
		int contentHeight = 0;
		int maxWidth = 0;

		for (auto it = UIElem::children.begin(); it != UIElem::children.end(); it++){
			UIElem* elem = *it;
			elem->setPosition(0, (int)-scrollingPosition + contentHeight);

			int w = elem->getWidth();
			int h = elem->getHeight();

			if (w > maxWidth) maxWidth = w;
			contentHeight += h;
			if (std::next(it) != UIElem::children.end()) contentHeight += SCROLLING_FRAME_ELMENT_SEPARATION;
		}

		setSize(maxWidth, height); //don't make longer than before
	}

	void scroll(int delta){
		scrollingPosition += delta;
		if (scrollingPosition < 0.0f) scrollingPosition = 0.0f;
		if (scrollingPosition > contentHeight - height) scrollingPosition = contentHeight - height;
	}

	virtual void draw(){//float r, float g, float b){
		Frame::draw();
		UIElem::draw();
	}

	virtual UIElem* collides(int x, int y){
		return Frame::collides(x, y);
	}

protected:
	float scrollingPosition;
	int width, height;
	int contentHeight;
	static Texture* roundedBox;
};

#endif