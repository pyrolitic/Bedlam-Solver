#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

#include <cstdint>

#include <string>

#ifndef TEST_BUILD //remove all references to gl when doing unit tests
#include <GL/glew.h>

#include "text_render.h"
#endif
#include "frame.h"

class TextInput : public Frame{
public:
	TextInput(vec2i pos, int width, const char* defaultText = "") : Frame(pos, vec2i(0, 0)), defaultText(defaultText) {
		Frame::setSize(vec2i(width + 2 * FRAME_ROUNDED_RADIUS, 2 * FRAME_ROUNDED_RADIUS + TextRender::getReasonableHeight()));
		caretIndex = -1; //end
		caretPos = pos.x + FRAME_ROUNDED_RADIUS;
	}

	virtual ~TextInput(){
		if (UIElem::focus == this){
			UIElem::focus = nullptr;
		}
	}

	void setDefaultText(const char* text){
		defaultText.assign(text);
	}

	std::string getDefaultText(){
		return defaultText;
	}

	void clearEntry(){
		text.assign("");
	}

	std::string getText(){
		return text; //copy not reference
	}

	void onControlKey(int key){
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

	void onTextKey(int key, int modifiers){
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

	void onMouseDown(vec2i at, int button){
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

#ifndef TEST_BUILD //remove all references to gl when doing unit tests
	void setText(const char* newText){
		text.assign(newText);
		int textWidth, textHeight;
		TextRender::metrics(text.c_str(), &textWidth, &textHeight);
		Frame::setSize(vec2i(textWidth + 2 * FRAME_ROUNDED_RADIUS, 2 * FRAME_ROUNDED_RADIUS + std::max(BUTTON_MIN_TEXT_HEIGHT, textHeight)));
	}

	void draw(){
		Frame::draw(); //draw frame underneath

		//render text
		vec2i renderAt = pos + vec2i(FRAME_ROUNDED_RADIUS, FRAME_ROUNDED_RADIUS); //overlay text, centered
		if (text.size() == 0 and UIElem::focus != this){
			glColor4f(0.2f, 0.3f, 0.3f, 1.0f);
			TextRender::render(defaultText.c_str(), renderAt.x, renderAt.y);
		}
		else{
			glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
			TextRender::render(text.c_str(), renderAt.x, renderAt.y);
		}

		if (UIElem::focus == this){
			//also render caret
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINES);
			glVertex2i(pos.x + FRAME_ROUNDED_RADIUS + caretPos, pos.y + FRAME_ROUNDED_RADIUS);
			glVertex2i(pos.x + FRAME_ROUNDED_RADIUS + caretPos, pos.y + size.y - FRAME_ROUNDED_RADIUS);
			glEnd();
		}
	}
#endif

	void addChild(UIElem* child){
		printf("warning: trying to add child to a button\n");
	}

private:
	std::string text;
	std::string defaultText;
	int caretIndex;
	int caretPos; //spatial offset from the beginning of text, in pixels
};

#endif