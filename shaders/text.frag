#version 120

uniform sampler2D texture;
uniform vec4 entityColor;

varying vec2 texCoord;

void main(){
	float alpha = texture2D(texture, texCoord).r;
	gl_FragColor = entityColor * vec4(1, 1, 1, alpha);
}