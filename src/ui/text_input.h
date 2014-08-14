#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

#include <cstdint>

#include <string>

#include "../graphics/text_render.h"

#include "frame.h"

extern Shader* textShader; //app.cpp

class TextInput : public Frame{
public:
	TextInput(int width, const char* defaultText = "") : Frame(){
		Frame::setSize(vec2i(width + 2 * FRAME_ROUNDED_RADIUS, 2 * FRAME_ROUNDED_RADIUS + TextRender::getReasonableHeight()));
		caretIndex = -1; //end
		caretPos = pos.x + FRAME_ROUNDED_RADIUS;

		setDefaultText(defaultText);
	}

	virtual ~TextInput(){
		if (UIElem::focus == this){
			UIElem::focus = nullptr;
		}
	}

	void update(){
		//do nothing
	}

	void setDefaultText(const char* text){
		defaultText.assign(text);
		vec2i s; //throw-away
		TextRender::prepare(defaultTextCache, s, defaultText.c_str());
	}

	std::string getDefaultText(){
		return defaultText;
	}

	std::string getText(){
		return text; //copy not reference
	}

	void setText(const char* newText){
		text.assign(newText);
		int textWidth, textHeight;
		vec2i textSize;
		TextRender::prepare(textCache, textSize, text.c_str());
		Frame::setSize(vec2i(textSize.x + 2 * FRAME_ROUNDED_RADIUS, 2 * FRAME_ROUNDED_RADIUS + std::max(BUTTON_MIN_TEXT_HEIGHT, textSize.y)));
	}

	void draw(int depth){
		Frame::draw(depth); //draw frame underneath

		vec2i renderAt = pos + vec2i(FRAME_ROUNDED_RADIUS); //overlay text, centered
		std::vector<uiVert>* cache;
		uint8_t textCol[4];

		if (text.size() == 0 and UIElem::focus != this){
			//render default text
			textCol[0] = 0x33;
			textCol[1] = 0x4C;
			textCol[2] = 0x4C;
			textCol[3] = 0xFF;
			cache = &defaultTextCache;
		}
		else{
			//render entered text
			textCol[0] = 0x19;
			textCol[1] = 0x19;
			textCol[2] = 0x19;
			textCol[3] = 0xFF;
			cache = &textCache;
		}

		uiRender->startEntity(UI_ENTITY_TEXT, renderAt, depth + 1, textCol, TextRender::getFontTexture());
		uiRender->addVerts(cache->size(), cache->data());
		uiRender->endEntity();

		if (UIElem::focus == this){
			//also render caret
			uint8_t caretCol[4];
			caretCol[0] = caretCol[1] = caretCol[2] = 30;
			caretCol[3] = (glutGet(GLUT_ELAPSED_TIME) % 1100 < 400) * 255;

			vec2i offset(pos.x + FRAME_ROUNDED_RADIUS + caretPos, pos.y + FRAME_ROUNDED_RADIUS);
			uiRender->startEntity(UI_ENTITY_TEXT, offset, depth + 1, textCol, TextRender::getFontTexture());
			uiRender->addVerts(caretVerts.size(), caretVerts.data());
			uiRender->endEntity();
		}
	}

	void addChild(UIElem* child){
		printf("warning: trying to add child to a button\n");
	}

protected:
	void privateOnControlKey(int key){
		printf("textinput::oncontrolkey %d\n", key);
		if (key == GLUT_KEY_LEFT){
			if (caretIndex > 1) caretIndex--;
		}
		else if (key == GLUT_KEY_RIGHT){
			if (caretIndex >= 0){
				caretIndex++;
				if (caretIndex >= text.size()){
					caretIndex = -1;
				}
			}
		}
		else if (key == GLUT_KEY_HOME){
			if (text.size() > 0){
				caretIndex = 0;
			}
		}
		else if (key == GLUT_KEY_END){
			caretIndex = -1;
		}

		//place the caret indicator in the right place
		if (caretIndex >= 0){
			TextRender::metricsToChar(text.c_str(), text.c_str() + caretIndex, &caretPos, nullptr);
		}
		else{
			TextRender::metrics(text.c_str(), &caretPos, nullptr);
		}
	}

	void privateOnTextKey(int key, int modifiers){
		if (key == '\b'){
			//backspace
			if (text.size() > 0){
				if (caretIndex == -1){
					text.erase(text.size() - 1, 1);
				}
				else if (caretIndex > 0){
					text.erase(caretIndex, 1);
					caretIndex--;
				}
			}
		}

		else if (key == 0x7F){
			//delete
			if (caretIndex >= 0){
				text.erase(caretIndex, 1);
				if (caretIndex == text.size()){
					caretIndex = -1;
				}
			}
		}

		else if (key >= ' ' and key <= '~'){
			//insert character
			if (caretIndex >= 0){
				text.insert(caretIndex, 1, key);
				caretIndex++;
				if (caretIndex == text.size()) caretIndex = -1;
			}
			else{
				text.insert(text.end(), key);
			}
		}

		//place the caret indicator in the right place
		if (caretIndex >= 0){
			TextRender::metricsToChar(text.c_str(), text.c_str() + caretIndex, &caretPos, nullptr);
		}
		else{
			TextRender::metrics(text.c_str(), &caretPos, nullptr);
		}
	}

	void privateOnMouseDown(vec2i at, int button){
		if (button == GLUT_LEFT_BUTTON){
			if (UIElem::focus == this){
				if (at >= pos + vec2i(FRAME_ROUNDED_RADIUS) and at < pos + size - vec2i(FRAME_ROUNDED_RADIUS)){
					//jump to symbol under mouse pointer
					int offset = at.x - (pos.x + FRAME_ROUNDED_RADIUS);
					int begin, end;

					caretIndex = TextRender::findSymbolAt(text.c_str(), offset, &begin, &end);
					if (caretIndex >= 0){
						int midway = (begin + end) / 2;
						if (offset <= midway){
							//at the current character
							caretPos = begin;
						}
						else{
							//at the next character
							caretIndex++;
							if (caretIndex == text.size()) caretIndex = -1;
							caretPos = end;
						}
					}
					else{
						caretPos = begin;
					}
				}
			}
			else{
				UIElem::focus = this;
				caretIndex = -1; //at the end
				TextRender::metrics(text.c_str(), &caretPos, nullptr);
			}
		}
	}

public:
	static void init(){
		vec2i s;
		TextRender::prepare(caretVerts, s, "|");
	}

	static void end(){
	}

private:
	std::string text;
	std::string defaultText;
	int caretIndex;
	int caretPos; //spatial offset from the beginning of text, in pixels

	std::vector<uiVert> defaultTextCache;
	std::vector<uiVert> textCache;
	
	static std::vector<uiVert> caretVerts;
	static Texture* caretTexture;
};

#endif