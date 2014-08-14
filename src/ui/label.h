#ifndef LABEL_H
#define LABEL_H

#include <cstdint>

#include <string>
#include <vector>

#include "../graphics/text_render.h"
#include "../graphics/ui_render.h"

#include "ui_elem.h"

class Label : public UIElem{
public:
	#define BUTTON_MIN_TEXT_HEIGHT 10 //pixels

	Label(){
		//blank, no area
	}

	Label(const char* initialText) {
		setText(initialText);
	}

	virtual ~Label(){
	}

	UIElem* collides(vec2i at) const{
		//pass-through, needed for buttons
		return nullptr;
	}

	void update(){
		//do nothing
	}

	std::string getText(){
		return text; //copy not reference
	}

	void setText(const char* newText){
		text.assign(newText);
		TextRender::prepare(vertCache, size, text.c_str());
	}

	void draw(int depth){
		uint8_t textCol[4] = {0x11, 0x11, 0x11, 0xFF};
		uiRender->startEntity(UI_ENTITY_TEXT, pos, depth, textCol, TextRender::getFontTexture());
		uiRender->addVerts(vertCache.size(), vertCache.data());
		uiRender->endEntity();
	}

	void addChild(UIElem* child){
		printf("warning: trying to add child to a label\n");
	}

private:
	std::vector<uiVert> vertCache;
	std::string text;
};

#endif