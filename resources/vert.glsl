#version 120

uniform sampler2D tex0;
uniform mat4 texTransform;
uniform float zoom;

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    vec4 heightCoord = gl_TexCoord[0] * texTransform;
    vec4 heightColor = texture2D( tex0, heightCoord.st);
    gl_FrontColor = gl_Color;
    gl_Position = ftransform(); // gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_Position.y += heightColor.r * 20.0 * (1.0 - zoom);
}
