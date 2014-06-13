#ifndef UI_ELEM_H
#define UI_ELEM_H

#include <cstdint>
#include <cstdio>

#include <list>

#include <GL/glew.h>

class UIElem{
public:
	UIElem() {
		px = py = 0;
		parent = nullptr;
		r = g = b = a = 1.0f;
	}
	virtual ~UIElem() {
		if (parent) parent->notifyChildDeleted(this);
		for (auto it = children.begin(); it != children.end(); it++){
			delete (*it);
		}
	}

	virtual void setPosition(int x, int y){
		px = x;
		py = y;
	}

	virtual void getPosition(int& x, int& y){
		x = px;
		y = py;
	}

	virtual void setSize(int w, int h){}
	virtual int getWidth(){ return 0; }
	virtual int getHeight(){ return 0; }

	void setColor(float r, float g, float b, float a = 1.0f){
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	virtual void update(int dt){}

	virtual void draw(){
		printf("UIElem::draw()");
		for (auto it = children.begin(); it != children.end(); it++){
			(*it)->draw();
		}
	}

	//searches for a child element that collides with the mouse pointer, but does no action
	virtual UIElem* collides(int x, int y){
		for (auto it = children.begin(); it != children.end(); it++){
			if ((*it)->collides(x - px, y - py)) return *it;
		}

		return nullptr; //UIElem has no content by itself
	}

	virtual void onLeftClick() {}
	virtual void onRightClick() {}
	virtual void onWheel(int delta) {}

	void addChild(UIElem* child){
		children.push_back(child);
	}

protected:
	float r, g, b, a;
	int px, py; //top-left corner
	std::list<UIElem*> children;
	UIElem* parent;

	void notifyChildDeleted(UIElem* child){
		for (auto it = children.begin(); it != children.end(); it++){
			if ((*it) == child){
				children.erase(it);
				return;
			}
		}

		printf("warning: UIElem notified of nonexiting child deletion\n");
	}
};

#endif
