#version 330 core

// el texture shader se usa si el objeto tiene textura

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoords;

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
	fragPosition = vec3(vmp)/vmp.w; //pos del fragmento en NDC
	fragNormal = mat3(transpose(inverse(vm))) * vertexNormal; // normal del fragmento
	
	lightVSPosition = viewMatrix * lightPosition; // posicion de la luz en el espacio de la camara
	fragTexCoords = vertexTexCoords; //matcheamos coords de textura de frag con las de vertice, esto porque el modelo tiene textura
	
	// aca se crea la matriz del "espacio de la luz" y se premultiplica por
	// el vertice del objeto ya transformado al espacio de la escena
	
	mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
	fragPosLightSpace = lightSpaceMatrix * modelMatrix * vec4(vertexPosition,1.f);
	
}
