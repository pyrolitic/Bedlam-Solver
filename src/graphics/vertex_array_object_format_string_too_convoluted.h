#ifndef GL_VERTEX_ARRAY_H
#define GL_VERTEX_ARRAY_H

#include <GL/glew.h>

struct _vert {
		float v[3];
		float n[3];
		float tc[2];
};

/*
Format string specification:

string: list of items, separated by zero or more spaces

item: [size of vector][unsigned][data type][presentation]

size of vector: 1 to 4 - that many consecutive elements, will be accessed as .x, .y, .z and .w in shader

unsigned: U - make fixed point data type unsigned, invalid if applied to a floating point type
          lack of makes it signed

data type: fixed point:   floating point:     
           B - byte(8)    H - half(16)
           S - short(16)  F - float(32)
           I - int(32)    D - double(64)

presentation: C - convert; present all types as float (don't normalize fixed types)
              N - normalize; present all types as float, normalizing signed fixed point types to [-1, 1] for unsigned to [0, 1]
              K - keep integers as integers and floats as floats

The alignment of the struct passed in must follow normal alignment rules, where 
*/

//===============================================================================
class VertexArrayObject {
	private:
		struct formatItem{
			int offset;
			int size;
			GLenum type;
			char presentation;
			formatItem(int offset, int size, GLenum type, char presentation): 
				offset(offset), size(size), type(type), presentation(presentation) {}
		};

		void setAttribPointers(){
			char errorPointer[64];
			bool errorDetected = false;
			#define SET_ATTRIB_POINTER_ERROR(at, msg) { \
				for (int i = 0; i < at - 1; i++) errorPointer[i] = ' '; \
				errorPointer[at-1] = '^'; \
				errorPointer[at] = '\0'; \
				fprintf(stderr, "error in VAO format string: %s\n%s\n%s\n", msg, format, errorPointer); \
				errorDetected = true; \
			}

			const int PARSE_VECTOR_SIZE = 0;
			const int PARSE_UNSIGNED = 1;
			const int PARSE_DATA_TYPE = 2;
			const int PARSE_PRESENTATION = 3;

			const unsigned int typeSizes[] = {
				[GL_BYTE - GL_BYTE] = 1,
				[GL_UNSIGNED_BYTE - GL_BYTE] = 1,

				[GL_SHORT - GL_BYTE] = 2,
				[GL_UNSIGNED_SHORT - GL_BYTE] = 2,

				[GL_INT - GL_BYTE] = 4,
				[GL_UNSIGNED_INT - GL_BYTE] = 4,

				[GL_HALF_FLOAT - GL_BYTE] = 2,
				[GL_FLOAT - GL_BYTE] = 4,
				[GL_DOUBLE - GL_BYTE] = 8,
			};

			int state = PARSE_VECTOR_SIZE;
			int vectorSize = 1;
			bool typeUnsigned = false;
			GLenum dataType;

			const char* c = format;
			int nextByte = 0; //computed running offset of the byte right after the last processed field

			std::list<formatItem> items;

