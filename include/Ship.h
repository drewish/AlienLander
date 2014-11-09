//
//  Ship.h
//  AlienLander
//
//  Created by Andrew Morton on 11/7/14.
//
//

#ifndef __AlienLander__Ship__
#define __AlienLander__Ship__

using namespace ci;
using namespace std;

class Ship {
public:
    void setup();
    void update();
    float cameraRotation();

    Vec3f mThrusters = Vec3f::zero();
    Vec3f mAcc = Vec3f::zero();
    Vec3f mVel = Vec3f::zero();
    Vec3f mPos = Vec3f(10, 10, 0.5);

    float mFuel = 1000;

    float mMainMotor = 0;
};

#endif /* defined(__AlienLander__Ship__) */
