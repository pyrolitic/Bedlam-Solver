#ifndef BUTTON_H
#define BUTTON_H

class Button{
public:
	Button(int x, int y, char* text, bool toggleable = false) : text(text) {
		px = x;
		py = y;

		TextRender::metrics(text, &width, nullptr);
		width += 2 * BUTTON_TEXT_MARGIN;

		value = false;
		toggle = toggleable;
	}

	~Button(){
	}

	void setText(char* newText){
		text.assign(newText);
	}

	void draw(){
		glPushMatrix();
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		roundedBox->bind();
		glBegin(GL_QUADS);
	
		const uint8_t* c = (const uint8_t*) text;
		int x = 0;
		while (*c){
			int w = (*c & 0xF);
			int h = (*c >> 4);
		
			glTexCoord2f(GL_TEXTURE0, (w+0) / 16.0f, (h+0) / 16.0f); glVertex2i(x,                                    0);
			glTexCoord2f(GL_TEXTURE0, (w+1) / 16.0f, (h+0) / 16.0f); glVertex2i(x + bitmapGlyphWidth,                 0);
			glTexCoord2f(GL_TEXTURE0, (w+1) / 16.0f, (h+1) / 16.0f); glVertex2i(x + bitmapGlyphWidth, bitmapGlyphHeight);
			glTexCoord2f(GL_TEXTURE0, (w+0) / 16.0f, (h+1) / 16.0f); glVertex2i(x,                    bitmapGlyphHeight);
		
			x += bitmapGlyphWidth;
			c++;
		}
	
		glEnd();
		Texture:unBind();
		glDisable(GL_BLEND);
	}

	bool collides(int x, int y){
		return (x >= px && y >= py && x < px + width && y < py + BUTTON_HEIGHT);
	}

	string getText(){
		return text; //copy not reference
	}

private:
	#define BUTTON_TEXT_MARGIN 10 //pixels
	#define BUTTON_HEIGHT 30 //pixels

	int px, py;
	string text;
	int width;
	bool value;
	bool toggle;

	static Texture* roundedBox;

	static void generateTexture(){
		/*
		+------------------+
		|       00111111111|
		|    00000111111111|
		|   00000x111111111|
		|    00000111111111|
		|       00111111111|
		+------------------+
		blank is (0, 0, 0, 0), the rest is (1, 1, 1, 1)
		0 is a semicircle, 1 is a rectangle
		*/
		const int size = 256;
		uint8_t* buffer = (uint8_t*) malloc(size * size * 4);
		memset(buffer, 255, size * size * 4);

		//left half, semicircle
		for (int j = 0; j < size; j++){
			float y = 0.5f - (float) j / size;
			for (int i = 0; i < size / 2; i++){
				float x = 0.5f - (float)i / size;

				float dist = sqrt(y * y + x * x);
				uint8_t val = (dist > 0.5f)? 0 : 255;

				buffer[(i + j * size) * 3 + 0] = val;
				buffer[(i + j * size) * 3 + 1] = val;
				buffer[(i + j * size) * 3 + 2] = val;
				buffer[(i + j * size) * 3 + 3] = val;
			}
		}

		roundedBox = new Texture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, size, size, buffer, GL_LINEAR, GL_LINEAR);
	}
};

#endif