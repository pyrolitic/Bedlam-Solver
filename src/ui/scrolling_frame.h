#ifndef SCROLLING_FRAME_H
#define SCROLLING_FRAME_H

#include <cstdint>

#include <string>

#include "frame.h"
#include "ui_elem.h"

class ScrollingFrame : public Frame{
public:
	#define SCROLLING_FRAME_ELMENT_SEPARATION FRAME_ROUNDED_RADIUS

	ScrollingFrame(int height) : Frame(){
		setSize(ivec2(0, height));
		scrollingPosition = 0.0f;
		contentHeight = 0;
	}

	virtual ~ScrollingFrame() {}

	virtual void update(){
		//update all active children first
		for (auto child : children){
			if (child->getFlags() & UI_ACTIVE){
				child->update();
			}
		}

		//compute the dimensions and position each element
		int contentHeight = 0;
		int maxWidth = 0;

		for (auto it = UIElem::children.begin(); it != UIElem::children.end(); it++){
			UIElem* elem = *it;
			elem->setPosition(pos + ivec2(FRAME_ROUNDED_RADIUS, FRAME_ROUNDED_RADIUS + (int)scrollingPosition + contentHeight));

			ivec2 dim = elem->getSize();

			if (dim.x > maxWidth) maxWidth = dim.x;
			contentHeight += dim.y;
			if (std::next(it) != UIElem::children.end()) contentHeight += SCROLLING_FRAME_ELMENT_SEPARATION;
		}

		setSize(ivec2(maxWidth + 2 * FRAME_ROUNDED_RADIUS, size.y)); //don't make it longer than before
	}

	virtual void draw(int depth){
		Frame::draw(depth);
		//UIElem::draw();
	}

	virtual UIElem* collides(ivec2 at) const{
		return Frame::collides(at);
	}

protected:
	bool privateOnWheel(ivec2 at, int delta){
		scrollingPosition += delta * 10;
		if (scrollingPosition > 0.0f) scrollingPosition = 0.0f;
		//if (scrollingPosition < contentHeight - size.y) scrollingPosition = contentHeight - size.y;
		return true;
	}

	float scrollingPosition;
	int contentHeight;
};

#endif