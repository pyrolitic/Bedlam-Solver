#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H

#include <cstring>
#include <cassert>
#include <cstdio>

#include <GL/glew.h>

#include "texture.h"

extern const uint8_t fontBitmapData[];

class TextRender{
public:
	static void init(){
		//font texture
		tex = new Texture(GL_ALPHA8, GL_ALPHA, GL_UNSIGNED_BYTE, bitmapWidth, bitmapHeight, (void*)fontBitmapData, GL_NEAREST, GL_NEAREST);
		printf("created font texture\n");
	}

	//translate and scale to taste beforehand
	static void render(const char* text){
		assert (tex);
		assert (text);
		
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glActiveTexture(GL_TEXTURE0);
		tex->bind();
		
		glBegin(GL_QUADS);
	
		const uint8_t* c = (const uint8_t*) text;
		int x = 0;
		while (*c){
			int w = (*c & 0xF);
			int h = (*c >> 4);
		
			glMultiTexCoord2f(GL_TEXTURE0, (w+0) / 16.0f, (h+0) / 16.0f); glVertex2i(x,                                    0);
			glMultiTexCoord2f(GL_TEXTURE0, (w+1) / 16.0f, (h+0) / 16.0f); glVertex2i(x + bitmapGlyphWidth,                 0);
			glMultiTexCoord2f(GL_TEXTURE0, (w+1) / 16.0f, (h+1) / 16.0f); glVertex2i(x + bitmapGlyphWidth, bitmapGlyphHeight);
			glMultiTexCoord2f(GL_TEXTURE0, (w+0) / 16.0f, (h+1) / 16.0f); glVertex2i(x,                    bitmapGlyphHeight);
		
			x += bitmapGlyphWidth;
			c++;
		}
	
		glEnd();
		Texture::unBind();
		glDisable(GL_BLEND);
	}

	static void metrics(const char* text, int* w, int* h){
		assert (text);
		if (w) *w = bitmapGlyphWidth * strlen(text);
		if (h) *h = bitmapGlyphHeight;
	}
	
private:
	static Texture* tex;
	
	static const int bitmapWidth = 144;
	static const int bitmapHeight = 256;

	static const int bitmapGlyphWidth = 9;
	static const int bitmapGlyphHeight = 16;
};

#endif //TEXT_RENDER_H

