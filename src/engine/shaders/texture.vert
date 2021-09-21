#version 330 core

in vec2 position;
in vec2 uvCoords;

uniform vec2 v2screen;
uniform vec2 v2screenscale;
uniform vec2 v2screenoffset;

uniform vec2 translation;
uniform vec2 origin;
uniform float zlayer;
uniform vec2 angle;
uniform vec2 scale;

uniform vec2 cropSize;
uniform vec2 uvOffset;

out vec4 vertcolor;
out vec2 uvs;

void main() {

   uvs       = (uvCoords * cropSize) + uvOffset;

   vec2 finalOrig =  (position * cropSize) - origin;
        finalOrig *= (scale*v2screenscale);

   vec2 finalPos = vec2( 
             finalOrig.x * angle.x + finalOrig.y * (-angle.y), 
             finalOrig.x * angle.y + finalOrig.y *  angle.x
        );
        finalPos += (translation*v2screenscale) + v2screenoffset;

   gl_Position = vec4(finalPos / v2screen - vec2(1, -1) ,zlayer , 1);
};
