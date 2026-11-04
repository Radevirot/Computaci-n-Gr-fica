#version 330 core

in vec3 vertexPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragPosition;

void main() {
	
	// Por lo que entiendo este deberia ser el calculo de la posicion de la luz
	// desde la posicion de la camara.
	
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition,1.f);
}
