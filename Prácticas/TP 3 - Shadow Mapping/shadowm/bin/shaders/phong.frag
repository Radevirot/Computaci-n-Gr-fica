#version 330 core
#include "funcs/calcColor.frag"

// en la implementacion del fragment shader de texture y phong, ambos
// llaman a "calcColor.frag", ahi es donde se hacen los calculos para
// determinar si hay o no sombras.

void main() {
	fragColor = calcColor(vec3(1.f,1.f,1.f));
}

