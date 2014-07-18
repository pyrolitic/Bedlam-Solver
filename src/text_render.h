#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H

#include <cstring>
#include <cassert>
#include <cstdio>

#include <GL/glew.h>

#include "texture.h"

extern const uint8_t fontBitmapData[];

//TODO: revamp to add atalses, support for fonts

class TextRender{
public:
	//translate and scale to taste beforehand
	static void render(const char* text, int xOffset, int yOffset){
		assert(tex);
		assert(text);

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
		
			glMultiTexCoord2f(GL_TEXTURE0, (w+0) / 16.0f, (h+0) / 16.0f); glVertex2i(xOffset + x,                    yOffset);
			glMultiTexCoord2f(GL_TEXTURE0, (w+1) / 16.0f, (h+0) / 16.0f); glVertex2i(xOffset + x + bitmapGlyphWidth, yOffset);
			glMultiTexCoord2f(GL_TEXTURE0, (w+1) / 16.0f, (h+1) / 16.0f); glVertex2i(xOffset + x + bitmapGlyphWidth, yOffset + bitmapGlyphHeight);
			glMultiTexCoord2f(GL_TEXTURE0, (w+0) / 16.0f, (h+1) / 16.0f); glVertex2i(xOffset + x,                    yOffset + bitmapGlyphHeight);
		
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

	static void metricsToChar(const char* text, const char* upTo, int* w, int *h){
		assert(text);
		assert(upTo >= text);
		assert(upTo - text <= strlen(text));
		int len = upTo - text;
		if (w) *w = len * bitmapGlyphWidth;
		if (h) *h = bitmapGlyphHeight;
	}

	//Returns the index of the character at $spatialOffset, or -1 if it's past the end of the text
	//If >= 0 is returned, $symbolBegin and $nextSymbolBegin are set, respectively, 
	//to the spatial start of the current character and the spatial start of the next character
	//Otherwise, $symbolBegin is set to the start of what would be the next character,
	//and $nextSymbolBegin is unaffected
	static int findSymbolAt(const char* text, int spatialOffset, int* symbolBegin, int* nextSymbolBegin){
		assert(text);
		if (spatialOffset < 0) spatialOffset = 0;
		int textLen = strlen(text) * bitmapGlyphWidth;

		if (spatialOffset >= textLen){
			if (symbolBegin) *symbolBegin = textLen;
			return -1;
		}

		int charId = spatialOffset / bitmapGlyphWidth;
		if (symbolBegin) *symbolBegin = charId * bitmapGlyphWidth;
		if (nextSymbolBegin) *nextSymbolBegin = (charId + 1) * bitmapGlyphWidth;
		return charId;
	}

	static void init(){
		//font texture
		tex = new Texture(GL_ALPHA8, GL_ALPHA, GL_UNSIGNED_BYTE, bitmapWidth, bitmapHeight, (void*)fontBitmapData, GL_NEAREST, GL_NEAREST);
		printf("created font texture\n");
	}

	//terrible hack, but no way to avoid it until fonts are in
	static int getReasonableHeight(){
		return bitmapGlyphHeight;
	}
	
private:
	static Texture* tex;
	
	static const int bitmapWidth = 144;
	static const int bitmapHeight = 256;

	static const int bitmapGlyphWidth = 9;
	static const int bitmapGlyphHeight = 16;
};

#endif //TEXT_RENDER_H

