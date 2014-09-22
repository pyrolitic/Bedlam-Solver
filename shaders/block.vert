#version 120

//per frame/entity input
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix; //transpose(inverse(3x3 part of model matrix))
uniform mat3 textureMatrix;

//per vertex input, in object space
attribute vec3 pos;
attribute vec3 nor;
attribute vec2 tex;
attribute vec4 col;

//extra output
varying vec2 texCoord; //face space
varying vec3 fragmentPosition; //world space
varying vec3 fragmentNormal; //world space
varying vec4 vertColor;

void main(){
	gl_Position = modelViewProjectionMatrix * vec4(pos, 1.0f);

	fragmentPosition = (modelMatrix * vec4(pos, 1.0f)).xyz;
	fragmentNormal = normalMatrix * nor;

	texCoord = (textureMatrix * vec3(tex, 1.0)).xy;
	vertColor = col;
}