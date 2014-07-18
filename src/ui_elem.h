#ifndef UI_ELEM_H
#define UI_ELEM_H

#include <cstdint>
#include <cstdio>

#include <list>

#include <GL/glew.h>

#include "vec2.h"

class UIElem{
public:
	UIElem(int x, int y) {
		this->pos = vec2i(x, y);
		parent = nullptr;
		r = g = b = a = 1.0f;
	}

	UIElem(vec2i pos) {
		this->pos = pos;
		parent = nullptr;
		r = g = b = a = 1.0f;
	}

	virtual ~UIElem() {
		if (parent){
			parent->notifyChildDeleted(this);
			parent = nullptr;
		} 
		for (auto child : children){
			child->setParent(nullptr);
			delete child;
		}
	}

	void setParent(UIElem* elem){
		parent = elem;
	}

	UIElem* getParent(){
		return parent;
	}

	virtual void setPosition(vec2i p){
		for (auto child : children){
			child->setPosition(child->pos - pos + p);
		}
		pos = p;
	}

	virtual vec2i getPosition(){
		return pos;
	}

	virtual void setSize(vec2i s){
		size = s;
	}
    virtual vec2i getSize() { 
    	return size; 
    }

    void setColorHex(uint32_t hex){
    	if(hex < 0x1000000){
    		hex = (hex << 8) | 0xFF; //no alpha specified -> full alpha
    	}
    	r = ((hex >> 24) & 0xFF) / 255.0f;
    	g = ((hex >> 16) & 0xFF) / 255.0f;
    	b = ((hex >>  8) & 0xFF) / 255.0f;
    	a = ((hex >>  0) & 0xFF) / 255.0f;
    }

    void setColorBytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255){
    	this->r = r / 255.0f;
		this->g = g / 255.0f;
		this->b = b / 255.0f;
		this->a = a / 255.0f;
    }

	void setColor(float r, float g, float b, float a = 1.0f){
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	virtual void update(){}

	virtual void draw(){
		for (auto child : children){
			child->draw();
		}
	}

	//searches for a child element that collides with the mouse pointer, but does no action
	virtual UIElem* collides(vec2i at) const{
		for (auto child : children){
			UIElem* fromChild = child->collides(at);
			if (fromChild) return fromChild;
		}

		return nullptr; //UIElem has no content in itself
	}

	virtual void onMouseDown(vec2i at, int button) { }
	virtual void onMouseUp(vec2i at, int button) { }

	virtual void onMouseHover(vec2i at) { }
	virtual void onMouseDrag(vec2i to, vec2i base, int button) { }

	//called only if focus is on the element and a key is pressed
	virtual void onControlKey(int key){ } //arrows, ctrl, shift, alt, Fs, pageup, pagedown, insert, home, end (but not mod4, strangely)
	virtual void onTextKey(int key, int modifiers){ } //backspace, delete, space, a-z, A-Z, 0-9 and so on

	void onWheel(vec2i at, int delta) {
		printf("UIElem::on wheel, %d\n", delta);
		if (!privateOnWheel(at, delta)){
			//pass the message along to the parent is one exists
			if (parent){
				parent->onWheel(at, delta);
			}
		}
	}

	virtual void addChild(UIElem* child){
		child->setParent(this);
		children.emplace_back(child);
		child->setPosition(pos + child->getPosition()); //place inside $this
	}

	//adds $child above $before if $before exists,
	//otherwise adds at the end
	virtual void addChildBefore(UIElem* child, UIElem* before){
		child->setParent(this);
		for (auto it = children.begin(); it != children.end(); it++){
			if (*it == before){
				children.insert(it, child);
				return;
			}
		}
		children.emplace_back(child); //$before doesn't exist
	}

	bool hasChild(UIElem* thing){
		for (auto child : children){
			if (child == thing){
				return true;
			}
		}
		return false;
	}

	const std::list<UIElem*>& getChildren() const{
		return children;
	}

	static UIElem* focus; //last clicked element

protected:
	//does the acutal onWheel action, 
	//returns whether to stop the event from propagating down to the next parent
	virtual bool privateOnWheel(vec2i at, int delta){
		printf("UIElem::privateOnwheel\n");
		return false;
	}

	void notifyChildDeleted(UIElem* child){
		auto childIt = children.end();
		for (auto it = children.begin(); it != children.end(); it++){
			if (*it == child){
				childIt = it;
				break;
			}
		}

		if (childIt != children.end()){
			children.erase(childIt);
			printf("removed child that says it's being deleted, 0x%lx\n", (size_t)child);
		}
		else{
			printf("warning: UIElem notified of nonexiting child deletion\n");
		}
	}

	float r, g, b, a;
	vec2i pos;
	vec2i size;
	std::list<UIElem*> children;
	UIElem* parent;
};

#endif
