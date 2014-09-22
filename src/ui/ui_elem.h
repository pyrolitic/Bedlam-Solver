#ifndef UI_ELEM_H
#define UI_ELEM_H

#include <cstdint>
#include <cstdio>

#include <list>

#include <GL/glew.h>

#include "../FastDelegate.h"
#include "../maths/vec.h"

template <class T>
extern std::string demangledTypeName(const T* thing);

extern ivec2 windowResolution; //app.cpp

class UIElem{
public:
	static int elemsAlive;

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

	UIElem() : pos(0, 0), size(0, 0) {
		parent = nullptr;
		col = 0xFFFFFFFF;
		flags = UI_ACTIVE;

		elemsAlive++;

		//debug, trying to get rid of a valgrind unitialized memory report
		memset((uint8_t*)&onMouseDownCallback, 0, ((uint8_t*)&onMouseWheelCallback - (uint8_t*)&onMouseDownCallback) + sizeof(onMouseWheelCallback));
	}

	virtual ~UIElem() {
		//must duplicate the children array, since their 
		for (auto it = children.begin(); it != children.end();){
			UIElem* child = *it++; //the child will cause notifiChildDeleted on us, which will make us remove its reference from this very array, so increment before deleting
			delete child; 
		}

		if (this == hover){
			hover = getParent();
		}
		if (this == focus){
			focus = getParent();
		}

		if (parent){
			parent->notifyChildDeleted(this);
			parent = nullptr;
		}

		elemsAlive--;
	}

