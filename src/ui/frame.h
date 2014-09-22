#ifndef FRAME_H
#define FRAME_H

#include <cstring>
#include <cstdint>
#include <cmath>

#include <string>

#include "../graphics/texture.h"
#include "../graphics/ui_render.h"

#include "../maths/vec.h"
#include "../maths/mat4.h"

#include "ui_elem.h"

class Frame : public UIElem{
public:
	#define FRAME_ROUNDED_RADIUS 7 //pixels

	Frame() : UIElem(){
	}

	virtual ~Frame() {}

	virtual void update(){
		//update all active children first
		for (auto child : children){
			if (child->getFlags() & UI_ACTIVE){
				child->update();
			}
		}

		//fit all contents
		if (!children.empty()){
			ivec2 largest = children.front()->getSize();

			for (auto elem : children){
				elem->setPosition(pos + ivec2(FRAME_ROUNDED_RADIUS));
				ivec2 elemSize = elem->getSize();
				if (elemSize > largest) largest = elemSize;
			}

			setSize(ivec2(FRAME_ROUNDED_RADIUS) * 2 + largest);
		}
	}

	virtual void draw(int depth){
		assert(roundedBox);

		//TODO: refactor below
		int px = pos.x;
		int py = pos.y;
		int width = size.x - 2 * FRAME_ROUNDED_RADIUS;
		int height = size.y - 2 * FRAME_ROUNDED_RADIUS;

		//top left corner
		verts[0].pos.set(px,                        py);
		verts[1].pos.set(px + FRAME_ROUNDED_RADIUS, py);
		verts[2].pos.set(px + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS);

		verts[3].pos.set(px,                        py);
		verts[4].pos.set(px + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS);
		verts[5].pos.set(px,                        py + FRAME_ROUNDED_RADIUS);

		//top right corner
		verts[6].pos.set(px + FRAME_ROUNDED_RADIUS + width,                        py);
		verts[7].pos.set(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py);
		verts[8].pos.set(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS);

		verts[9].pos.set(px + FRAME_ROUNDED_RADIUS + width,                        py);
		verts[10].pos.set(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS);
		verts[11].pos.set(px + FRAME_ROUNDED_RADIUS + width,                        py + FRAME_ROUNDED_RADIUS);

		//bottom right corner
		verts[12].pos.set(px + FRAME_ROUNDED_RADIUS + width,                        py + FRAME_ROUNDED_RADIUS + height);
		verts[13].pos.set(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height);
		verts[14].pos.set(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);
		
		verts[15].pos.set(px + FRAME_ROUNDED_RADIUS + width,                        py + FRAME_ROUNDED_RADIUS + height);
		verts[16].pos.set(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);
		verts[17].pos.set(px + FRAME_ROUNDED_RADIUS + width,                        py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);

		//bottom left corner
		verts[18].pos.set(px,                        py + FRAME_ROUNDED_RADIUS + height);
		verts[19].pos.set(px + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height);
		verts[20].pos.set(px + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);

		verts[21].pos.set(px,                        py + FRAME_ROUNDED_RADIUS + height);
		verts[22].pos.set(px + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);
		verts[23].pos.set(px,                        py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);

		//center, vertical
		verts[24].pos.set(px + FRAME_ROUNDED_RADIUS,         py);
		verts[25].pos.set(px + FRAME_ROUNDED_RADIUS + width, py);
		verts[26].pos.set(px + FRAME_ROUNDED_RADIUS + width, py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);

		verts[27].pos.set(px + FRAME_ROUNDED_RADIUS,         py);
		verts[28].pos.set(px + FRAME_ROUNDED_RADIUS + width, py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);
		verts[29].pos.set(px + FRAME_ROUNDED_RADIUS,         py + FRAME_ROUNDED_RADIUS + height + FRAME_ROUNDED_RADIUS);

		//center, horizontal
		verts[30].pos.set(px,                                                       py + FRAME_ROUNDED_RADIUS);
		verts[31].pos.set(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS);
		verts[32].pos.set(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height);

		verts[33].pos.set(px,                                                       py + FRAME_ROUNDED_RADIUS);
		verts[34].pos.set(px + FRAME_ROUNDED_RADIUS + width + FRAME_ROUNDED_RADIUS, py + FRAME_ROUNDED_RADIUS + height);
		verts[35].pos.set(px,                                                       py + FRAME_ROUNDED_RADIUS + height);

		uiRender->startEntity(UI_ENTITY_PANE, ivec2(), depth, col, roundedBox);
		uiRender->addVerts(36, verts);
		uiRender->endEntity();

		//draw all children on top
		UIElem::draw(depth);
	}

