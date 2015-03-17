#ifndef GL_VERTEX_ARRAY_H
#define GL_VERTEX_ARRAY_H

#include <cstddef> //offsetof

#include <GL/glew.h>

#include "../maths/vec.h"

//===============================================================================
template<class T>
class VertexArrayObject {
	public:
		VertexArrayObject() {
			glGenBuffers(1, &vertexBuffer);
			glGenVertexArrays(1, &vertexArray);
			verticesAssigned = 0;
			specified = false;
		}

		~VertexArrayObject() {
			glDeleteBuffers(1, &vertexBuffer);
			glDeleteVertexArrays(1, &vertexArray);
		}

		VertexArrayObject(VertexArrayObject& rhs); //unimplemented, error
		void operator =(VertexArrayObject& rhs); //same

		int getVerticesAssigned(){
			return verticesAssigned;
		}

		void bind() {
			glBindVertexArray(vertexArray);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

			if (!specified){
				setAttribPointers();
				specified = true;
			}
		}

		static void bindDefault() {
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
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

		void assign(int elements, const T* data) {
			bind();

			glBufferData(GL_ARRAY_BUFFER, sizeof(T) * elements, data, GL_STATIC_DRAW);
			verticesAssigned = elements;

			bindDefault();
		}

	private:
		GLuint vertexBuffer;
		GLuint vertexArray;
		int verticesAssigned;
		bool specified;
};

#endif
