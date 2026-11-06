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

uniform float desvio;
uniform int samples;
uniform float texScaleFactor;

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
	// --- return currentDepth > closestDepth ? 1.0 : 0.0;
	
	
	// PROBLEMA 1: "Shadow Acne"
	//
	// Las partes iluminadas se ven con un patron cuadriculado inclinado de iluminacion
	// y oscuridad. Esto pasa cuando la fuente de luz esta inclinada, el shadow map
	// se inclina tambien y está lejos de la fuente de luz, haciendo que varios fragmentos
	// caigan en el mismo texel inclinado del shadow map.
	
	// SOLUCIÓN 1:
	// restarle un pequeño desvío a la profundidad actual, para que al hacer la comparación
	// siempre encuentre que los fragmentos de las superficies están más cerca que los
	// texeles inclinados del shadow map.
	
	// Como esto entra en la categoria de valor mágico, tengo que agregarlo
	// como slider en la cajita de la interfaz, así que lo voy a pasar como
	// uniform desde arriba y lo voy a setear desde el main.
	
	// --- return currentDepth - desvio > closestDepth ? 1.0 : 0.0;
	
	
	// PROBLEMA 2: PETER PANNING
	// Por querer solucionar el problema del shadow acne con un desvio, estamos
	// técnicamente "acercando" lo que "ve" la luz en el momento hacia ella,
	// esto hace que las sombras no salgan de donde deberían en los objetos:
	// como estamos achicando la profundidad actual, el checkeo de la condicion
	// acaba haciendo que se iluminen partes que aún deberían estar oscuras y
	// que las sombras comiencen más tarde y/o desplazadas de las posiciones
	// de los objetos. 
	
	// SOLUCIÓN 2:
	// En vez de generar el shadow map usando las caras frontales de los modelos
	// (así es como funciona el back culling), vamos a generarlo usando las caras
	// traseras (front-face culling).
	// --- PREGUNTAR ---
	// Acá yo pensé que la implementación venía con back-culling, pero ya venía
	// con front culling, por lo que agregué la opción de switchear entre
	// ambos modos y, curiosamente, la opción que pareciera ser más fiel es
	// la que usa back-culling, ¿asumo que esto es porque al usar las caras
	// con las que la luz da directamente se genera un mapa de profundidad
	// más certero? ¿Y que el problema de peter panning ocurre con back-culling
	// solamente si el desvio es exageradamente alto?
	
	
	// PROBLEMA 3: BORDES DE SOMBRAS SERRUCHADOS
	// Acá pasa lo mismo que cuando trabajás con texturas comunes y corrientes:
	// como la resolución de la textura elegida es bastante chica, te caen
	// varios fragmentos en el mismo texel (o sea, hay varios fragmentos que 
	// tienen el mismo nivel de profundidad).
	
	// SOLUCIÓN 3:
	// Acá hay dos opciones, o hacés más grande la textura del shadow map
	// (agregué un slider que te deja ponerla entre 1024 y 8192), o usás
	// alguna técnica de anti-aliasing para esos bordes.
	// Por lo que investigué se sugiere usar una llamada "Percentage-Closer
	// Filtering" o PCF.
	// Básicamente consiste en agarrar varios texeles alrededor del fragmento
	// objetivo, revisamos si son o no son sombra y promediamos todos los valores
	// obtenidos. Podríamos decir que estamos interpolando todos los texeles con
	// igual peso.
	// Voy a hacer que el tamaño del texel y la cantidad de texeles a promediar
	// sean valores configurables.
	
	float shadow = 0.0f;
	
	vec2 texelSize = texScaleFactor / textureSize(depthTexture,0);
	
	for(int x = -samples; x <= samples; ++x)
	{
		for(int y = -samples; y <= samples; ++y)
		{
			float pcf = texture(depthTexture, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - desvio > pcf ? 1.0 : 0.0;        
		}    
	}
	shadow /= ((samples*2)+1)*((samples*2)+1);
	
	return shadow;
	
	
	
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
