#ifndef GLERROR_H
#define GLERROR_H

#include <stdio.h>

class GLerror{
public:
	#define CHECK_GL_ERROR GLerror::list(__FILE__, __LINE__, __func__);

	static void list(const char* file, int line, const char* function){
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR){
			fprintf(stderr, "GL error from %s:%d(%s): %s\n", file, line, function, gluErrorString(err));
		}
	}

	static void list(const char* from){
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR){
			fprintf(stderr, "GL error from %s: %s\n", from, gluErrorString(err));
		}
	}
};

#endif
