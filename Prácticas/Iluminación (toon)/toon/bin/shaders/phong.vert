#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragPosition;
out vec3 fragNormal;

void main() {
	vec4 pos4 = vec4(vertexPosition, 1.0f);
	mat4 modViewProj = projectionMatrix * viewMatrix * modelMatrix;
	gl_Position = modViewProj * pos4;
	fragPosition = vec3( modelMatrix * pos4 );
	fragNormal = mat3(transpose(inverse(modelMatrix))) * vertexNormal;
}
