#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H

#include <cstring>
#include <cassert>
#include <cstdio>

#include <GL/glew.h>

#include "texture.h"
#include "vertex_array_object.h"

extern const uint8_t fontBitmapData[];

class TextRender{
public:
	//TODO: line wrapping
	static void prepare(std::vector<uiVert>& cache, ivec2& size, const char* text, float maxWidth = 0.0f){
		assert(fontTexture);
		assert(text);

		cache.clear();
		int len = strlen(text);

		vec2 pen(0.0f, 0.0f);
		for (int i = 0; i < len; i++){
			uint8_t c = (uint8_t) text[i];
			int w = c & 0xF;
			int h = c >> 4;

			uiVert triangles[6];

			triangles[0].pos.set(pen.x,                    pen.y);
			triangles[0].tex.set((w+0) / 16.0f, (h+0) / 16.0f);

			triangles[1].pos.set(pen.x + bitmapGlyphWidth, pen.y);
			triangles[1].tex.set((w+1) / 16.0f, (h+0) / 16.0f);

			triangles[2].pos.set(pen.x + bitmapGlyphWidth, pen.y + bitmapGlyphHeight);
			triangles[2].tex.set((w+1) / 16.0f, (h+1) / 16.0f);

			triangles[3].pos = triangles[0].pos;
			triangles[3].tex = triangles[0].tex;

			triangles[4].pos = triangles[2].pos;
			triangles[4].tex = triangles[2].tex;

			triangles[5].pos.set(pen.x,                    pen.y + bitmapGlyphHeight);
			triangles[5].tex.set((w+0) / 16.0f, (h+1) / 16.0f);

			pen.x += bitmapGlyphWidth;

			cache.insert(cache.end(), triangles, triangles + 6);
		}

		size.set(pen.x, bitmapGlyphHeight);
	}

	static Texture* getFontTexture(){
		return fontTexture;
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

	//terrible hack, but no way to avoid it until fonts are in
	static int getReasonableHeight(){
		return bitmapGlyphHeight;
	}

	static void init(){
		//font texture
		fontTexture = new Texture(1, GL_UNSIGNED_BYTE, bitmapWidth, bitmapHeight, (void*)fontBitmapData, GL_NEAREST, GL_NEAREST);
		printf("created font texture\n");
	}

	static void end(){
		delete fontTexture;
	}
	
private:
	static Texture* fontTexture;
	
	static const int bitmapWidth = 144;
	static const int bitmapHeight = 256;

	static const int bitmapGlyphWidth = 9;
	static const int bitmapGlyphHeight = 16;
};

#endif //TEXT_RENDER_H

