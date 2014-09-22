#ifndef GL_VERTEX_ARRAY_H
#define GL_VERTEX_ARRAY_H

#include <cstddef> //offsetof

#include <GL/glew.h>

#include "../maths/vec.h"

//VAO backported to opengl 2
//surprisingly easy. the only downside is that attribute metadata must be set(setAttribPointers) on every bind

template<class T>
class VertexArrayObject {
	public:
		VertexArrayObject() {
			glGenBuffers(1, &vertexBuffer);
			verticesAssigned = 0;
		}

		~VertexArrayObject() {
			glDeleteBuffers(1, &vertexBuffer);
		}

		VertexArrayObject(VertexArrayObject& rhs); //unimplemented, error
		void operator =(VertexArrayObject& rhs); //same

		int getVerticesAssigned(){
			return verticesAssigned;
		}

		void bind() {
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			setAttribPointers();
		}

		static void bindDefault() {
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		/*must be defined for every T as follows
		for every field to be passed to opengl in T, do:
		1) glEnableVertexAttribArray(location); 
		  where location specifies the layout location

		2a) glVertexAttribPointer(location, numComponents, sourceType, normalize, stride, offsetof);
		 for all types, causing them to be converted to floats when accessed by the shader, 
		 optionally normalized to the [-1, 1] or [0, 1] range (for signed and unsigned types, respectively)

		or 2b) glVertexAttribIPointer(location, numComponents, sourceType, stride, offsetof);
		 for integer types, causing them to be accessed as integers (int, ivec2, and so on)

		  numComponents states how many components there in the vector (1 to 4), and will be accessed as .x through .w

		  sourceType is one of GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT for fixed types
		  and GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE - GL_BYTE for floating types

		  stride should be the size of the struct in bytes

		  offsetof should be the offset of the field from the beginning of the struct (use the offsetof macro from stddef.h)

		optionally 3) glVertexAttribDivisor(location, div);
		  where div is the number of vertices to get the same data at location before moving to the next one
		  by default, it's 0, meaning every vertex gets one data
		*/
		void setAttribPointers(); 

		void assign(int elements, const T* data, GLenum use = GL_STATIC_DRAW) {
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

			glBufferData(GL_ARRAY_BUFFER, sizeof(T) * elements, data, use);
			verticesAssigned = elements;

			bindDefault();
		}

	private:
		GLuint vertexBuffer;
		int verticesAssigned;
};

#endif
