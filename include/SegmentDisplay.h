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
    SegmentDisplay(uint length, const Vec2f position = Vec2f::zero(), float size = 1.0);

    void update(string s);
    void update(string s, const Color on, const Color off);
    void setup();
    void draw();

protected:
    uint mLength;
    Vec2f mPosition;
    float mSize;
    Vec2f mDimensions = Vec2f(18, 22) * mSize;
    Color mOn = Color(1, 0, 0);
    Color mOff = Color(0.25, 0, 0);
    gl::VboMeshRef mMesh;
};

#endif /* defined(__Segments__SegmentDisplay__) */
