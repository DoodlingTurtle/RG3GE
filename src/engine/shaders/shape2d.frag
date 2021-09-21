#version 330 core

uniform vec4 drawcolor;

in vec4 vertcolor;

void main() {
	gl_FragColor = drawcolor * vertcolor;
};
