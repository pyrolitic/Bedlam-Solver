#ifndef SHADER_H
#define SHADER_H

#include <cstdlib>
#include <cstdio>

#include <unordered_map>
#include <string>

#include <GL/glew.h>

#include "../io.h"

class Shader{
public:
	Shader(const char* baseFileName){
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
			build (vertText, fragText);
		}
		else{
			printf("cannot build shader %s, lacking source file\n", name.c_str());
		}

		free((void*)vertText);
		free((void*)fragText);
	}
	
	Shader(const char* vertText, const char* fragText, const char* shaderName = nullptr){
		if (shaderName) name = shaderName;
		else name = "<builtin>";
		
		program = 0;
		build (vertText, fragText);
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
				fprintf(stderr, "shader %s reports no uniform '%s'\n", name.c_str(), varName);
			}
			else{
				uniforms[varName] = location;
			}
		}
		return location;
	}
	
	GLint getAttribLocation(const char* varName){
		return glGetAttribLocation(program, varName);
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
	
	/*GLint getVariableLocation(const char* varName, std::map<std::string, GLint>& container, GLint(*fun)(GLuint, const char*)){
		use();
		
		GLint loc;
		std::map<std::string, GLint>::iterator it = container.find(varName);
		if (it == uniforms.end()){
			loc = fun(program, varName);
			if (loc == -1){
				fprintf(stderr, "no uniform \"%s\" found in shader \"%s\"\n", varName, name.c_str());
				exit(1);
			}
			container.insert(std::pair<std::string, GLint>(varName, loc));
		}
		else{
			loc = it->second;
		}
		
		return loc;
	}*/

	void build(const char* vertText, const char* fragText){
		printf("building shader '%s'\n", name.c_str());
		const char* shader_text[2] = {vertText, fragText};
		program = glCreateProgram();

		GLint status;
		for (int i = 0; i < 2; i++){
			const char* shader_type_name = (i == 0)? "vertex" : "fragment";
			GLenum shader_type = (i == 0)? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
	
			GLuint ob = glCreateShader(shader_type);
			glShaderSource(ob, 1, &shader_text[i], NULL);
			glCompileShader(ob);

			glGetShaderiv(ob, GL_COMPILE_STATUS, &status);

			if (status == GL_FALSE){
				fprintf(stderr, "could not compile %s shader:\n", shader_type_name);
				printShaderLog(ob);
				exit(1);
			}

			glAttachShader(program, ob);
			printf("compiled %s shader\n", shader_type_name);
		}

		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &status);

		if (status == GL_FALSE){
			fprintf(stderr, "could not link shader:\n");
			printShaderLog(program);
			exit(1);
		}
	}
};

#endif
