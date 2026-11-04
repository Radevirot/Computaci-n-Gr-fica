#version 330 core

in vec3 vertexPosition;

uniform mat4 modelMatrix;

uniform mat4 lightViewMatrix;
uniform mat4 lightProjectionMatrix;


void main() {
	
	// Este vertex shader se usa para renderizar la escena desde la posicion
	// de la luz y deberia ser lo que da lugar a guardar un depth buffer
	// con los objetos mas cercanos a la misma (el shadow map)
	
	// Esto es parte de la primera parte, crea la matriz de transformacion
	// de la luz usando la modelMatrix, la view matrix de la luz y la
	// projection matrix de la luz.
	// Luego, la premultiplica por la posicion de los vertices del modelo.
	
	// Esto se renderiza guardando unicamente el depth buffer.
	
	mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
	gl_Position = lightSpaceMatrix * modelMatrix * vec4(vertexPosition,1.f);
	
	
}
