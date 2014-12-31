#version 120

uniform sampler2D tex0;
uniform mat4 texTransform;
uniform float zoom;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 there = gl_TexCoord[0] * texTransform;
	gl_FrontColor = texture2D( tex0, there.st);
	gl_Position = ftransform(); // gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position.y += gl_FrontColor.r * 20.0 * (1.0 - zoom);
}
