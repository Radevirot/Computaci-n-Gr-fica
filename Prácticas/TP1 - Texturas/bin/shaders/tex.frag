#version 330 core

in vec3 fragNormal;
in vec3 fragPosition;
in vec2 fragTexCoords;
in vec4 lightVSPosition;

// propiedades del material
uniform sampler2D colorTexture; // ambient and diffuse components
uniform vec3 specularColor;
uniform float shininess;

// propiedades de la luz
uniform float ambientStrength;
uniform vec3 lightColor;

out vec4 fragColor;

#include "funcs/calcPhong.frag"

void main() {
	uint s16 = uint(fragTexCoords.x * 65535.0);
	uint t16 = uint(fragTexCoords.y * 65535.0);
	
	uint sHigh = (s16 >> 8u) & 0xFFu;
	uint sLow = s16 & 0xFFu;
	uint tHigh = (t16 >> 8u) & 0xFFu;
	uint tLow = t16 & 0xFFu;
	
	fragColor = vec4(
					 float(sHigh) / 255.0,
					 float(sLow) / 255.0,
					 float(tHigh) / 255.0,
					 float(tLow) / 255.0
	);
}

