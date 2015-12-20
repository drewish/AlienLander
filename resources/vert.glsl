#version 150

uniform mat4        textureMatrix;
uniform mat4        ciModelViewProjection;
uniform sampler2D   uTex0;

in vec4			ciPosition;
in vec4			ciColor;
out vec4		Color;

void main( void ) {
    Color = ciColor;
    vec4 position = ciPosition;
    // Skip the texture lookup for positions with 0 or negative y values. We use
    // y=0 for the bottom of the masking strip and it doesn't need to move.
    if (position.y > 0.0) {
        // Texture is 2d but our matrix should affect the z values...
        vec4 texturePos = textureMatrix * vec4( ciPosition.xzy, 1 );
        // Pull the red channel (it's a grayscale image) and scale by the
        // value in the matrix.
        position.y = texture( uTex0, texturePos.xy ).r * texturePos.z;
    }
    gl_Position = ciModelViewProjection * position;

}
