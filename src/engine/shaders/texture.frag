#version 330 core

uniform vec4 drawcolor;
uniform sampler2D mytexture;

in vec2 uvs;

void main() {
	gl_FragColor = texture(mytexture, uvs) * drawcolor;
};
