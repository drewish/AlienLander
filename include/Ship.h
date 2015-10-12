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

    vec4 mThrusters = vec4();
    vec4 mAcc = vec4();
    vec4 mVel = vec4();
    vec4 mPos = vec4(0.0, 0.0, 1, 0.0);
};

#endif /* defined(__AlienLander__Ship__) */
