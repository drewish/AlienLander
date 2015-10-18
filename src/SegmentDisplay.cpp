//
//  SegmentDisplay.cpp
//  Segments
//
//  Created by Andrew Morton on 11/6/14.
//
//

#include "SegmentDisplay.h"

using namespace ci;
using namespace ci::geom;
using namespace std;

const uint SEGMENTS = 16;
const uint VERTS_PER_SEGMENT = 6;
// Awesome font stolen from http://www.msarnoff.org/alpha32/
// The first 32, non-printable ASCII characters are omitted.
const uint CHAR_OFFSET = 32;
const uint CHAR_LENGTH = 96;
int charPatterns[CHAR_LENGTH] = {
    0x0000,  /*   */
    0x1822,  /* ! */
    0x0880,  /* " */
    0x4b3c,  /* # */
    0x4bbb,  /* $ */
    0xdb99,  /* % */
    0x2d79,  /* & */
    0x1000,  /* ' */
    0x3000,  /* ( */
    0x8400,  /* ) */
    0xff00,  /* * */
    0x4b00,  /* + */
    0x8000,  /* , */
    0x0300,  /* - */
    0x0020,  /* . */
    0x9000,  /* / */
    0x90ff,  /* 0 */
    0x100c,  /* 1 */
    0x0377,  /* 2 */
    0x123b,  /* 3 */
    0x038c,  /* 4 */
    0x21b3,  /* 5 */
    0x03fb,  /* 6 */
    0x000f,  /* 7 */
    0x03ff,  /* 8 */
    0x03bf,  /* 9 */
    0x0021,  /* : */
    0x8001,  /* ; */
    0x9030,  /* < */
    0x0330,  /* = */
    0x2430,  /* > */
    0x4207,  /* ? */
    0x417f,  /* @ */
    0x03cf,  /* A */
    0x4a3f,  /* B */
    0x00f3,  /* C */
    0x483f,  /* D */
    0x01f3,  /* E */
    0x01c3,  /* F */
    0x02fb,  /* G */
    0x03cc,  /* H */
    0x4833,  /* I */
    0x4863,  /* J */
    0x31c0,  /* K */
    0x00f0,  /* L */
    0x14cc,  /* M */
    0x24cc,  /* N */
    0x00ff,  /* O */
    0x03c7,  /* P */
    0x20ff,  /* Q */
    0x23c7,  /* R */
    0x03bb,  /* S */
    0x4803,  /* T */
    0x00fc,  /* U */
    0x90c0,  /* V */
    0xa0cc,  /* W */
    0xb400,  /* X */
    0x5400,  /* Y */
    0x9033,  /* Z */
    0x00e1,  /* [ */
    0x2400,  /* \ */
    0x001e,  /* ] */
    0xa000,  /* ^ */
    0x0030,  /* _ */
    0x0400,  /* ` */
    0x4170,  /* a */
    0x41e0,  /* b */
    0x0160,  /* c */
    0x4960,  /* d */
    0x8160,  /* e */
    0x4b02,  /* f */
    0x2238,  /* g */
    0x41c0,  /* h */
    0x4000,  /* i */
    0x4020,  /* j */
    0x6a00,  /* k */
    0x4811,  /* l */
    0x4348,  /* m */
    0x4140,  /* n */
    0x4160,  /* o */
    0x09c1,  /* p */
    0x4991,  /* q */
    0x0140,  /* r */
    0x4220,  /* s */
    0x4b10,  /* t */
    0x4060,  /* u */
    0x8040,  /* v */
    0x4078,  /* w */
    0xd800,  /* x */
    0x2038,  /* y */
    0x8120,  /* z */
    0x4912,  /* { */
    0x4800,  /* | */
    0x4a21,  /* } */
    0x0a85,  /* ~ */
    0x0000,  /* DEL */
};

SegmentDisplay::SegmentDisplay(uint length, const vec2 &pos, float size)
    : mDigits(length), mPosition(pos), mScale(size)
{
    mColors[1] = vec4( 1, 0, 0, 1 );
    mColors[0] = vec4( 0.25, 0, 0, 1 );
    mDimensions = vec2( 16, 24 );
    mSlant = -0.2f;
}

