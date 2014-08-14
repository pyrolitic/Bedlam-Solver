#ifndef UI_RENDER
#define UI_RENDER

#include <cstddef>

#include "vertex_array_object.h"
#include "shader.h"

#include "../maths/vec2.h"
#include "../maths/vec3.h"
#include "../maths/mat4.h"

struct uiVert{
	vec2f pos;
	vec2f tex;

	static const int posId = 0;
	static const int texId = 1;
};

#define UI_ENTITY_PANE 1
#define UI_ENTITY_TEXT 0
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
		shaders[UI_ENTITY_PANE] = new Shader("shaders/ui");
		shaders[UI_ENTITY_TEXT] = new Shader("shaders/text");

		currentEntity = nullptr;
		currentEntityVerts = 0;
		currentEntityType = 0;
	}

	~UIRender(){
		for (int type = 0; type < UI_NUM_ENTITY_TYPES; type++){
			delete shaders[type];
		}
	}

	void windowResized(vec2i size){
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
			vaos[type].assign(vertices[type].size(), vertices[type].data());
			vaos[type].bind();

			glUniformMatrix4fv(shaders[type]->getUniformLocation("projection"), 1, GL_FALSE, (const GLfloat*)projection.data);
			glUniform1i(shaders[type]->getUniformLocation("texture"), 0);

			for (auto entity : entities[type]){
				//printf("drawing entity with %d triangles, with color %x%x%x%x\n", entity.count, entity.col[0], entity.col[1], entity.col[2], entity.col[3]);
				mat4 modelView = mat4::translation(entity.offset);
				glUniformMatrix4fv(shaders[type]->getUniformLocation("modelView"), 1, GL_FALSE, (const GLfloat*)modelView.data);
				glUniform4f(shaders[type]->getUniformLocation("entityColor"), entity.col[0] / 255.0f, entity.col[1] / 255.0f, entity.col[2] / 255.0f, entity.col[3] / 255.0f);
				entity.tex->bind();

				/*if (type == UI_ENTITY_TEXT){
					printf("o o%llx\n", &entity);
					for (int i = entity.start; i < entity.start + entity.count; i++){
						vec3f v = projection * (modelView * vec3f(vertices[type][i].pos, 0.0f));
						printf("v %f %f %f\n", v.x, v.y, v.z);
					}

					for (int i = 1; i < entity.count; i += 3){
						printf("f %d %d %d\n", i+0, i+1, i+2);
					}

					printf("\n\n");
				}*/

				glDrawArrays(GL_TRIANGLES, entity.start, entity.count);
				glutReportErrors();
			}

			glutReportErrors();
		}

		clear();
	}

	void clear(){
		for (int type = 0; type < UI_NUM_ENTITY_TYPES; type++){
			vertices[type].clear();
			entities[type].clear();
		}
	}

	void startEntity(int type, vec2i offset, int zOrder, uint8_t* col, Texture* tex){
		assert(!currentEntity);
		assert(type >= 0 and type < UI_NUM_ENTITY_TYPES);

		if (col[3] != 255){
			printf("alpha!\n");
		}

		entities[type].emplace_back(vertices[type].size(), vec3f(vec2f(offset), -(float)zOrder / 100), col, tex);
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
			printf("warning: added ui drawEntity with no vertices\n");
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
		assert(count > 0);
		vertices[currentEntityType].insert(vertices[currentEntityType].end(), array, array + count);
		currentEntityVerts += count;
	}

private:
	struct drawEntity{
		int start;
		int count;
		vec3f offset;
		uint8_t col[4];
		Texture* tex;

		drawEntity(int start, const vec3f& offset, uint8_t* col, Texture* tex) : start(start), count(0), offset(offset), tex(tex) {
			memcpy(this->col, col, 4);
		}
	};

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
