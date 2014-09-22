#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdlib>
#include <cassert>

#include <GL/glew.h>

class Texture{
public:
	Texture(){
		glGenTextures(1, &id);
		channels = type = min = mag = w = h = 0;
	}

	Texture(int channels, GLenum sourceType, int width, int height, void* data = nullptr, GLenum minMode = GL_NEAREST, GLenum magMode = GL_NEAREST, float anisotropyLevel = 1.0f){
		glGenTextures(1, &id);
		updateData(channels, sourceType, width, height, data);
		updateAccessMode(minMode, magMode, anisotropyLevel);
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
	}
	
	void updateData(int channels, GLenum sourceType, int width, int height, void* data){
		assert(width >= 64 and height >= 64); //gl spec requires this
		assert(channels >= 1 and channels <= 4);
	
		GLenum format = channelsToFormat(channels);
		type = sourceType;
		
		w = width;
		h = height;
	
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, sourceType, data);
		//gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, w, h, sourceFormat, sourceType, data);
	}
	
	void updateSubData(int dataWidth, int dataHeight, int offsetX, int offsetY, void* data){
		assert (data);
		assert (offsetX >= 0 and offsetY >= 0);
		assert (offsetX + dataWidth <= w and offsetY + dataHeight <= h);
		
		bind();
		GLenum format = channelsToFormat(channels);
		glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, dataWidth, dataHeight, format, type, data);
	}
	
	void bind(){
		//avoid unnecessary texture binding
		//if (id != boundTexture){
			boundTexture = id;
			glBindTexture(GL_TEXTURE_2D, id);
		//}
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
	static GLenum channelsToFormat(int channels){
		assert(channels >= 1 and channels <= 4);
		const GLenum channelsToFormatTable[] = {
			0, 
			GL_RED, //1
			GL_RG,  //2
			GL_RGB, //3
			GL_RGBA //4
		};
		return channelsToFormatTable[channels];
	}

	static GLuint boundTexture; //= 0 for no texture

	GLuint id;
	int channels;
	GLenum type; //source type
	
	//access modes
	GLenum min;
	GLenum mag;
	
	int w;
	int h;
};

#endif //TEXTURE_H
