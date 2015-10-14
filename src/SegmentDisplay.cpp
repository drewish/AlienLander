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
    mColors[1] = Color( 1, 0, 0 );
    mColors[0] = Color( 0.25, 0, 0 );
    mDimensions = vec2( 18, 22 ) * mScale;
}

void SegmentDisplay::setup()
{
    // coordinates of the points where segments intersect. we need to color
    // segments individually so each one gets its own copies of the vertexes.
    vec3 coords[9] = {
        vec3(  6, 2,0 ) * mScale,
        vec3( 12, 2,0 ) * mScale,
        vec3( 18, 2,0 ) * mScale,
        vec3( 16,12,0 ) * mScale,
        vec3( 14,22,0 ) * mScale,
        vec3(  8,22,0 ) * mScale,
        vec3(  2,22,0 ) * mScale,
        vec3(  4,12,0 ) * mScale,
        vec3( 10,12,0 ) * mScale,
    };

    int totalVertices = 2 * SEGMENTS * mDigits;
	vector<gl::VboMesh::Layout> bufferLayout = {
		gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
		gl::VboMesh::Layout().usage( GL_DYNAMIC_DRAW ).attrib( geom::Attrib::COLOR, 3 )
	};
	mMesh = gl::VboMesh::create( totalVertices, GL_LINES, bufferLayout);

    vec3 offset = vec3(mPosition, 0);
    vector<vec3> verts;
    for ( int i = 0; i < mDigits; i++ ) {
        verts.push_back( coords[0] + offset ); verts.push_back( coords[1] + offset );
        verts.push_back( coords[1] + offset ); verts.push_back( coords[2] + offset );
        verts.push_back( coords[2] + offset ); verts.push_back( coords[3] + offset );
        verts.push_back( coords[3] + offset ); verts.push_back( coords[4] + offset );
        verts.push_back( coords[4] + offset ); verts.push_back( coords[5] + offset );
        verts.push_back( coords[5] + offset ); verts.push_back( coords[6] + offset );
        verts.push_back( coords[6] + offset ); verts.push_back( coords[7] + offset );
        verts.push_back( coords[7] + offset ); verts.push_back( coords[0] + offset );
        verts.push_back( coords[7] + offset ); verts.push_back( coords[8] + offset );
        verts.push_back( coords[8] + offset ); verts.push_back( coords[3] + offset );
        verts.push_back( coords[0] + offset ); verts.push_back( coords[8] + offset );
        verts.push_back( coords[8] + offset ); verts.push_back( coords[1] + offset );
        verts.push_back( coords[8] + offset ); verts.push_back( coords[2] + offset );
        verts.push_back( coords[8] + offset ); verts.push_back( coords[4] + offset );
        verts.push_back( coords[8] + offset ); verts.push_back( coords[5] + offset );
        verts.push_back( coords[8] + offset ); verts.push_back( coords[6] + offset );

        offset += vec3( mDimensions.x, 0, 0 );
    }
    mMesh->bufferAttrib( geom::Attrib::POSITION, verts );

    gl::ScopedLineWidth lineScope(2);
    mBatch = gl::Batch::create( mMesh, gl::getStockShader( gl::ShaderDef().color() ) );
}

SegmentDisplay& SegmentDisplay::colors( const Color &on, const Color &off )
{
    mColors[1] = on;
    mColors[0] = off;

    return *this;
}

SegmentDisplay& SegmentDisplay::display( string s )
{
    char c;
    int len = s.length();

	auto mappedColorAttrib = mMesh->mapAttrib3f( geom::Attrib::COLOR, false );
    for (int j = 0; j < mDigits; ++j) {
        c = ' ';
        // Make sure we're don't go off the end of the string and that the
        // character is one in our table.
        if (j < len && s[j] > CHAR_OFFSET && s[j] < CHAR_OFFSET + CHAR_LENGTH) {
            c = s[j];
        }

        int pattern = charPatterns[(int)c - CHAR_OFFSET];
        for ( int i = 0; i < SEGMENTS; i++ ) {
            vec3 color = (pattern & (1 << i)) ? mColors[1] : mColors[0];
            *mappedColorAttrib = color;
            ++mappedColorAttrib;
            *mappedColorAttrib = color;
            ++mappedColorAttrib;
        }
    }
	mappedColorAttrib.unmap();

    return *this;
}

void SegmentDisplay::draw()
{
    mBatch->draw();
}

