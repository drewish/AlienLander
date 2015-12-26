//
//  SegmentDisplay.cpp
//  Segments
//
//  Created by Andrew Morton on 11/6/14.
//
//

#include "SegmentDisplay.h"
#include "Resources.h"

using namespace ci;
using namespace ci::geom;
using namespace std;

static const uint SEGMENTS = 16;
static const uint VERTS_PER_SEGMENT = 12;
// Awesome font stolen from http://www.msarnoff.org/alpha32/
// The first 32, non-printable ASCII characters are omitted.
static const uint CHAR_OFFSET = 32;
static const uint CHAR_LENGTH = 96;
static uint16_t charPatterns[CHAR_LENGTH] = {
    0x0000,  /*   */ 0x1822,  /* ! */ 0x0880,  /* " */ 0x4b3c,  /* # */
    0x4bbb,  /* $ */ 0xdb99,  /* % */ 0x2d79,  /* & */ 0x1000,  /* ' */
    0x3000,  /* ( */ 0x8400,  /* ) */ 0xff00,  /* * */ 0x4b00,  /* + */
    0x8000,  /* , */ 0x0300,  /* - */ 0x0020,  /* . */ 0x9000,  /* / */
    0x90ff,  /* 0 */ 0x100c,  /* 1 */ 0x0377,  /* 2 */ 0x123b,  /* 3 */
    0x038c,  /* 4 */ 0x21b3,  /* 5 */ 0x03fb,  /* 6 */ 0x000f,  /* 7 */
    0x03ff,  /* 8 */ 0x03bf,  /* 9 */ 0x0021,  /* : */ 0x8001,  /* ; */
    0x9030,  /* < */ 0x0330,  /* = */ 0x2430,  /* > */ 0x4207,  /* ? */
    0x417f,  /* @ */ 0x03cf,  /* A */ 0x4a3f,  /* B */ 0x00f3,  /* C */
    0x483f,  /* D */ 0x01f3,  /* E */ 0x01c3,  /* F */ 0x02fb,  /* G */
    0x03cc,  /* H */ 0x4833,  /* I */ 0x4863,  /* J */ 0x31c0,  /* K */
    0x00f0,  /* L */ 0x14cc,  /* M */ 0x24cc,  /* N */ 0x00ff,  /* O */
    0x03c7,  /* P */ 0x20ff,  /* Q */ 0x23c7,  /* R */ 0x03bb,  /* S */
    0x4803,  /* T */ 0x00fc,  /* U */ 0x90c0,  /* V */ 0xa0cc,  /* W */
    0xb400,  /* X */ 0x5400,  /* Y */ 0x9033,  /* Z */ 0x00e1,  /* [ */
    0x2400,  /* \ */ 0x001e,  /* ] */ 0xa000,  /* ^ */ 0x0030,  /* _ */
    0x0400,  /* ` */ 0x4170,  /* a */ 0x41e0,  /* b */ 0x0160,  /* c */
    0x4960,  /* d */ 0x8160,  /* e */ 0x4b02,  /* f */ 0x2238,  /* g */
    0x41c0,  /* h */ 0x4000,  /* i */ 0x4020,  /* j */ 0x6a00,  /* k */
    0x4811,  /* l */ 0x4348,  /* m */ 0x4140,  /* n */ 0x4160,  /* o */
    0x09c1,  /* p */ 0x4991,  /* q */ 0x0140,  /* r */ 0x4220,  /* s */
    0x4b10,  /* t */ 0x4060,  /* u */ 0x8040,  /* v */ 0x4078,  /* w */
    0xd800,  /* x */ 0x2038,  /* y */ 0x8120,  /* z */ 0x4912,  /* { */
    0x4800,  /* | */ 0x4a21,  /* } */ 0x0a85,  /* ~ */ 0x0000,  /* DEL */
};
static const ci::vec2 DISPLAY_DIMENSIONS = ci::vec2( 16, 24 );
// Segments (order is 0-F):
//
//   *-0-*-1-*
//   |\  |  /|
//   7 A B C 2
//   |  \|/  |
//   *-8-*-9-*
//   |  /|\  |
//   6 F E D 3
//   |/  |  \|
//   *-5-*-4-*
//
// We need to color segments individually so we can't share vertexes between
// them.
static const vector<vec3> DISPLAY_VERTS = {
    // Segment 0
    vec3( 1, 1, 1 ),    vec3( 2, 2, 1 ),   vec3( 2, 0, 1 ),
    vec3( 2, 2, 1 ),    vec3( 2, 0, 1 ),   vec3( 6, 2, 1 ),
    vec3( 2, 0, 1 ),    vec3( 6, 2, 1 ),   vec3( 6, 0, 1 ),
    vec3( 6, 2, 1 ),    vec3( 6, 0, 1 ),   vec3( 7, 1, 1 ),

    // Segment 1 (Segment 0 + vec(6, 0))
    vec3( 7, 1, 1 ),    vec3( 8, 2, 1 ),    vec3( 8, 0, 1 ),
    vec3( 8, 2, 1 ),    vec3( 8, 0, 1 ),    vec3( 12, 2, 1 ),
    vec3( 8, 0, 1 ),    vec3( 12, 2, 1 ),   vec3( 12, 0, 1 ),
    vec3( 12, 2, 1 ),   vec3( 12, 0, 1 ),   vec3( 13, 1, 1 ),

    // Segment 2
    vec3( 13, 1, 1 ),   vec3( 12, 2, 1 ),   vec3( 14, 2, 1 ),
    vec3( 12, 2, 1 ),   vec3( 14, 2, 1 ),   vec3( 12, 10, 1 ),
    vec3( 14, 2, 1 ),   vec3( 12, 10, 1 ),  vec3( 14, 10, 1 ),
    vec3( 12, 10, 1 ),  vec3( 14, 10, 1 ),  vec3( 13, 11, 1 ),

    // Segment 3 (Segment 2 + vec(0, 10))
    vec3( 13, 11, 1 ),  vec3( 12, 12, 1 ),  vec3( 14, 12, 1 ),
    vec3( 12, 12, 1 ),  vec3( 14, 12, 1 ),  vec3( 12, 20, 1 ),
    vec3( 14, 12, 1 ),  vec3( 12, 20, 1 ),  vec3( 14, 20, 1 ),
    vec3( 12, 20, 1 ),  vec3( 14, 20, 1 ),  vec3( 13, 21, 1 ),

    // Segment 4 (Segment 0 + vec(6, 20))
    vec3( 7, 21, 1 ),   vec3( 8, 22, 1 ),   vec3( 8, 20, 1 ),
    vec3( 8, 22, 1 ),   vec3( 8, 20, 1 ),   vec3( 12, 22, 1 ),
    vec3( 8, 20, 1 ),   vec3( 12, 22, 1 ),  vec3( 12, 20, 1 ),
    vec3( 12, 22, 1 ),  vec3( 12, 20, 1 ),  vec3( 13, 21, 1 ),

    // Segment 5 (Segment 0 + vec(0, 20))
    vec3( 1, 21, 1 ),   vec3( 2, 22, 1 ),   vec3( 2, 20, 1 ),
    vec3( 2, 22, 1 ),   vec3( 2, 20, 1 ),   vec3( 6, 22, 1 ),
    vec3( 2, 20, 1 ),   vec3( 6, 22, 1 ),   vec3( 6, 20, 1 ),
    vec3( 6, 22, 1 ),   vec3( 6, 20, 1 ),   vec3( 7, 21, 1 ),

    // Segment 6 (Segment 2 + vec(-12, -10))
    vec3( 1, 11, 1 ),   vec3( 0, 12, 1 ),   vec3( 2, 12, 1 ),
    vec3( 0, 12, 1 ),   vec3( 2, 12, 1 ),   vec3( 0, 20, 1 ),
    vec3( 2, 12, 1 ),   vec3( 0, 20, 1 ),   vec3( 2, 20, 1 ),
    vec3( 0, 20, 1 ),   vec3( 2, 20, 1 ),   vec3( 1, 21, 1 ),

    // Segment 7 (Segment 2 + vec(-12, 0)
    vec3( 1, 1, 1 ),    vec3( 0, 2, 1 ),    vec3( 2, 2, 1 ),
    vec3( 0, 2, 1 ),    vec3( 2, 2, 1 ),    vec3( 0, 10, 1 ),
    vec3( 2, 2, 1 ),    vec3( 0, 10, 1 ),   vec3( 2, 10, 1 ),
    vec3( 0, 10, 1 ),   vec3( 2, 10, 1 ),   vec3( 1, 11, 1 ),

    // Segment 8 (Segment 0 + vec(0, 10))
    vec3( 1, 11, 1 ),   vec3( 2, 12, 1 ),   vec3( 2, 10, 1 ),
    vec3( 2, 12, 1 ),   vec3( 2, 10, 1 ),   vec3( 6, 12, 1 ),
    vec3( 2, 10, 1 ),   vec3( 6, 12, 1 ),   vec3( 6, 10, 1 ),
    vec3( 6, 12, 1 ),   vec3( 6, 10, 1 ),   vec3( 7, 11, 1 ),

    // Segment 9 (Segment 0 + vec(6, 10))
    vec3( 7, 11, 1 ),   vec3( 8, 12, 1 ),   vec3( 8, 10, 1 ),
    vec3( 8, 12, 1 ),   vec3( 8, 10, 1 ),   vec3( 12, 12, 1 ),
    vec3( 8, 10, 1 ),   vec3( 12, 12, 1 ),  vec3( 12, 10, 1 ),
    vec3( 12, 12, 1 ),  vec3( 12, 10, 1 ),  vec3( 13, 11, 1 ),

    // Segment A
    vec3( 2, 2, 1 ),    vec3( 2, 4.3, 1 ),   vec3( 3, 2, 1 ),
    vec3( 2, 4.3, 1 ),  vec3( 3, 2, 1 ),     vec3( 5.3, 10, 1 ),
    vec3( 3, 2, 1 ),    vec3( 5.3, 10, 1 ),  vec3( 6, 7.2, 1 ),
    vec3( 5.3, 10, 1 ), vec3( 6, 7.2, 1 ),   vec3( 6, 10, 1 ),

    // Segment B (Segment 2 + vec(-6, 0))
    vec3( 7, 1, 1 ),    vec3( 6, 2, 1 ),     vec3( 8, 2, 1 ),
    vec3( 6, 2, 1 ),    vec3( 8, 2, 1 ),     vec3( 6, 10, 1 ),
    vec3( 8, 2, 1 ),    vec3( 6, 10, 1 ),    vec3( 8, 10, 1 ),
    vec3( 6, 10, 1 ),   vec3( 8, 10, 1 ),    vec3( 7, 11, 1 ),

    // Segment C
    vec3( 8, 10, 1 ),   vec3( 8.7, 10, 1 ),  vec3( 8, 7.2, 1 ),
    vec3( 8.7, 10, 1 ), vec3( 8, 7.2, 1 ),   vec3( 12, 4.3, 1 ),
    vec3( 8, 7.2, 1 ),  vec3( 12, 4.3, 1 ),  vec3( 11, 2, 1 ),
    vec3( 12, 4.3, 1 ), vec3( 11, 2, 1 ),    vec3( 12, 2, 1 ),

    // Segment D
    vec3( 8, 12, 1 ),   vec3( 8, 14.8, 1 ),  vec3( 8.7, 12, 1 ),
    vec3( 8, 14.8, 1 ), vec3( 8.7, 12, 1 ),  vec3( 11, 20, 1 ),
    vec3( 8.7, 12, 1 ), vec3( 11, 20, 1 ),   vec3( 12, 17.7, 1 ),
    vec3( 11, 20, 1 ),  vec3( 12, 17.7, 1 ), vec3( 12, 20, 1 ),

    // Segment E (Segment 2 + vec(-6, 10))
    vec3( 7, 11, 1 ),   vec3( 6, 12, 1 ),    vec3( 8, 12, 1 ),
    vec3( 6, 12, 1 ),   vec3( 8, 12, 1 ),    vec3( 6, 20, 1 ),
    vec3( 8, 12, 1 ),   vec3( 6, 20, 1 ),    vec3( 8, 20, 1 ),
    vec3( 6, 20, 1 ),   vec3( 8, 20, 1 ),    vec3( 7, 21, 1 ),

    // Segment F
    vec3( 2, 20, 1 ),   vec3( 3, 20, 1 ),    vec3( 2, 17.7, 1 ),
    vec3( 3, 20, 1 ),   vec3( 2, 17.7, 1 ),  vec3( 6, 14.8, 1 ),
    vec3( 2, 17.7, 1 ), vec3( 6, 14.8, 1 ),  vec3( 5.3, 12, 1 ),
    vec3( 6, 14.8, 1 ), vec3( 5.3, 12, 1 ),  vec3( 6, 12, 1 ),
};


