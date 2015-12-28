#version 150

uniform sampler2D tex0;

in vec4 mColor;

out vec4 oColor;

void main(void)
{
    oColor = mColor;
}