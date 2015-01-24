#version 120

uniform sampler2D tex0;
uniform float zoom;

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    vec4 heightCoord = gl_TextureMatrix[0] * gl_TexCoord[0];
    vec4 heightColor = texture2D( tex0, heightCoord.st );
    gl_FrontColor = gl_Color * gl_Color; // try: heightColor;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_Position.y += heightColor.r * ( 1.0 - zoom );
}
