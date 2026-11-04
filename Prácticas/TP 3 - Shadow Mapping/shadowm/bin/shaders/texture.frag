#version 330 core

uniform sampler2D colorTexture;

#include "funcs/calcColor.frag"

// en la implementacion del fragment shader de texture y phong, ambos
// llaman a "calcColor.frag", ahi es donde se hacen los calculos para
// determinar si hay o no sombras.

void main() {
	vec4 base_color = texture(colorTexture,fragTexCoords);
	fragColor = calcColor(vec3(base_color));
}

