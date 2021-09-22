std::string universal_vs = 
"#version 330 core\n"
"\n"
"\n"
"//=============================================================================\n"
"// Shader Mode\n"
"//-----------------------------------------------------------------------------\n"
"//=============================================================================\n"
"uniform int u_shader_mode;\n"
"\n"
"//=============================================================================\n"
"// Screen Setup\n"
"//-----------------------------------------------------------------------------\n"
"//=============================================================================\n"
"uniform vec2 u_screen;\n"
"\n"
"//=============================================================================\n"
"// Transform\n"
"//-----------------------------------------------------------------------------\n"
"//=============================================================================\n"
"uniform vec2    u_translation;\n"
"uniform vec2    u_origin;\n"
"uniform float   u_zlayer;\n"
"uniform vec2    u_angle;\n"
"uniform vec2    u_scale;\n"
"\n"
"uniform vec4    u_textureCrop;\n"
"\n"
"//=============================================================================\n"
"// Vector2D Attributes\n"
"//-----------------------------------------------------------------------------\n"
"//=============================================================================\n"
"in vec2 a_position;\n"
"in vec4 a_color;\n"
"in vec2 a_uvCoords;\n"
"\n"
"//=============================================================================\n"
"// Fragment shader setup\n"
"//-----------------------------------------------------------------------------\n"
"//=============================================================================\n"
"out vec4 vertcolor;\n"
"out vec2 uvs;\n"
"\n"
"void main() {\n"
"    vec2 finalOrig;\n"
"    switch(u_shader_mode) {\n"
"        case 0: /* Shape 2D */\n"
"            vertcolor = a_color;\n"
"            finalOrig = a_position - u_origin;\n"
"            break;\n"
"\n"
"        case 1: /* Texture */\n"
"            uvs = ( a_uvCoords * u_textureCrop.xy )\n"
"                     + u_textureCrop.zw;\n"
"            finalOrig = (a_position * u_textureCrop.xy) - u_origin;\n"
"            break;\n"
"    }\n"
"\n"
"    finalOrig *= u_scale;\n"
"\n"
"    vec2 finalPos = vec2( \n"
"        finalOrig.x * u_angle.x + finalOrig.y * (-u_angle.y), \n"
"        finalOrig.x * u_angle.y + finalOrig.y *   u_angle.x\n"
"    ) + u_translation;\n"
"\n"
"\n"
"    gl_Position = vec4( \n"
"            ((finalPos / u_screen) * vec2(1, -1)) + vec2(-1, 1)\n"
"            , u_zlayer , 1);\n"
"}\n"
;


std::string universal_fs = 
"#version 330 core\n"
"\n"
"//=============================================================================\n"
"// Shader Mode\n"
"//-----------------------------------------------------------------------------\n"
"//=============================================================================\n"
"uniform int u_shader_mode;\n"
"\n"
"//=============================================================================\n"
"// Colors\n"
"//-----------------------------------------------------------------------------\n"
"//=============================================================================\n"
"uniform vec4 u_drawcolor;\n"
"uniform sampler2D u_texture;\n"
"\n"
"//=============================================================================\n"
"// Fragment shader setup\n"
"//-----------------------------------------------------------------------------\n"
"//=============================================================================\n"
"in vec4 vertcolor;\n"
"in vec2 uvs;\n"
"\n"
"void main() {\n"
"    switch(u_shader_mode) {\n"
"        case 0: /* Shape2D */\n"
"	        gl_FragColor = vertcolor * u_drawcolor;\n"
"            break;\n"
"\n"
"        case 1: /* Texture */\n"
"	        gl_FragColor = texture(u_texture, uvs);\n"
"            break;\n"
"    }\n"
"\n"
"    gl_FragColor *= u_drawcolor;\n"
"};\n"
"\n"
;