SegmentDisplay::SegmentDisplay(uint length)
    : mDigits(length)
{
    mPosition = ci::vec2();
    mScale = 1.0;
    mColors[1] = vec4( 1, 0, 0, 1 );
    mColors[0] = vec4( 0.25, 0, 0, 1 );
    mSlant = -0.2f;
}

SegmentDisplay& SegmentDisplay::position( const vec2 &pos )
{
    mPosition = pos;
    return *this;
}

SegmentDisplay& SegmentDisplay::below( const SegmentDisplay &other )
{
    mPosition = other.mPosition + vec2( 0, other.height() );
    return *this;
}

SegmentDisplay& SegmentDisplay::rightOf( const SegmentDisplay &other )
{
    mPosition = other.mPosition + vec2( other.width(), other.height() - height() );
    return *this;
}

SegmentDisplay& SegmentDisplay::scale( const float &s )
{
    mScale = s;
    return *this;
}

SegmentDisplay& SegmentDisplay::colors( const ColorA &on, const ColorA &off )
{
    mColors[1] = on;
    mColors[0] = off;
    return *this;
}

void SegmentDisplay::setup()
{
    // TODO: would be nice to avoid calling the copy constructor here
    vector<vec3> verts(DISPLAY_VERTS);
    // If there's a slant apply it.
    if ( mSlant != 0.0) {
        // We want to shear from the baseline of the text rather than the top so
        // move the text up perform the shear, then put it back.
        vec2 offset = vec2( 0, DISPLAY_DIMENSIONS.y );
        mat3 transform = translate( shearY( translate( mat3(), offset ), mSlant ), -offset );
        for ( auto v = verts.begin(); v != verts.end(); ++v ) {
            *v = transform * *v;
        }
    }

    // Group the verts in each segment using the bone index so the shader knows
    // how to color them.
    vector<int> segmentBones;
    for ( uint s = 0; s < SEGMENTS; ++s ) {
        for ( uint v = 0; v < VERTS_PER_SEGMENT; ++v ) {
            segmentBones.push_back( s );
        }
    }

    vector<gl::VboMesh::Layout> bufferLayout = {
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::AttribInfo( geom::Attrib::BONE_INDEX, DataType::INTEGER, 1, 0, 0, 0 ) ),
    };
    gl::VboMeshRef mesh = gl::VboMesh::create( verts.size(), GL_TRIANGLES, bufferLayout );
    mesh->bufferAttrib( geom::Attrib::POSITION, verts );
    mesh->bufferAttrib( geom::Attrib::BONE_INDEX, segmentBones );

    std::vector<vec3> characterPosition;
    std::vector<int> segmentValue;

    for ( uint d = 0; d < mDigits; ++d ) {
        characterPosition.push_back( vec3( DISPLAY_DIMENSIONS.x * d, 0, 0 ) );
        segmentValue.push_back( 0 );
    }

    // TODO: See if we can merge these two VBOs of per instance data. Not clear
    // to me if they have to be of the same data type though.
    mInstanceValueVbo = gl::Vbo::create( GL_ARRAY_BUFFER, segmentValue.size() * sizeof( int ), segmentValue.data(), GL_DYNAMIC_DRAW );
    geom::BufferLayout instanceColorLayout;
    instanceColorLayout.append( geom::Attrib::CUSTOM_0, 1, 0, 0, 1 /* per instance */ );
    mesh->appendVbo( instanceColorLayout, mInstanceValueVbo );

    mInstancePositionVbo = gl::Vbo::create( GL_ARRAY_BUFFER, characterPosition.size() * sizeof( vec3 ), characterPosition.data(), GL_STATIC_DRAW );
    geom::BufferLayout instancePositionLayout;
    instancePositionLayout.append( geom::Attrib::CUSTOM_1, 3, 0, 0, 1 /* per instance */ );
    mesh->appendVbo( instancePositionLayout, mInstancePositionVbo );

    auto shader = ci::gl::GlslProg::create(
        ci::app::loadResource( RES_SEGMENT_VERT ),
        ci::app::loadResource( RES_SEGMENT_FRAG )
    );
    shader->uniform( "offColor", mColors[0] );
    shader->uniform( "onColor", mColors[1] );

    mBatch = gl::Batch::create( mesh, shader, {
        { geom::Attrib::CUSTOM_0, "vInstanceValue" },
        { geom::Attrib::CUSTOM_1, "vInstancePosition" },
    } );
}

// TODO: would be good to store the string and only update the VBO when the
// string changes.
SegmentDisplay& SegmentDisplay::display( string s )
{
    int *value = (int*)mInstanceValueVbo->mapReplace();
    for ( uint d = 0, len = s.length(); d < mDigits; ++d ) {
        // When we get to the end of the input, keep going and blank out the
        // rest of the display.
        *value++ = valueOf( d < len ? s[d] : ' ' );
    }
    mInstanceValueVbo->unmap();

    return *this;
}

uint16_t SegmentDisplay::valueOf( const char input )
{
    // If the input is out of range then just return a blank space.
    if ( input < CHAR_OFFSET || input > CHAR_OFFSET + CHAR_LENGTH ) {
        return charPatterns[0];
    }

    return charPatterns[ input - CHAR_OFFSET ];
}

float SegmentDisplay::height() const
{
    return DISPLAY_DIMENSIONS.y * mScale;
}

float SegmentDisplay::width() const
{
    return ( DISPLAY_DIMENSIONS.x * mDigits ) * mScale;
}

void SegmentDisplay::draw() const
{
    gl::ScopedModelMatrix matrixScope;
    gl::translate( mPosition );
    gl::scale( vec2( mScale ) );

    mBatch->drawInstanced( mDigits );
}

