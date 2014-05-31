#ifndef FRAME_H
#define FRAME_H

#include <cstdint>
#include <cmath>

#include <string>

#include <GL/glew.h>

#include "ui_elem.h"
#include "text_render.h"

class Frame : public UIElem{
public:
	#define FRAME_ROUNDED_RADIUS 7 //pixels

	Frame(int x, int y, int w, int h){
		setPosition(x, y);
		setSize(w, h);
	}

	virtual ~Frame() {}

	virtual void setSize(int w, int h){
		width = w;
		height = h;
	}

	virtual int getWidth(){ 
		return width + 2 * FRAME_ROUNDED_RADIUS; 
	}

	virtual int getHeight(){
		return height + 2 * FRAME_ROUNDED_RADIUS;
	}

	virtual void draw(){//float r, float g, float b){
		assert(roundedBox);

		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		roundedBox->bind();
		glBegin(GL_QUADS);
		glColor4f(r, g, b, a);

		//top left corner
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f); glVertex2i(px,                        py);
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 0.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS, py);
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 1.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f); glVertex2i(px,                        py + FRAME_ROUNDED_RADIUS);

		//bottom left corner
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f); glVertex2i(px,                        py + FRAME_ROUNDED_RADIUS + height);
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 1.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height);
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 0.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f); glVertex2i(px,                        py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);

		//top right corner
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 0.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width,                        py);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 1.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width,                        py + FRAME_ROUNDED_RADIUS);

		//bottom right corner
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 1.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width,                        py + FRAME_ROUNDED_RADIUS + height);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 0.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width,                        py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);

		//center, vertical
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 0.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS        , py);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width, py);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width, py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 1.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS        , py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);

		//center, horizontal
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 0.0f); glVertex2i(px                                                      , py + FRAME_ROUNDED_RADIUS);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f); glVertex2i(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height);
		glMultiTexCoord2f(GL_TEXTURE0, 0.5f, 1.0f); glVertex2i(px                                                      , py + FRAME_ROUNDED_RADIUS + height);
	
		glEnd();
		Texture::unBind();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		UIElem::draw();
	}

	virtual UIElem* collides(int x, int y){
		UIElem* child = UIElem::collides(x, y);
		if (child) return child;
		else if (x >= px && y >= py && x < px + width + FRAME_ROUNDED_RADIUS * 2 && y < py + height + FRAME_ROUNDED_RADIUS * 2) return this; //TODO: rounded edges pass-through
		else return nullptr;
	}

	static void init(){
		/*
		+------------------+
		|        crrrrrrrrr|
		|    cccccrrrrrrrrr|
		| ccccccccrrrrrrrrr|
		|ccccccccxrrrrrrrrr|
		+------------------+
		c is circle, r is rectangle, x is the center of the circle
		' ' is 0.0, {c, r, x} is 1.0
		*/
		const int w = 256;
		const int h = 128;
		uint8_t* buffer = (uint8_t*) malloc(w * h);
		memset(buffer, 255, w * h);

		//left half, semicircle
		for (int j = 0; j < h; j++){
			float y = 1.0f - (float)j / h;
			for (int i = 0; i < w / 2; i++){
				float x = 1.0f - (float)i / (w / 2);

				float dist = sqrt(y * y + x * x);
				uint8_t val = (dist > 1.0f)? 0 : 255;

				buffer[i + j * w] = val;
			}
		}

		roundedBox = new Texture(GL_ALPHA8, GL_ALPHA, GL_UNSIGNED_BYTE, w, h, buffer, GL_LINEAR, GL_LINEAR);
	}

protected:
	int width, height;
	static Texture* roundedBox;
};

#endif