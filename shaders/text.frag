#version 330 core

uniform sampler2D texture;
uniform vec4 entityColor;

in vec2 texCoord;

out vec4 outColor;

void main(){
	float alpha = texture2D(texture, texCoord).r;
	outColor = entityColor * vec4(1, 1, 1, alpha);
}