#ifndef UI_ELEM_H
#define UI_ELEM_H

#include <cstdint>
#include <cstdio>

#include <list>

#include <GL/glew.h>

#include "../FastDelegate.h"
#include "../maths/vec2.h"

extern vec2i windowResolution; //app.cpp

class UIElem{
public:
	#define UI_ELEM_CORNER_MARGIN 15

	//flags
	#define UI_STICK_TOP_LEFT     (1 << 0)
	#define UI_STICK_TOP_RIGHT    (1 << 1)
	#define UI_STICK_BOTTOM_LEFT  (1 << 2)
	#define UI_STICK_BOTTOM_RIGHT (1 << 3)
	#define UI_STICK_MASK         (UI_STICK_TOP_LEFT | UI_STICK_TOP_RIGHT | UI_STICK_BOTTOM_LEFT | UI_STICK_BOTTOM_RIGHT)

	#define UI_ACTIVE             (1 << 4)
	#define UI_AUTO_RESIZE        (1 << 5)

	UIElem() {
		parent = nullptr;
		r = g = b = a = 255;
		flags = UI_ACTIVE;
	}

	/*UIElem(vec2i pos) {
		this->pos = pos;
		parent = nullptr;
		r = g = b = a = 255;
		active = true;
		flags = UI_ACTIVE;
	}*/

	virtual ~UIElem() {
		if (parent){
			parent->notifyChildDeleted(this);
			parent = nullptr;
		} 
		for (auto child : children){
			child->setParent(nullptr);
			delete child;
		}

		//do this after the child deletion loop so that the pointers keep going up the parent links
		if (this == hover){
			hover = getParent();
		}
		if (this == focus){
			focus = getParent();
		}
	}

	//try not to use this
	void setParent(UIElem* elem){
		parent = elem;
	}

	UIElem* getParent(){
		return parent;
	}

	uint32_t getFlags(){
		return flags;
	}

	void setFlag(uint32_t flag){
		flags |= flag;
	}

	void clearFlag(uint32_t flag){
		flags = flags & ~flag;
	}

	void clearAllFlags(){
		flags = 0;
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

    //0xRRGGBBAA
    void setColor(uint32_t hex){
    	r = (hex >> 24) & 0xFF;
    	g = (hex >> 16) & 0xFF;
    	b = (hex >>  8) & 0xFF;
    	a = (hex >>  0) & 0xFF;
    }

    void setColorBytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255){
    	this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
    }

	virtual void update(){
		for (auto child : children){
			if (child->getFlags() & UI_ACTIVE){
				child->update();
				vec2i cs = child->getSize();

				uint8_t cf = child->getFlags();
				if (cf & UI_STICK_MASK){
					//stick to corner, with a small margin
					if (cf & UI_STICK_TOP_LEFT){
						child->setPosition(vec2i(UI_ELEM_CORNER_MARGIN, UI_ELEM_CORNER_MARGIN));
					}

					else if(cf & UI_STICK_TOP_RIGHT){
						child->setPosition(vec2i(windowResolution.x - cs.x - UI_ELEM_CORNER_MARGIN, UI_ELEM_CORNER_MARGIN));
					}

					else if(cf & UI_STICK_BOTTOM_RIGHT){
						child->setPosition(windowResolution - cs - vec2i(UI_ELEM_CORNER_MARGIN));
					}

					else{ //UI_STICK_BOTTOM_LEFT
						child->setPosition(vec2i(UI_ELEM_CORNER_MARGIN, windowResolution.y - cs.y - UI_ELEM_CORNER_MARGIN));
					}
				}
			}
		}
	}

	virtual void draw(int depth){
		for (auto child : children){
			if (child->getFlags() & UI_ACTIVE){
				child->draw(depth + 5);
			}
		}
	}

	//searches for a child element that collides with the mouse pointer, but does no action
	virtual UIElem* collides(vec2i at) const{
		for (auto child : children){
			if (child->getFlags() & UI_ACTIVE){
				UIElem* fromChild = child->collides(at);
				if (fromChild) return fromChild;
			}
		}

		return nullptr; //UIElem has no content in itself
	}

	void onMouseDown(vec2i at, int button) {
		privateOnMouseDown(at, button);
		if (onMouseDownCallback){
			onMouseDownCallback(this, at, button);
		}
	}
	void onMouseUp(vec2i at, int button) {
		privateOnMouseUp(at, button);
		if (onMouseUpCallback){
			onMouseUpCallback(this, at, button);
		}
	}

