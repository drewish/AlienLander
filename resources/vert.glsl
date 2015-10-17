#version 150

uniform mat4        textureMatrix;
uniform mat4        ciModelViewProjection;
uniform sampler2D   uTex0;

in vec4			ciPosition;
in vec4			ciColor;
in vec2			ciTexCoord0;

out vec4		Color;

void main( void ) {
    // TODO: should probably get rid of this zoom rather than hard coding it.
    float zoom = 0.5;

    vec4 heightSample = texture( uTex0, (textureMatrix * vec4(ciTexCoord0, 0, 1)).st );
    Color = ciColor;
    gl_Position = ciModelViewProjection * ciPosition;
    gl_Position.y += heightSample.r * ( 1.0 - zoom );
}