void SegmentDisplay::setup()
{
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
    // We need to color segments individually so we don't share vertexes between
    // them.

    int totalVertices = VERTS_PER_SEGMENT * SEGMENTS * mDigits;
	vector<gl::VboMesh::Layout> bufferLayout = {
		gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
		gl::VboMesh::Layout().usage( GL_DYNAMIC_DRAW ).attrib( geom::Attrib::COLOR, 4 )
	};
	mMesh = gl::VboMesh::create( totalVertices, GL_TRIANGLE_STRIP, bufferLayout);

    mat3 transform = scale( mat3(), vec2( mScale ) );
    if ( mSlant != 0.0) {
        // We want to shear from the baseline of the text rather than the top so
        // move the text up perform the shear, then put it back.
        transform = translate( transform, vec2( 0, mDimensions.y ) );
        transform = shearY( transform, mSlant );
        transform = translate( transform, vec2( 0, -mDimensions.y ) );
    }
    transform = translate( transform, mPosition );

    vector<vec3> verts;
    for ( int i = 0; i < mDigits; i++ ) {
        // Segment 0
        verts.push_back( transform * vec3(  1,  1, 1 ) );
        verts.push_back( transform * vec3(  2,  2, 1 ) );
        verts.push_back( transform * vec3(  2,  0, 1 ) );
        verts.push_back( transform * vec3(  6,  2, 1 ) );
        verts.push_back( transform * vec3(  6,  0, 1 ) );
        verts.push_back( transform * vec3(  7,  1, 1 ) );

        // Segment 1 (Segment 0 + vec(6, 0))
        verts.push_back( transform * vec3(  7,  1, 1 ) );
        verts.push_back( transform * vec3(  8,  2, 1 ) );
        verts.push_back( transform * vec3(  8,  0, 1 ) );
        verts.push_back( transform * vec3( 12,  2, 1 ) );
        verts.push_back( transform * vec3( 12,  0, 1 ) );
        verts.push_back( transform * vec3( 13,  1, 1 ) );

        // Segment 2
        verts.push_back( transform * vec3( 13,  1, 1 ) );
        verts.push_back( transform * vec3( 12,  2, 1 ) );
        verts.push_back( transform * vec3( 14,  2, 1 ) );
        verts.push_back( transform * vec3( 12, 10, 1 ) );
        verts.push_back( transform * vec3( 14, 10, 1 ) );
        verts.push_back( transform * vec3( 13, 11, 1 ) );

        // Segment 3 (Segment 2 + vec(0, 10))
        verts.push_back( transform * vec3( 13, 11, 1 ) );
        verts.push_back( transform * vec3( 12, 12, 1 ) );
        verts.push_back( transform * vec3( 14, 12, 1 ) );
        verts.push_back( transform * vec3( 12, 20, 1 ) );
        verts.push_back( transform * vec3( 14, 20, 1 ) );
        verts.push_back( transform * vec3( 13, 21, 1 ) );

        // Segment 4 (Segment 0 + vec(6, 20))
        verts.push_back( transform * vec3(  7, 21, 1 ) );
        verts.push_back( transform * vec3(  8, 22, 1 ) );
        verts.push_back( transform * vec3(  8, 20, 1 ) );
        verts.push_back( transform * vec3( 12, 22, 1 ) );
        verts.push_back( transform * vec3( 12, 20, 1 ) );
        verts.push_back( transform * vec3( 13, 21, 1 ) );

        // Segment 5 (Segment 0 + vec(0, 20))
        verts.push_back( transform * vec3(  1, 21, 1 ) );
        verts.push_back( transform * vec3(  2, 22, 1 ) );
        verts.push_back( transform * vec3(  2, 20, 1 ) );
        verts.push_back( transform * vec3(  6, 22, 1 ) );
        verts.push_back( transform * vec3(  6, 20, 1 ) );
        verts.push_back( transform * vec3(  7, 21, 1 ) );

        // Segment 6 (Segment 2 + vec(-12, -10))
        verts.push_back( transform * vec3(  1, 11, 1 ) );
        verts.push_back( transform * vec3(  0, 12, 1 ) );
        verts.push_back( transform * vec3(  2, 12, 1 ) );
        verts.push_back( transform * vec3(  0, 20, 1 ) );
        verts.push_back( transform * vec3(  2, 20, 1 ) );
        verts.push_back( transform * vec3(  1, 21, 1 ) );

        // Segment 7 (Segment 2 + vec(-12, 0)
        verts.push_back( transform * vec3(  1,  1, 1 ) );
        verts.push_back( transform * vec3(  0,  2, 1 ) );
        verts.push_back( transform * vec3(  2,  2, 1 ) );
        verts.push_back( transform * vec3(  0, 10, 1 ) );
        verts.push_back( transform * vec3(  2, 10, 1 ) );
        verts.push_back( transform * vec3(  1, 11, 1 ) );

        // Segment 8 (Segment 0 + vec(0, 10))
        verts.push_back( transform * vec3(  1, 11, 1 ) );
        verts.push_back( transform * vec3(  2, 12, 1 ) );
        verts.push_back( transform * vec3(  2, 10, 1 ) );
        verts.push_back( transform * vec3(  6, 12, 1 ) );
        verts.push_back( transform * vec3(  6, 10, 1 ) );
        verts.push_back( transform * vec3(  7, 11, 1 ) );

        // Segment 9 (Segment 0 + vec(6, 10))
        verts.push_back( transform * vec3(  7, 11, 1 ) );
        verts.push_back( transform * vec3(  8, 12, 1 ) );
        verts.push_back( transform * vec3(  8, 10, 1 ) );
        verts.push_back( transform * vec3( 12, 12, 1 ) );
        verts.push_back( transform * vec3( 12, 10, 1 ) );
        verts.push_back( transform * vec3( 13, 11, 1 ) );

        // Segment A
        verts.push_back( transform * vec3( 2.0,  2.0, 1 ) );
        verts.push_back( transform * vec3( 2.0,  4.3, 1 ) );
        verts.push_back( transform * vec3( 3.0,  2.0, 1 ) );
        verts.push_back( transform * vec3( 5.3, 10.0, 1 ) );
        verts.push_back( transform * vec3( 6.0,  7.2, 1 ) );
        verts.push_back( transform * vec3( 6.0, 10.0, 1 ) );

        // Segment B (Segment 2 + vec(-6, 0))
        verts.push_back( transform * vec3(  7,  1, 1 ) );
        verts.push_back( transform * vec3(  6,  2, 1 ) );
        verts.push_back( transform * vec3(  8,  2, 1 ) );
        verts.push_back( transform * vec3(  6, 10, 1 ) );
        verts.push_back( transform * vec3(  8, 10, 1 ) );
        verts.push_back( transform * vec3(  7, 11, 1 ) );

        // Segment C
        verts.push_back( transform * vec3(  8.0, 10.0, 1 ) );
        verts.push_back( transform * vec3(  8.7, 10.0, 1 ) );
        verts.push_back( transform * vec3(  8.0,  7.2, 1 ) );
        verts.push_back( transform * vec3( 12.0,  4.3, 1 ) );
        verts.push_back( transform * vec3( 11.0,  2.0, 1 ) );
        verts.push_back( transform * vec3( 12.0,  2.0, 1 ) );

        // Segment D
        verts.push_back( transform * vec3(  8.0, 12.0, 1 ) );
        verts.push_back( transform * vec3(  8.0, 14.8, 1 ) );
        verts.push_back( transform * vec3(  8.7, 12.0, 1 ) );
        verts.push_back( transform * vec3( 11.0, 20.0, 1 ) );
        verts.push_back( transform * vec3( 12.0, 17.7, 1 ) );
        verts.push_back( transform * vec3( 12.0, 20.0, 1 ) );

        // Segment E (Segment 2 + vec(-6, 10))
        verts.push_back( transform * vec3(  7, 11, 1 ) );
        verts.push_back( transform * vec3(  6, 12, 1 ) );
        verts.push_back( transform * vec3(  8, 12, 1 ) );
        verts.push_back( transform * vec3(  6, 20, 1 ) );
        verts.push_back( transform * vec3(  8, 20, 1 ) );
        verts.push_back( transform * vec3(  7, 21, 1 ) );

        // Segment F
        verts.push_back( transform * vec3( 2.0, 20.0, 1 ) );
        verts.push_back( transform * vec3( 3.0, 20.0, 1 ) );
        verts.push_back( transform * vec3( 2.0, 17.7, 1 ) );
        verts.push_back( transform * vec3( 6.0, 14.8, 1 ) );
        verts.push_back( transform * vec3( 5.3, 12.0, 1 ) );
        verts.push_back( transform * vec3( 6.0, 12.0, 1 ) );

        transform = translate( transform, vec2( mDimensions.x, 0 ) );
    }
    mMesh->bufferAttrib( geom::Attrib::POSITION, verts );

    auto shader = gl::getStockShader( gl::ShaderDef().color() );
    mBatch = gl::Batch::create( mMesh, shader );
}

