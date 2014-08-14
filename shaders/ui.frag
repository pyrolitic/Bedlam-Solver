#version 330 core

uniform sampler2D texture;
uniform vec4 entityColor;

in vec2 texCoord;

out vec4 outColor;

void main(){
	vec4 texel = texture2D(texture, texCoord);
	outColor = texel * entityColor;
}