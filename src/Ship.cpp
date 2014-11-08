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
    mAcc = Vec3f(0, 0, -0.00001); // gravity
    mAcc += Vec3f(0, 0, mMainMotor);
    mAcc += mThrusters;

    mVel += mAcc;

    mPos += mVel;
    mPos.z = math<float>::clamp(mPos.z, 0, 1);

    // Consider this landed...
    if (mPos.z <= 0) mVel = Vec3f::zero();
}

float Ship::cameraRotation()
{
    return 1.0 - mPos.z;
}
