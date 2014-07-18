#ifndef LINEAR_CONTAINER_H
#define LINEAR_CONTAINER_H

#include <cstdint>
#include <cmath>

#include <string>

#include "vec2.h"
#include "ui_elem.h"

//A container of UIElems that lays out its children in a line, spacing them out,
//to eliminate the hassle of manual positioning
//Unline Frame and its ilk, it has no border and it is invisible
class LinearContainer : public UIElem{
public:
	#define LINEAR_CONTAINER_HORIZONTAL 0
	#define LINEAR_CONTAINER_VERTICAL 1
	#define LINEAR_CONTAINER_SEPARATION 15

	LinearContainer(vec2i pos, int direction = LINEAR_CONTAINER_HORIZONTAL) : UIElem(pos){
		orientation = direction;
	}

	virtual ~LinearContainer() {}

	void setSize(vec2i s){
		printf("attempting to resize a LinearContainer\n");
	}

	void update(){
		//reposition elements since they might have changed size
		size.set(0, 0);
		for (UIElem* child : children){
			vec2i cs = child->getSize();
			if (orientation == LINEAR_CONTAINER_HORIZONTAL){
				child->setPosition(pos + vec2i(size.x + LINEAR_CONTAINER_SEPARATION, 0));
				size.x += LINEAR_CONTAINER_SEPARATION + cs.x;
				if (cs.y > size.y) size.y = cs.y;
			}
			else{
				child->setPosition(pos + vec2i(0, size.y + LINEAR_CONTAINER_SEPARATION));
				size.y += LINEAR_CONTAINER_SEPARATION + cs.y;
				if (cs.x > size.x) size.x = cs.x;
			}
		}
	}

	void addChild(UIElem* child){
		assert(child->getParent() == nullptr); //no double custody

		child->setParent(this);
		children.emplace_back(child);
		vec2i cs = child->getSize();
		if (orientation == LINEAR_CONTAINER_HORIZONTAL){
			child->setPosition(pos + vec2i(size.x + LINEAR_CONTAINER_SEPARATION, 0));
			size.x += LINEAR_CONTAINER_SEPARATION + cs.x;
			if (cs.y > size.y) size.y = cs.y;
		}
		else{
			child->setPosition(pos + vec2i(0, size.y + LINEAR_CONTAINER_SEPARATION));
			size.y += LINEAR_CONTAINER_SEPARATION + cs.y;
			if (cs.x > size.x) size.x = cs.x;
		}
	}

private:
	int orientation;
};

#endif
