#version 120

uniform sampler2D texture; //1 channel, alpha

varying vec4 vertColor;
varying vec2 texCoord;

void main(){
	float texel = texture2D(texture, texCoord).r;
	gl_FragColor = vec4(vertColor.rgb, texel);
}