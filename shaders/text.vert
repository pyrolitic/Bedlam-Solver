#version 120

uniform mat4 projection;
uniform mat4 modelView;

attribute vec2 pos;
attribute vec2 tex;

varying vec2 texCoord;

void main(){
	gl_Position = projection * (modelView * vec4(pos, 0.0f, 1.0f));
	texCoord = tex;
}