			while(*c){
				switch(state){
				case PARSE_VECTOR_SIZE:
					while(*(c + 1) == ' ') c++; //skip space at the beginning

					if (*c >= '1' and *c <= '4'){
						vectorSize = (int)(*c - '0');
					}
					else{
						SET_ATTRIB_POINTER_ERROR(c - format, "invalid vector size");
					}
					state++;
					break;

				case PARSE_UNSIGNED:
					if (*c == 'U'){
						typeUnsigned = true;
					}
					state++;
					break;

				case PARSE_DATA_TYPE:
					if (typeUnsigned){
						if (*c == 'B') dataType = GL_UNSIGNED_BYTE;
						else if (*c == 'S') dataType = GL_UNSIGNED_SHORT;	
						else if (*c == 'I') dataType = GL_UNSIGNED_INT;
						else if (*c == 'H' or *c == 'F' or *c == 'D'){
							SET_ATTRIB_POINTER_ERROR(c - format, "unsigned flag applied to floating type");
						}
						else{
							SET_ATTRIB_POINTER_ERROR(c - format, "invalid type specifier");
						}
					}
					else{
						if (*c == 'B') dataType = GL_BYTE;
						else if (*c == 'S') dataType = GL_SHORT;
						else if (*c == 'I') dataType = GL_INT;
						else if (*c == 'H') dataType = GL_HALF_FLOAT;
						else if (*c == 'F') dataType = GL_FLOAT;
						else if (*c == 'D') dataType = GL_DOUBLE;
						else{
							SET_ATTRIB_POINTER_ERROR(c - format, "invalid type specifier");
						}
					}
					state++;
					break;

				case PARSE_PRESENTATION:
					if (*c == 'C' or *c == 'N' or *c == 'K') {
						int typeSize = typeSizes[dataType - GL_BYTE];

						if (typeSize > 1){
							//needs alignment
							int addrMask = typeSize - 1;
							if (nextByte & typeSize){
								//unaligned
								nextByte = (nextByte & ~addrMask) + typeSize;
							}
						}
						items.emplace_back(nextByte, vectorSize, dataType, *c);
						nextByte += typeSize * vectorSize;
					}
					else{
						SET_ATTRIB_POINTER_ERROR(c - format, "invalid presentation specifier");
					}
					state = PARSE_VECTOR_SIZE;

					while(*(c + 1) == ' ') c++; //skip space at the end
					break;
				}

				c++;
			}

			if (!errorDetected){
				int index = 0;
				int size = nextByte;

				for (auto& item : items){
					if (item.presentation == 'C'){
						glVertexAttribPointer(index, item.size, item.type, GL_FALSE, size, (void*) (item.offset));
					}
					else if(item.presentation == 'N'){
						glVertexAttribPointer(index, item.size, item.type, GL_TRUE, size, (void*) (item.offset));
					}
					else{
						if (item.type == GL_HALF_FLOAT or item.type == GL_FLOAT or item.type == GL_DOUBLE){
							glVertexAttribPointer(index, item.size, item.type, GL_FALSE, size, (void*) (item.offset));
						}
						else{
							glVertexAttribIPointer(index, item.size, item.type, size, (void*) (item.offset));
						}
					}

					index++;
				}
			}
		}

	public:
		VertexArrayObject(const char* formatString) {
			glGenBuffers(1, &vertexBuffer);
			glGenVertexArrays(1, &vertexArray);
			strncpy(format, formatString, sizeof(format));
		}

		~VertexArrayObject() {
			glDeleteBuffers(1, &vertexBuffer);
			glDeleteVertexArrays(1, &vertexArray);
		}

		VertexArrayObject(VertexArrayObject& rhs); //unimplemented, error
		void operator =(VertexArrayObject& rhs); //same

		void bind() {
			glBindVertexArray(vertexArray);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		}

		static void bindDefault() {
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		void assign(int elements, void* data) {
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(_vert) * elements, data, GL_STATIC_DRAW);

			glBindVertexArray(vertexArray);

			glEnableVertexAttribArray(attrib_position);
			glEnableVertexAttribArray(attrib_normal);
			glEnableVertexAttribArray(attrib_texCoord);
			glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof(_vert), (void*) (0 * sizeof(float)));
			glVertexAttribPointer(attrib_normal, 3, GL_FLOAT, GL_FALSE, sizeof(_vert), (void*) (3 * sizeof(float)));
			glVertexAttribPointer(attrib_texCoord, 2, GL_FLOAT, GL_FALSE, sizeof(_vert), (void*) (6 * sizeof(float)));
			glVertexAttribDivisor(attrib_position, 0);
			glVertexAttribDivisor(attrib_texCoord, 0);
			glVertexAttribDivisor(attrib_normal, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		GLuint vertexBuffer;
		GLuint vertexArray;
		char format[64];
};

#endif