SegmentDisplay& SegmentDisplay::colors( const ColorA &on, const ColorA &off )
{
    mColors[1] = on;
    mColors[0] = off;

    return *this;
}

// TODO: would be good to store the string and only update the VBO when the
// string changes.
SegmentDisplay& SegmentDisplay::display( string s )
{
    char c;
    int len = s.length();

	auto mappedColorAttrib = mMesh->mapAttrib4f( geom::Attrib::COLOR, false );
    for ( int j = 0; j < mDigits; ++j ) {
        c = ' ';
        // Make sure we're don't go off the end of the string and that the
        // character is one in our table.
        if ( j < len && s[j] > CHAR_OFFSET && s[j] < CHAR_OFFSET + CHAR_LENGTH ) {
            c = s[j];
        }

        int pattern = charPatterns[(int)c - CHAR_OFFSET];
        for ( uint i = 0; i < SEGMENTS; ++i ) {
            vec4 color = (pattern & (1 << i)) ? mColors[1] : mColors[0];
            for ( uint vert = 0; vert < VERTS_PER_SEGMENT; ++vert ) {
                *mappedColorAttrib = vec4(color);
                ++mappedColorAttrib;
            }
        }
    }
	mappedColorAttrib.unmap();

    return *this;
}

void SegmentDisplay::draw()
{
    for ( uint start = 0; start < mDigits * SEGMENTS * VERTS_PER_SEGMENT; start += VERTS_PER_SEGMENT ) {
        mBatch->draw( start, VERTS_PER_SEGMENT );
    }
}

