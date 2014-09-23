#version 120

//per frame, all in world space
uniform vec3 pointLightPosition;
//uniform vec3 pointLightHalfVector;
uniform vec3 directionalLightDirection;
//uniform vec3 directionalHalfVector;

//per material
uniform sampler2D texture; //3 channel, rgb

//per fragment, linearly interpolated input from vertex shader
varying vec2 texCoord; //face space
varying vec3 fragmentPosition; //world space
varying vec3 fragmentNormal; //world space
varying vec4 vertColor;

void main(){
	vec4 texel = texture2D(texture, texCoord);
	vec3 normal = normalize(fragmentNormal);
	vec3 light = vec3(0.0);

	/*vec3 pointLightDiff = pointLightPosition - fragmentPosition;
	float pointLightStrength = max(1.0 - length(pointLightDiff) * 0.5, 0.0);
	float pointLight = max(dot(normalize(pointLightDiff), normal), 0.0) * pointLightStrength;*/

	float dirLight = max(dot(directionalLightDirection, normal), 0.0);
	light += dirLight * vec3(0.5, 0.1, 0.9);

	light = min(light, vec3(1.0));
	gl_FragColor = vertColor * vec4(light, 1.0);
}