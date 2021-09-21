std::string texture_vs = 
"#version 330 core\n"
"\n"
"in vec2 position;\n"
"in vec2 uvCoords;\n"
"\n"
"uniform vec2 v2screen;\n"
"uniform vec2 v2screenscale;\n"
"uniform vec2 v2screenoffset;\n"
"\n"
"uniform vec2 translation;\n"
"uniform vec2 origin;\n"
"uniform float zlayer;\n"
"uniform vec2 angle;\n"
"uniform vec2 scale;\n"
"\n"
"out vec4 vertcolor;\n"
"out vec2 uvs;\n"
"\n"
"void main() {\n"
"\n"
"   uvs       = uvCoords;\n"
"\n"
"   vec2 finalOrig = position - origin;\n"
"        finalOrig *= (scale*v2screenscale);\n"
"\n"
"   vec2 finalPos = vec2( finalOrig.x * angle.x + finalOrig.y * (-angle.y), finalOrig.x * angle.y + finalOrig.y *  angle.x);\n"
"        finalPos += (translation*v2screenscale) + v2screenoffset;\n"
"\n"
"   gl_Position = vec4(finalPos / v2screen - vec2(1, -1) ,zlayer , 1);\n"
"};\n"
;


std::string texture_fs = 
"#version 330 core\n"
"\n"
"uniform vec4 drawcolor;\n"
"uniform sampler2D mytexture;\n"
"\n"
"in vec2 uvs;\n"
"\n"
"void main() {\n"
"	gl_FragColor = texture(mytexture, uvs) * drawcolor;\n"
"};\n"
;