	virtual UIElem* collides(ivec2 at) const{
		UIElem* child = UIElem::collides(at);
		if (child){
			return child;
		}
		else if (at >= pos && at < pos + size){
			return (UIElem*)this; //TODO: rounded edges pass-through
		}
		else{
			return nullptr;
		}
	}

	virtual void addChild(UIElem* child){
		UIElem::addChild(child);
		child->setPosition(pos + child->getPosition() + ivec2(FRAME_ROUNDED_RADIUS, FRAME_ROUNDED_RADIUS)); //place inside $this
	}

public:
	static void init(){
		memset(verts, 0, 36 * sizeof(uiVert));
		//top left corner
		verts[0].tex.set(0.0f, 0.0f);
		verts[1].tex.set(0.5f, 0.0f);
		verts[2].tex.set(0.5f, 1.0f);
		verts[3].tex.set(0.0f, 0.0f);
		verts[4].tex.set(0.5f, 1.0f);
		verts[5].tex.set(0.0f, 1.0f);

		//top right corner
		verts[6].tex.set(0.5f, 0.0f);
		verts[7].tex.set(0.0f, 0.0f);
		verts[8].tex.set(0.0f, 1.0f);
		verts[9].tex.set(0.5f, 0.0f);
		verts[10].tex.set(0.0f, 1.0f);
		verts[11].tex.set(0.5f, 1.0f);

		//bottom right corner
		verts[12].tex.set(0.5f, 1.0f);
		verts[13].tex.set(0.0f, 1.0f);
		verts[14].tex.set(0.0f, 0.0f);
		verts[15].tex.set(0.5f, 1.0f);
		verts[16].tex.set(0.0f, 0.0f);
		verts[17].tex.set(0.5f, 0.0f);

		//bottom left corner
		verts[18].tex.set(0.0f, 1.0f);
		verts[19].tex.set(0.5f, 1.0f);
		verts[20].tex.set(0.5f, 0.0f);
		verts[21].tex.set(0.0f, 1.0f);
		verts[22].tex.set(0.5f, 0.0f);
		verts[23].tex.set(0.0f, 0.0f);

		//center1
		verts[24].tex.set(0.5f, 0.0f);
		verts[25].tex.set(1.0f, 0.0f);
		verts[26].tex.set(1.0f, 1.0f);
		verts[27].tex.set(0.5f, 0.0f);
		verts[28].tex.set(1.0f, 1.0f);
		verts[29].tex.set(0.5f, 1.0f);

		//center2
		verts[30].tex.set(0.5f, 0.0f);
		verts[31].tex.set(1.0f, 0.0f);
		verts[32].tex.set(1.0f, 1.0f);
		verts[33].tex.set(0.5f, 0.0f);
		verts[34].tex.set(1.0f, 1.0f);
		verts[35].tex.set(0.5f, 1.0f);

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
		uint8_t* buffer = (uint8_t*) malloc(w * h * 4);
		memset(buffer, 255, w * h * 4);

		//left half, a quarter-circle
		for (int j = 0; j < h; j++){
			float y = 1.0f - (float)j / h;
			for (int i = 0; i < w / 2; i++){
				float x = 1.0f - (float)i / (w / 2);

				float dist = sqrt(y * y + x * x);
				uint8_t val = (dist > 1.0f)? 0 : 255;

				buffer[(i + j * w) * 4 + 3] = val; //alpha only
			}
		}

		roundedBox = new Texture(4, GL_UNSIGNED_BYTE, w, h, buffer, GL_LINEAR, GL_LINEAR);
		free(buffer);

		printf("initialized Frame mesh and texture\n");
	}

	static void end(){
		delete roundedBox;
	}

protected:
	static uiVert verts[36];
	static Texture* roundedBox;
};

#endif
