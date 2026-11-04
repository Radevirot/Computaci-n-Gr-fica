#version 330 core

// el phong shader se usa si el objeto no tiene textura

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 modelMatrix; // matriz modelo, se usa en ambos casos
uniform mat4 viewMatrix; // matriz vista de la camara
uniform mat4 projectionMatrix; // matriz proyeccion de la camara
uniform vec4 lightPosition; // posicion de la luz

uniform mat4 lightViewMatrix; // matriz vista pero de la luz
uniform mat4 lightProjectionMatrix; // matriz de proyeccion de la luz

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoords;
out vec4 lightVSPosition;
out vec4 fragPosLightSpace;

void main() {
	mat4 vm = viewMatrix * modelMatrix;
	vec4 vmp = vm * vec4(vertexPosition,1.f);
	gl_Position = projectionMatrix * vmp;
	fragPosition = vec3(vmp)/vmp.w;
	fragNormal = mat3(transpose(inverse(vm))) * vertexNormal;
	lightVSPosition = viewMatrix * lightPosition;
	mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
	fragPosLightSpace = lightSpaceMatrix * modelMatrix * vec4(vertexPosition,1.f);
}
