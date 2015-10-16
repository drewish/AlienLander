//
//  SegmentDisplay.h
//  Segments
//
//  Created by Andrew Morton on 11/6/14.
//
//

#ifndef __Segments__SegmentDisplay__
#define __Segments__SegmentDisplay__

#include "cinder/gl/Vbo.h"

using namespace ci;
using namespace std;

class SegmentDisplay {
public:
    SegmentDisplay( uint digits, const ci::vec2 &position = ci::vec2(), float size = 1.0 );

    void setup();
    SegmentDisplay& colors( const ColorA &on, const ColorA &off );
    SegmentDisplay& display( string s );
    void draw();

protected:
    uint            mDigits; // Number of characters in display
    ci::vec2        mPosition;
    float           mScale;
    ci::vec2        mDimensions;
    ci::vec4        mColors[2];
    gl::VboMeshRef  mMesh;
    gl::BatchRef    mBatch;
};

#endif /* defined(__Segments__SegmentDisplay__) */
