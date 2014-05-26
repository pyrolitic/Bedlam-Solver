#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdlib>
#include <cassert>

#include <GL/glew.h>

#include "gl_error.h"

class Texture{
public:
	Texture(){
		glGenTextures(1, &id);
		internal = format = type = min = mag = w = h = 0;
	}

	Texture(GLenum internalFormat, GLenum sourceFormat, GLenum sourceType, int width, int height, void* data = NULL, GLenum minMode = GL_NEAREST, GLenum magMode = GL_NEAREST, float anisotropyLevel = 1.0f){
		glGenTextures(1, &id);
		updateData(internalFormat, sourceFormat, sourceType, width, height, data);
		updateAccessMode(minMode, magMode, anisotropyLevel);

		CHECK_GL_ERROR
	}
	
	~Texture(){
		glDeleteTextures(1, &id);
	}
	
	void updateAccessMode(GLenum minMode, GLenum magMode, float anisotropyLevel){
		min = minMode;
		mag = magMode;
		
		bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropyLevel);

		CHECK_GL_ERROR
	}
	
	void updateData(GLenum internalFormat, GLenum sourceFormat, GLenum sourceType, int width, int height, void* data){
		assert(width >= 64 and height >= 64); //gl spec requires this
	
		internal = internalFormat;
		format = sourceFormat;
		type = sourceType;
		
		w = width;
		h = height;
	
		bind();
		//glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, sourceFormat, sourceType, data);
		gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, w, h, sourceFormat, sourceType, data);
		GLerror::list("texture_update");
	}
	
	void updateSubData(int dataWidth, int dataHeight, int offsetX, int offsetY, void* data){
		assert (data);
		assert (offsetX >= 0 and offsetY >= 0);
		assert (offsetX + dataWidth <= w and offsetY + dataHeight <= h);
		
		bind();
		glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, dataWidth, dataHeight, format, type, data);
		GLerror::list("texutre_sub_update");
	}
	
	void bind(){
		//avoid unnecessary texture binding
		//if (id != boundTexture){
			boundTexture = id;
			glBindTexture(GL_TEXTURE_2D, id);
		//}
		GLerror::list("texure_bind");
	}
	
	static void unBind(){
		//if (boundTexture != 0){
			boundTexture = 0;
			glBindTexture(GL_TEXTURE_2D, boundTexture);
		//}
	}
	
	GLuint getId(){
		return id;
	}
	
	int getWidth(){
		return w;
	}
	
	int getHeight(){
		return h;
	}
	
private:
	static GLuint boundTexture; //= 0 for no texture

	GLuint id;
	GLenum internal; //internal format
	GLenum format; //source format
	GLenum type; //source type
	
	//access modes
	GLenum min;
	GLenum mag;
	
	int w;
	int h;
};

#endif //TEXTURE_H
