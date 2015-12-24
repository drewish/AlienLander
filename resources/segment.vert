#version 150

uniform mat4 ciModelViewProjection;
uniform vec4 onColor;
uniform vec4 offColor;

in vec4 ciPosition;
in int  ciBoneIndex;
// Per-instance position of the character.
in vec3 vInstancePosition;
// Bitmapped value for the display. The segments of the display are grouped
// using the bone index.
in int  vInstanceValue;

out lowp vec4 Color;

void main( void )
{
    // Discard z to keep us on the xy plane.
    vec4 instanceOffset = vec4( vInstancePosition.xy, 0, 0 );
    gl_Position	= ciModelViewProjection * ( ciPosition + instanceOffset );
    // Figure out the vertex color using the display's bits and the bone index.
    Color = (vInstanceValue & (1 << ciBoneIndex)) == 0 ?  offColor: onColor;
}
