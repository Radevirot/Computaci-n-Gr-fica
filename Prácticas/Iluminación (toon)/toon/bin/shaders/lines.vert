#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;
uniform float outline_factor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
	vec4 pos4 = vec4(vertexPosition + vertexNormal*outline_factor*0.001, 1.0);
	mat4 modViewProj = projectionMatrix * viewMatrix * modelMatrix;
	gl_Position = modViewProj * pos4;
}
