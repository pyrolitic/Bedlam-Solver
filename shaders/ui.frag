#version 120

uniform sampler2D texture;
uniform vec4 entityColor;

varying vec2 texCoord;

void main(){
	vec4 texel = texture2D(texture, texCoord);
	gl_FragColor = texel * entityColor;
}