	void onMouseMove(vec2i at) {
		privateOnMouseHover(at);
		if (onMouseMoveCallback){
			onMouseMoveCallback(this, at);
		}
	}
	void onMouseDrag(vec2i to, vec2i base, int button) {
		privateOnMouseDrag(to, base, button);
		if (onMouseDragCallback){
			onMouseDragCallback(this, to, base, button);
		}
	}

	void onControlKey(int key){
		privateOnControlKey(key);
		if (onControlKeyCallback){
			onControlKeyCallback(this, key);
		}
	}
	void onTextKey(int key, int modifiers){
		privateOnTextKey(key, modifiers);
		if (onTextKeyCallback){
			onTextKeyCallback(this, key, modifiers);
		}
	}

	void onWheel(vec2i at, int delta) {
		if (!privateOnWheel(at, delta)){
			//pass the message along to the parent is one exists
			if (parent){
				parent->onWheel(at, delta);
			}
		}
		else{
			if (onMouseWheelCallback){
				onMouseWheelCallback(this, at, delta);
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


	template <class X>
	inline void bindMouseDown(X* instance, void(X::*function)(UIElem* context, vec2i at, int button)) {
		onMouseDownCallback.bind(instance, function);
	}

	template <class X>
	inline void bindMouseUp(X* instance, void(X::*function)(UIElem* context, vec2i at, int button)) {
		onMouseUpCallback.bind(instance, function);
	}


	template <class X>
	inline void bindMouseMove(X* instance, void(X::*function)(UIElem* context, vec2i to)) {
		onMouseMoveCallback.bind(instance, function);
	}

	template <class X>
	inline void bindMouseDrag(X* instance, void(X::*function)(UIElem* context, vec2i from, vec2i to, int button)) {
		onMouseDragCallback.bind(instance, function);
	}


	template <class X>
	inline void bindControlKey(X* instance, void(X::*function)(UIElem* context, int key)) {
		onControlKeyCallback.bind(instance, function);
	}

	template <class X>
	inline void bindTextKey(X* instance, void(X::*function)(UIElem* context, int key, int modifiers)) {
		onTextKeyCallback.bind(instance, function);
	}


	template <class X>
	inline void bindMouseWheel(X* instance, void(X::*function)(UIElem* context, vec2i at, int delta)) {
		onMouseWheelCallback.bind(instance, function);
	}


	//KEEP THESE VALID
	static UIElem* focus; //last clicked element
	static UIElem* hover; //topmost element under mouse pointer

protected:
	//derive these for generic class behaviour, like counter value changing
	virtual void privateOnMouseDown(vec2i at, int button) { }
	virtual void privateOnMouseUp(vec2i at, int button) { }

	virtual void privateOnMouseHover(vec2i at) { }
	virtual void privateOnMouseDrag(vec2i to, vec2i base, int button) { }

	//called only if focus is on the element and a key is pressed
	virtual void privateOnControlKey(int key){ } //arrows, ctrl, shift, alt, Fs, pageup, pagedown, insert, home, end (but not mod4, strangely)
	virtual void privateOnTextKey(int key, int modifiers){ } //backspace, delete, space, a-z, A-Z, 0-9 and so on

	//does the acutal onWheel action, 
	//returns whether to stop the event from propagating down to the next parent
	virtual bool privateOnWheel(vec2i at, int delta){
		return false;
	}

	//called by the destructor against its parent
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

	//color
	union{
		struct{
			uint8_t r, g, b, a;
		};
		uint8_t col[4];
	};

	//layout
	vec2i pos;
	vec2i size;

	//logic
	uint32_t flags;
	std::list<UIElem*> children;
	UIElem* parent;

	//callbacks
	fastdelegate::FastDelegate3<UIElem*, vec2i, int> onMouseDownCallback;
	fastdelegate::FastDelegate3<UIElem*, vec2i, int> onMouseUpCallback;

	fastdelegate::FastDelegate2<UIElem*, vec2i> onMouseMoveCallback;
	fastdelegate::FastDelegate4<UIElem*, vec2i, vec2i, int> onMouseDragCallback;

	fastdelegate::FastDelegate2<UIElem*, int> onControlKeyCallback;
	fastdelegate::FastDelegate3<UIElem*, int, int> onTextKeyCallback;

	fastdelegate::FastDelegate3<UIElem*, vec2i, int> onMouseWheelCallback;
};

#endif
