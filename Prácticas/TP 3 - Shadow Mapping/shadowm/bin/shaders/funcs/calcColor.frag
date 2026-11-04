in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoords;
in vec4 lightVSPosition;
in vec4 fragPosLightSpace;

uniform sampler2D depthTexture;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform vec3 emissionColor;
uniform float shininess;
uniform float opacity;

uniform float ambientStrength;
uniform vec3 lightColor;

out vec4 fragColor;

#include "calcPhong.frag"

float calcShadow(vec4 fragPosLightSpace) {
	
	// vamos del espacio proyectivo al NDC dividiendo todo por w
	vec3 projCoords = vec3(fragPosLightSpace) / fragPosLightSpace.w;
	
	// el NDC va de -1 a 1 en todas las coords, nos conviene por como
	// se guarda el depth buffer (entre 0 y 1), normalizar este rango de valores
	projCoords = projCoords * 0.5 + 0.5;
	
	// agarro la profundidad actual desde el punto de vista de la luz
	float currentDepth = projCoords.z;
	
	// agarro la profundidad que hay en el shadow map (primera pasada del render)
	float closestDepth = texture(depthTexture, projCoords.xy).r;
	
	// si la actual esta mas cerca, hay luz (1), sino hay sombra (0)
	return currentDepth > closestDepth ? 1.0 : 0.0;
	
	
	// PROBLEMA 1: "Shadow Acne"
	//
	// Las partes iluminadas se ven con un patron cuadriculado inclinado de iluminacion
	// y oscuridad. Esto pasa cuando la fuente de luz esta inclinada y el shadow map
	// se inclina tambien, y el shadow map esta lejos de la luz, haciendo que varios fragmentos
	// caigan en el mismo texel inclinado del shadow map. La solucion a este problema es
	// 
	
	
}

vec4 calcColor(vec3 textureColor) {
	vec3 phong = calcPhong(lightVSPosition, lightColor, ambientStrength,
						   ambientColor*textureColor, diffuseColor*textureColor,
						   specularColor, shininess);
	
	float shadow = calcShadow(fragPosLightSpace);
	vec3 ambientComponent = ambientColor*textureColor*ambientStrength;
	vec3 lighting = mix(phong, ambientComponent, shadow);
	return vec4(lighting+emissionColor,opacity);
}
