#version 120

//per frame/entity input
uniform mat4 modelViewProjectionMatrix;
uniform mat3 textureMatrix;

//per vertex input
attribute vec3 pos;
attribute vec3 nor; //unused
attribute vec2 tex;
attribute vec4 col;

//extra output
varying vec2 texCoord;
varying vec4 vertColor;

void main(){
	gl_Position = modelViewProjectionMatrix * vec4(pos, 1.0f);
	texCoord = (textureMatrix * vec3(tex, 1.0)).xy;
	vertColor = col;
}