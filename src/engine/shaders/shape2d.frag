#version 330 core

out vec4 color;

uniform vec4 drawcolor;
uniform sampler2D mytexture;
uniform float texture_enable;

in vec4 vertcolor;
in vec2 uvs;

void main() {
	color = 
      ((texture_enable * texture(mytexture, uvs)) 
        + ((1.0 - texture_enable) * vec4(1.0, 1.0, 1.0, 1.0)))
        * drawcolor * vertcolor;
};
