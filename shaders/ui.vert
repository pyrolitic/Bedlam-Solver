#version 330 core

uniform mat4 projection;
uniform mat4 modelView;

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tex;

out vec2 texCoord;

void main(){
	gl_Position = projection * (modelView * vec4(pos, 0.0f, 1.0f));
	texCoord = tex;
}