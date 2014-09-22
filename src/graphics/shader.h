#ifndef SHADER_H
#define SHADER_H

#include <cstdlib>
#include <cstdio>

#include <unordered_map>
#include <string>

#include <GL/glew.h>

#include "../io.h"

//made for opengl 2.1 / glsl 120
class Shader{
public:
	struct attribLocationPair{
		int place;
		const char* name;
	};

	//if attribLocations is null, then don't bind attribs
	Shader(const char* baseFileName, const attribLocationPair* attribLocations){
		std::string base(baseFileName);
		
		//file name only, *nix and windoze
		size_t lastSlash = base.rfind('/');
		if (lastSlash == std::string::npos) lastSlash = base.rfind('\\');
		if (lastSlash != std::string::npos) name = base.substr(lastSlash + 1);
		else name = base;
		
		const char* vertText = (const char*) IO::readWholeFile((base + ".vert").c_str(), nullptr);
		const char* fragText = (const char*) IO::readWholeFile((base + ".frag").c_str(), nullptr);

		program = 0;

		if (vertText and fragText){
			build (vertText, fragText, attribLocations);
		}
		else{
			printf("cannot build shader %s, lacking source file\n", name.c_str());
		}

		free((void*)vertText);
		free((void*)fragText);
	}
	
	Shader(const char* vertText, const char* fragText, const attribLocationPair* attribLocations, const char* shaderName = nullptr){
		if (shaderName) name = shaderName;
		else name = "<builtin>";
		
		program = 0;
		build (vertText, fragText, attribLocations);
	}

	~Shader(){
		glDeleteProgram(program);
	}

	GLint getUniformLocation(const char* varName){
		GLint location;
		auto it = uniforms.find(varName);
		if (it != uniforms.end()){
			//hit
			location = it->second;
		}
		else{
			//miss
			location = glGetUniformLocation(program, varName);
			if (location == -1){
				//happens even if it exists but was compiled out if it doesn't affect the output
				fprintf(stderr, "shader '%s' reports no uniform '%s'\n", name.c_str(), varName);
			}

			uniforms[varName] = location;
		}
		return location;
	}
	
	GLint getAttribLocation(const char* varName){
		GLint location = glGetAttribLocation(program, varName);
		if (location == -1){ //TODO: I don't thin that's the right negative symbollic value
			fprintf(stderr, "shader '%s' reports no attribute '%s'\n", name.c_str(), varName);
		}

		return location;
	}

	void use(){
		//if (program != boundProgram){
			boundProgram = program;
			glUseProgram(program);
		//}
	}

	static void useDefaultProgram(){
		//if (boundProgram != 0){
			boundProgram = 0;
			glUseProgram(boundProgram);
		//}
	}

private:
	static GLuint boundProgram; //= 0 for fixed pipeline
	//static std::unordered_map<std::string, Shader*> shadersLoaded;
	//TODO:keep track of all shaders created and their source files, listen for file changes and rebuild when a file is changed

	std::string name;
	GLuint program;
	std::unordered_map<std::string, GLint> uniforms;
	//std::unordered_map<std::string, GLint> attribs;


	void printShaderLog(GLuint ob){
		GLint len;
		glGetShaderiv(ob, GL_INFO_LOG_LENGTH, &len); 
		GLchar* info = (GLchar*)malloc(len + 1);
		glGetShaderInfoLog(ob, len, NULL, info);
		fprintf(stderr, "%s\n", info);
		free((void*)info);
	}

	void build(const char* vertText, const char* fragText, const attribLocationPair* attribLocations){
		printf("building shader '%s'\n", name.c_str());
		const char* shader_text[2] = {vertText, fragText};
		program = glCreateProgram();

		GLint status;
		GLuint shaderObj[2];
		for (int i = 0; i < 2; i++){
			const char* shader_type_name = (i == 0)? "vertex" : "fragment";
			GLenum shaderType = (i == 0)? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
	
			shaderObj[i] = glCreateShader(shaderType);
			glShaderSource(shaderObj[i], 1, &shader_text[i], NULL);
			glCompileShader(shaderObj[i]);

			glGetShaderiv(shaderObj[i], GL_COMPILE_STATUS, &status);

			if (status == GL_FALSE){
				fprintf(stderr, "could not compile %s shader:\n", shader_type_name);
				printShaderLog(shaderObj[i]);
				exit(1);
			}

			glAttachShader(program, shaderObj[i]);
			printf("compiled %s shader\n", shader_type_name);
		}

		if (attribLocations){
			for (const attribLocationPair* p = attribLocations; p->name; p++){
				glBindAttribLocation(program, p->place, p->name);
			}
		}

		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &status);

		if (status == GL_FALSE){
			fprintf(stderr, "could not link shader:\n");
			printShaderLog(program);
			exit(1);
		}

		for (int i = 0; i < 2; i++){
			//mark for deletion; will be deleted when the program is deleted
			glDeleteShader(shaderObj[i]);
		}
	}
};

#endif