	//this shouldn't be public
	//void setParent(UIElem* elem){
	//	parent = elem;
	//}

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
		flags &= ~flag;
	}

	void clearAllFlags(){
		flags = 0;
	}

	virtual void setPosition(ivec2 p){
		for (auto child : children){
			child->setPosition(child->pos - pos + p);
		}
		pos = p;
	}

	virtual ivec2 getPosition(){
		return pos;
	}

	virtual void setSize(ivec2 s){
		size = s;
	}
    virtual ivec2 getSize() { 
    	return size; 
    }

    //0xRRGGBBAA
    void setColor(uint32_t hex){
    	col = hex;
    }

    void setColorBytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255){
    	col = (r << 24) | (g << 16) | (b << 8) | (a << 0);
    }

	virtual void update(){
		for (auto child : children){
			if (child->getFlags() & UI_ACTIVE){
				child->update();
				ivec2 cs = child->getSize();

				uint8_t cf = child->getFlags();
				if (cf & UI_STICK_MASK){
					//stick to corner, with a small margin
					if (cf & UI_STICK_TOP_LEFT){
						child->setPosition(ivec2(UI_ELEM_CORNER_MARGIN));
					}

					else if(cf & UI_STICK_TOP_RIGHT){
						child->setPosition(ivec2(windowResolution.x - cs.x - UI_ELEM_CORNER_MARGIN, UI_ELEM_CORNER_MARGIN));
					}

					else if(cf & UI_STICK_BOTTOM_RIGHT){
						child->setPosition(windowResolution - cs - ivec2(UI_ELEM_CORNER_MARGIN));
					}

					else{ //UI_STICK_BOTTOM_LEFT
						child->setPosition(ivec2(UI_ELEM_CORNER_MARGIN, windowResolution.y - cs.y - UI_ELEM_CORNER_MARGIN));
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
	virtual UIElem* collides(ivec2 at) const{
		for (auto child : children){
			if (child->getFlags() & UI_ACTIVE){
				UIElem* fromChild = child->collides(at);
				if (fromChild) return fromChild;
			}
		}

		return nullptr; //UIElem has no content in itself
	}

	void onMouseDown(ivec2 at, int button) {
		privateOnMouseDown(at, button);
		if (onMouseDownCallback){
			onMouseDownCallback(this, at, button);
		}
	}
	void onMouseUp(ivec2 at, int button) {
		privateOnMouseUp(at, button);
		if (onMouseUpCallback){
			onMouseUpCallback(this, at, button);
		}
	}

	void onMouseMove(ivec2 at) {
		privateOnMouseHover(at);
		if (onMouseMoveCallback){
			onMouseMoveCallback(this, at);
		}
	}
	void onMouseDrag(ivec2 to, ivec2 base, int button) {
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

	void onWheel(ivec2 at, int delta) {
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
		assert(child->getParent() == nullptr); //no double custody

		child->setParent(this);
		children.emplace_back(child);
		child->setPosition(pos + child->getPosition()); //maybe remove this
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

	void removeChild(UIElem* thing){
		auto childIt = children.end();
		for (auto it = children.begin(); it != children.end(); it++){
			if (*it == thing){
				childIt = it;
				break;
			}
		}

		if (childIt != children.end()){
			assert((*childIt)->getParent() == this);
			(*childIt)->setParent(nullptr);
			children.erase(childIt);
		}
		else{
			printf("warning: UIElem %lx asked to delete non-child element %lx\n", (size_t)this, (size_t)thing);
		}
	}

	const std::list<UIElem*>& getChildren() const{
		return children;
	}


	template <class X>
	inline void bindMouseDown(X* instance, void(X::*function)(UIElem* context, ivec2 at, int button)) {
		onMouseDownCallback.bind(instance, function);
	}

	template <class X>
	inline void bindMouseUp(X* instance, void(X::*function)(UIElem* context, ivec2 at, int button)) {
		onMouseUpCallback.bind(instance, function);
	}


	template <class X>
	inline void bindMouseMove(X* instance, void(X::*function)(UIElem* context, ivec2 to)) {
		onMouseMoveCallback.bind(instance, function);
	}

	template <class X>
	inline void bindMouseDrag(X* instance, void(X::*function)(UIElem* context, ivec2 from, ivec2 to, int button)) {
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
	inline void bindMouseWheel(X* instance, void(X::*function)(UIElem* context, ivec2 at, int delta)) {
		onMouseWheelCallback.bind(instance, function);
	}


	//KEEP THESE VALID
	static UIElem* focus; //last clicked element
	static UIElem* hover; //topmost element under mouse pointer

protected:
	//derive these for generic class behaviour, like counter value changing
	virtual void privateOnMouseDown(ivec2 at, int button) { }
	virtual void privateOnMouseUp(ivec2 at, int button) { }

	virtual void privateOnMouseHover(ivec2 at) { }
	virtual void privateOnMouseDrag(ivec2 to, ivec2 base, int button) { }

	//called only if focus is on the element and a key is pressed
	virtual void privateOnControlKey(int key){ } //arrows, ctrl, shift, alt, Fs, pageup, pagedown, insert, home, end (but not mod4, strangely)
	virtual void privateOnTextKey(int key, int modifiers){ } //backspace, delete, space, a-z, A-Z, 0-9 and so on

	//does the acutal onWheel action, 
	//returns whether to stop the event from propagating down to the next parent
	virtual bool privateOnWheel(ivec2 at, int delta){
		return false;
	}

	void setParent(UIElem* elem){
		parent = elem;
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

	//layout
	ivec2 pos;
	ivec2 size;

	//color
	uint32_t col;

	//logic
	uint32_t flags;
	std::list<UIElem*> children;
	UIElem* parent;

	//callbacks
	fastdelegate::FastDelegate3<UIElem*, ivec2, int> onMouseDownCallback;
	fastdelegate::FastDelegate3<UIElem*, ivec2, int> onMouseUpCallback;

	fastdelegate::FastDelegate2<UIElem*, ivec2> onMouseMoveCallback;
	fastdelegate::FastDelegate4<UIElem*, ivec2, ivec2, int> onMouseDragCallback;

	fastdelegate::FastDelegate2<UIElem*, int> onControlKeyCallback;
	fastdelegate::FastDelegate3<UIElem*, int, int> onTextKeyCallback;

	fastdelegate::FastDelegate3<UIElem*, ivec2, int> onMouseWheelCallback;
};

#endif
