//
//  Ship.cpp
//  AlienLander
//
//  Created by Andrew Morton on 11/7/14.
//
//

#include "Ship.h"


void Ship::setup()
{
}

void Ship::update()
{
    mAcc = Vec4f(0, 0, -0.00001, 0); // gravity
    mAcc += Vec4f(0, 0, mMainMotor, 0);

    // x/y thrusters should be summed up into a vector then apply that
    // considering the ship's heading (stored in mPos.w)
    //Vec2f vec = Vec2f(sin(mShip.mPos.w), cos(mShip.mPos.w)) * 0.009;
    mAcc += mThrusters;

    // FIXME:
    Vec3f drag = (mVel.xyz() * -1) * mVel.xyz().lengthSquared() * 5;
    float rotDrag = mVel.w * -1 * (mVel.w * mVel.w) * 50;
    mAcc += Vec4f(drag, rotDrag);

    mVel += mAcc;
    mPos += mVel;

    // Consider this landed...
//    mPos.z = math<float>::clamp(mPos.z, 0, 1);
//    if (mPos.z <= 0) mVel = Vec3f::zero();
}

float Ship::cameraRotation()
{
    return 1.0 - mPos.z;
}
