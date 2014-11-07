//
//  SegmentDisplay.h
//  Segments
//
//  Created by Andrew Morton on 11/6/14.
//
//

#ifndef __Segments__SegmentDisplay__
#define __Segments__SegmentDisplay__

using namespace ci;
using namespace std;

class SegmentDisplay {
public:
    void drawChar(char c, Vec2f offset = Vec2f::zero(), float scale = 1.0);
    Vec2f drawString(string s, Vec2f offset = Vec2f::zero(), float scale = 1.0);
    Vec2f charDimensions(float scale = 1.0);

    Color mOn = Color(1, 0, 0);
    Color mOff = Color(0.25, 0, 0);
};

#endif /* defined(__Segments__SegmentDisplay__) */
