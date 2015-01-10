//
//  Ship.h
//  AlienLander
//
//  Created by Andrew Morton on 11/7/14.
//
//

#ifndef __AlienLander__Ship__
#define __AlienLander__Ship__

#include "cinder/app/KeyEvent.h"

using namespace ci;

class Ship {
public:
    void setup();
    void update();
    void keyDown( app::KeyEvent event );
    void keyUp( app::KeyEvent event );

    Vec4f mThrusters = Vec4f::zero();
    Vec4f mAcc = Vec4f::zero();
    Vec4f mVel = Vec4f::zero();
    Vec4f mPos = Vec4f(0.5, 0.5, 1, 0.0);
};

#endif /* defined(__AlienLander__Ship__) */
