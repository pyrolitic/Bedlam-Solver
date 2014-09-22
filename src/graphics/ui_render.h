#ifndef UI_RENDER
#define UI_RENDER

#include <cstddef>
#include <list>

#include "vertex_array_object.h"
#include "shader.h"

#include "../maths/vec.h"
#include "../maths/vec.h"
#include "../maths/mat4.h"

struct uiVert{
	vec2 pos;
	vec2 tex;

	static const int posId = 0;
	static const int texId = 1;
};

#define UI_ENTITY_PANE 0
#define UI_ENTITY_TEXT 1
#define UI_NUM_ENTITY_TYPES 2

template<>
inline void VertexArrayObject<uiVert>::setAttribPointers(){
	glEnableVertexAttribArray(uiVert::posId);
	glEnableVertexAttribArray(uiVert::texId);
	glVertexAttribPointer(uiVert::posId, 2, GL_FLOAT, GL_FALSE, sizeof(uiVert), (void*) (offsetof(uiVert, pos)));
	glVertexAttribPointer(uiVert::texId, 2, GL_FLOAT, GL_FALSE, sizeof(uiVert), (void*) (offsetof(uiVert, tex)));
	glVertexAttribDivisor(uiVert::posId, 0);
	glVertexAttribDivisor(uiVert::texId, 0);
}

class UIRender{
public:
	UIRender(){
		const Shader::attribLocationPair attribs[] = {
			{0, "pos"},
			{1, "tex"},
			{-1, nullptr}
		};

		shaders[UI_ENTITY_PANE] = new Shader("shaders/ui", attribs);
		shaders[UI_ENTITY_TEXT] = new Shader("shaders/text", attribs);

		currentEntity = nullptr;
		currentEntityVerts = 0;
		currentEntityType = 0;
	}

	~UIRender(){
		for (int type = 0; type < UI_NUM_ENTITY_TYPES; type++){
			delete shaders[type];
		}
	}

	void windowResized(ivec2 size){
		projection = mat4::orthographicProjection(0.0f, (float)size.x, (float)size.y, 0.0f);
	}

	void render(){
		glDisable(GL_DEPTH_TEST); //always cover what was drawn before
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int type = 0; type < UI_NUM_ENTITY_TYPES; type++){
			shaders[type]->use();
			glActiveTexture(GL_TEXTURE0);

			//printf("rendering a batch of %lu entities, %lu vertices\n", entities[type].size(), vertices[type].size());
			vaos[type].assign(vertices[type].size(), vertices[type].data(), GL_STREAM_DRAW);
			vaos[type].bind();

			glUniformMatrix4fv(shaders[type]->getUniformLocation("projection"), 1, GL_FALSE, (const GLfloat*)projection.data);
			glUniform1i(shaders[type]->getUniformLocation("texture"), 0);

			for (auto& entity : entities[type]){
				//printf("drawing entity with %d triangles, with color 0x%08x\n", entity.count, entity.col);
				mat4 modelView = mat4::translation(entity.offset);
				glUniformMatrix4fv(shaders[type]->getUniformLocation("modelView"), 1, GL_FALSE, (const GLfloat*)modelView.data);
				glUniform4f(shaders[type]->getUniformLocation("entityColor"), 
					((entity.col >> 24) & 0xFF) / 255.0f, 
					((entity.col >> 16) & 0xFF) / 255.0f, 
					((entity.col >>  8) & 0xFF) / 255.0f, 
					((entity.col >>  0) & 0xFF) / 255.0f);

				entity.tex->bind();

				if (entity.rectClipping){
					glEnable(GL_SCISSOR_TEST);
					glScissor(entity.clipStart.x, entity.clipStart.y, entity.clipEnd.x, entity.clipEnd.y);
				}

				else{
					glDisable(GL_SCISSOR_TEST);
				}

				glDrawArrays(GL_TRIANGLES, entity.start, entity.count);
				glutReportErrors();
			}

			glutReportErrors();
		}

		glDisable(GL_SCISSOR_TEST);
		clear();
	}

	void clear(){
		for (int type = 0; type < UI_NUM_ENTITY_TYPES; type++){
			vertices[type].clear();
			entities[type].clear();
		}
		if (!clipStack.empty()){
			fprintf(stderr, "warning: clip stack not empty after drawing\n");
			clipStack.clear();
		}
	}

	void startEntity(int type, ivec2 offset, int zOrder, uint32_t col, Texture* tex){
		assert(!currentEntity);
		assert(type >= 0 and type < UI_NUM_ENTITY_TYPES);

		ivec2 clipStart, clipEnd;
		bool rectClipping = !clipStack.empty();
		if (rectClipping){
			clipStart = clipStack.back().start;
			clipEnd = clipStack.back().end;
		}

		entities[type].emplace_back(vertices[type].size(), vec3(vec2(offset), -(float)zOrder / 100), col, tex, rectClipping, clipStart, clipEnd);
		currentEntity = &(entities[type].back());
		currentEntityType = type;
		currentEntityVerts = 0;
	}

	void endEntity(){
		assert(currentEntity);
		currentEntity->count = currentEntityVerts;
		currentEntity = nullptr;

		if (currentEntityVerts == 0){
			//remove it
			entities[currentEntityType].pop_back();
		}

		currentEntityVerts = 0;
	}

	void addVert(const uiVert& vert){
		assert(currentEntity);
		vertices[currentEntityType].emplace_back(vert);
		currentEntityVerts++;
	}

	void addVerts(int count, const uiVert* array){
		assert(currentEntity);
		if (count > 0){
			vertices[currentEntityType].insert(vertices[currentEntityType].end(), array, array + count);
			currentEntityVerts += count;
		}
	}

	void setClip(ivec2 start, ivec2 end){
		assert((start > end) or (start == end));
		assert(currentEntity);

		if (!clipStack.empty()){
			//logical AND intersect the current and new clip rectangles
			start = minVec(clipStack.back().end, maxVec(clipStack.back().start, start));
			end = maxVec(clipStack.back().start, minVec(clipStack.back().end, end));
		}
		clipStack.emplace_back(start, end);
	}

	void stopClip(){
		assert(!clipStack.empty());
		clipStack.pop_back();
	}

private:
	struct drawEntity{
		int start;
		int count;
		vec3 offset;
		uint32_t col;
		Texture* tex;

		bool rectClipping;
		ivec2 clipStart;
		ivec2 clipEnd;

		drawEntity(int start, const vec3& offset, uint32_t col, Texture* tex, bool clip, const ivec2& clipStart = vec2(), const ivec2& clipEnd = vec2()) :
			start(start), count(0), offset(offset), col(col), tex(tex), rectClipping(false), clipStart(clipStart), clipEnd(clipEnd) {}
	};

	struct clipFrame{
		ivec2 start;
		ivec2 end;
		clipFrame(const ivec2& clipStart, const ivec2& clipEnd) :
			start(clipStart), end(clipEnd) {}
	};

	std::list<clipFrame> clipStack;

	drawEntity* currentEntity;
	int currentEntityVerts;
	int currentEntityType;

	mat4 projection;
	std::vector<uiVert> vertices[UI_NUM_ENTITY_TYPES];
	std::vector<drawEntity> entities[UI_NUM_ENTITY_TYPES];
	VertexArrayObject<uiVert> vaos[UI_NUM_ENTITY_TYPES];

	Shader* shaders[UI_NUM_ENTITY_TYPES];
};

extern UIRender* uiRender;

#endif
