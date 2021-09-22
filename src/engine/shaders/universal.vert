#version 330 core


//=============================================================================
// Shader Mode
//-----------------------------------------------------------------------------
//=============================================================================
uniform int u_shader_mode;

//=============================================================================
// Screen Setup
//-----------------------------------------------------------------------------
//=============================================================================
uniform vec2 u_screen;

//=============================================================================
// Transform
//-----------------------------------------------------------------------------
//=============================================================================
uniform vec2    u_translation;
uniform vec2    u_origin;
uniform float   u_zlayer;
uniform vec2    u_angle;
uniform vec2    u_scale;

uniform vec4    u_textureCrop;

//=============================================================================
// Vector2D Attributes
//-----------------------------------------------------------------------------
//=============================================================================
in vec2 a_position;
in vec4 a_color;
in vec2 a_uvCoords;

//=============================================================================
// Fragment shader setup
//-----------------------------------------------------------------------------
//=============================================================================
out vec4 vertcolor;
out vec2 uvs;

void main() {
    vec2 finalOrig;
    switch(u_shader_mode) {
        case 0: /* Shape 2D */
            vertcolor = a_color;
            finalOrig = a_position - u_origin;
            break;

        case 1: /* Texture */
            uvs = ( a_uvCoords * u_textureCrop.xy )
                     + u_textureCrop.zw;
            finalOrig = (a_position * u_textureCrop.xy) - u_origin;
            break;
    }

    finalOrig *= u_scale;

    vec2 finalPos = vec2( 
        finalOrig.x * u_angle.x + finalOrig.y * (-u_angle.y), 
        finalOrig.x * u_angle.y + finalOrig.y *   u_angle.x
    ) + u_translation;


    gl_Position = vec4( 
            ((finalPos / u_screen) * vec2(1, -1)) + vec2(-1, 1)
            , u_zlayer , 1);
}
