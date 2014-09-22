#ifndef LINEAR_CONTAINER_H
#define LINEAR_CONTAINER_H

#include <cstdint>
#include <cmath>

#include <string>

#include "../maths/vec.h"

#include "ui_elem.h"

//A container of UIElems that lays out its children in a line, spacing them out,
//to eliminate the hassle of manual positioning
//Unline Frame and its ilk, it has no border and it is invisible
class LinearContainer : public UIElem{
public:
	#define LINEAR_CONTAINER_HORIZONTAL 0
	#define LINEAR_CONTAINER_VERTICAL 1
	
	#define LINEAR_CONTAINER_SEPARATION 10

	LinearContainer(int direction = LINEAR_CONTAINER_HORIZONTAL) : UIElem(){
		orientation = direction;
		size.set(0, 0);
	}

	virtual ~LinearContainer() {}

	void setSize(ivec2 s){
		printf("attempting to resize a LinearContainer\n");
	}

	void update(){
		//update all active children first
		for (auto child : children){
			if (child->getFlags() & UI_ACTIVE){
				child->update();
			}
		}

		//reposition elements since they might have changed size
		size.set(0, 0);
		for (UIElem* child : children){
			if (child->getFlags() & UI_ACTIVE){
				ivec2 cs = child->getSize();
				int sep = LINEAR_CONTAINER_SEPARATION * (child != children.front()? 1 : 0);
				if (orientation == LINEAR_CONTAINER_HORIZONTAL){
					child->setPosition(pos + ivec2(size.x + sep, 0));
					size.x += sep + cs.x;
					if (cs.y > size.y) size.y = cs.y;
				}
				else{
					child->setPosition(pos + ivec2(0, size.y + sep));
					size.y += sep + cs.y;
					if (cs.x > size.x) size.x = cs.x;
				}
			}
		}
	}

	void addChild(UIElem* child){
		UIElem::addChild(child);

		if (child->getFlags() & UI_ACTIVE){
			ivec2 cs = child->getSize();
			int sep = LINEAR_CONTAINER_SEPARATION * (children.empty()? 0 : 1);
			if (orientation == LINEAR_CONTAINER_HORIZONTAL){
				child->setPosition(pos + ivec2(size.x + sep, 0));
				size.x += sep + cs.x;
				if (cs.y > size.y) size.y = cs.y;
			}
			else{
				child->setPosition(pos + ivec2(0, size.y + sep));
				size.y += sep + cs.y;
				if (cs.x > size.x) size.x = cs.x;
			}
		}
	}

private:
	int orientation;
};

#endif
