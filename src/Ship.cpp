//
//  Ship.cpp
//  AlienLander
//
//  Created by Andrew Morton on 11/7/14.
//
//

#include "Ship.h"

using namespace app;

void Ship::setup()
{
}

void Ship::update()
{
    mAcc = Vec4f(0, 0, -0.00001, 0); // gravity

    // x/y thrusters should be summed up into a vector then apply that
    // considering the ship's heading (stored in mPos.w)
    mAcc += Vec4f(sin(mPos.w) * mThrusters.x, cos(mPos.w) * mThrusters.y, mThrusters.z, mThrusters.w);

    // FIXME:
    Vec3f drag = (mVel.xyz() * -1) * mVel.xyz().lengthSquared() * 500;
    float rotDrag = mVel.w * -1 * (mVel.w * mVel.w) * 500;
    mAcc += Vec4f(drag, rotDrag);

    mVel += mAcc;
    mPos += mVel;

    // Consider this landed...
//    mPos.z = math<float>::clamp(mPos.z, 0, 1);
    if (mPos.z <= 0) {
        mVel = Vec4f::zero();
        mPos.z = 0;
    }
}

void Ship::keyDown( KeyEvent event )
{
    float rotationThrust = 0.0001;
    float lateralThrust = 0.00001;

    switch( event.getCode() ) {
        case KeyEvent::KEY_SPACE:
            mThrusters.z = 0.00008;
            break;
        case KeyEvent::KEY_DOWN:
            mThrusters.y = +lateralThrust;
            break;
        case KeyEvent::KEY_UP:
            mThrusters.y = -lateralThrust;
            break;
        case app::KeyEvent::KEY_LEFT:
            mThrusters.w = rotationThrust;
            break;
        case app::KeyEvent::KEY_RIGHT:
            mThrusters.w = -rotationThrust;
            break;
    }
}

void Ship::keyUp( KeyEvent event )
{
    switch( event.getCode() ) {
        case KeyEvent::KEY_SPACE:
            mThrusters.z = 0;
            break;
        case KeyEvent::KEY_DOWN:
        case KeyEvent::KEY_UP:
            mThrusters.y = 0.0;
            break;
        case KeyEvent::KEY_LEFT:
        case KeyEvent::KEY_RIGHT:
            mThrusters.w = 0.0;
            break;
    }
}
