/*
TODO list:
- Fix rotation so you land at spot that's in the middle of the board at max 
  altitude.
- Write code for drawing numbers
- Display height over ground
- Detect landing/collision
- Use thrusters for horizontal movement
- Add concept of fuel
*/
#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/Texture.h"
#include "cinder/Perlin.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include <boost/format.hpp>
#include "Resources.h"
#include "SegmentDisplay.h"
#include "Ship.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AlienLanderApp : public AppNative {
public:
    void prepareSettings( Settings *settings ) override;
    void setup();
    void resize();
    void mouseMove( MouseEvent event );
    void touchesMoved( TouchEvent event );
    void keyDown( KeyEvent event );
    void keyUp( KeyEvent event );
    void update();
    void draw();

    Vec2f mDelta1, mDelta2;

    int mPoints = 32;
    int mLines = 42;
    int mMargin = 20;

    Ship mShip;

    Perlin mPerlin = Perlin(16);
    Channel32f mMap = Channel32f(1024, 1024);
    SegmentDisplay mDisplay;
};

void AlienLanderApp::prepareSettings( Settings *settings )
{
    settings->enableMultiTouch();
    settings->enableHighDensityDisplay();
    settings->setWindowSize(800, 800);
}

void AlienLanderApp::setup()
{
    try {
        mMap = Channel32f(loadImage(loadResource(RES_MAP)));
    }
    catch( ... ) {
        console() << "unable to load the texture file!" << std::endl;
        // fall back to perlin noise
        Channel32f::Iter iter = mMap.getIter();
        while( iter.line() ) {
            while( iter.pixel() ) {
                float noise = (0.5 + mPerlin.fBm(iter.x() * 0.1, iter.y() * 0.1, 0.52));
                iter.v() = noise;
            }
        }
    };

    mShip.setup();
}

void AlienLanderApp::resize()
{
    int height = getWindowHeight();
    int width = getWindowWidth();
    mMargin = 15;
    mPoints = (width - (2 * mMargin)) / mMargin;
    mLines = (height - (2 * mMargin)) / mMargin;
}

void AlienLanderApp::mouseMove( MouseEvent event )
{
//    int height = getWindowHeight();
//    mRatio = 1 - (math<float>::clamp(event.getY(), 0, height) / height);
}

void AlienLanderApp::touchesMoved( TouchEvent event )
{
    const vector<TouchEvent::Touch>&touches = event.getTouches();
    if (touches.size() == 2) {
        mDelta1 = touches[0].getPrevPos() - touches[0].getPos();
        mDelta2 = touches[1].getPrevPos() - touches[1].getPos();

        mShip.mPos.x += (mDelta1.x + mDelta2.x) / 8;
        mShip.mPos.y += (mDelta1.y + mDelta2.y) / 8;
    }
}

void AlienLanderApp::keyDown( KeyEvent event )
{
    float offset = 0.0001;
    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE:
            quit();
            break;
        case KeyEvent::KEY_SPACE:
            mShip.mMainMotor = 0.00008;
            break;
        case KeyEvent::KEY_DOWN:
            mShip.mThrusters.y = +offset;
            break;
        case KeyEvent::KEY_UP:
            mShip.mThrusters.y = -offset;
            break;
        case KeyEvent::KEY_LEFT:
            mShip.mThrusters.x = -offset;
            break;
        case KeyEvent::KEY_RIGHT:
            mShip.mThrusters.x = +offset;
            break;
    }
}

void AlienLanderApp::keyUp( KeyEvent event )
{
    switch( event.getCode() ) {
        case KeyEvent::KEY_SPACE:
            mShip.mMainMotor = 0;
            break;
        case KeyEvent::KEY_DOWN:
            mShip.mThrusters.y = 0;
            break;
        case KeyEvent::KEY_UP:
            mShip.mThrusters.y = 0;
            break;
        case KeyEvent::KEY_LEFT:
            mShip.mThrusters.x = 0;
            break;
        case KeyEvent::KEY_RIGHT:
            mShip.mThrusters.x = 0;
            break;
    }
}

void AlienLanderApp::update()
{
    mShip.update();
}

void AlienLanderApp::draw()
{
    Color8u blue = Color8u(66, 161, 235);
    Color8u darkBlue = Color8u::hex(0x1A3E5A);

    gl::clear( Color::gray(0.1) );

    gl::pushModelView();

    gl::translate(mMargin, (mShip.cameraRotation()) * getWindowHeight() / 2);
    gl::rotate(Vec3f(mShip.cameraRotation() * 90,0,0));

    float xScale = (getWindowWidth() - (1.0 * mMargin)) / mPoints;
    float yScale = (getWindowHeight() - (6.0 * mMargin)) / mLines;
    Vec3f shipPos = mShip.mPos;
    Channel32f::Iter iter = mMap.getIter(Area((int)shipPos.x, (int)shipPos.y, (int)shipPos.x + mPoints, (int)shipPos.y + mLines));
    while( iter.line() ) {
        PolyLine<Vec2f> line;
        Shape2d mask;
        Vec2f point;

        gl::pushModelView();

        gl::translate(0, (iter.y() - shipPos.y) * yScale, 0);
        gl::rotate(Vec3f(mShip.cameraRotation() * -90,0,0));

        point = Vec2f((iter.x() - shipPos.x) * xScale, 0);
        mask.moveTo(point);

        while( iter.pixel() ) {
            float val = 200 * pow(1 * iter.v(), 2);
            point = Vec2f((iter.x() - shipPos.x) * xScale, -val);
            mask.lineTo( point );
            line.push_back( point );
        }

        point = Vec2f((iter.x() - shipPos.x) * xScale, 0);
        mask.lineTo(point);
        mask.close();

        gl::color( Color::black() );
        gl::drawSolid(mask);

        gl::color(blue);
        gl::lineWidth(2);
        gl::draw(line);

        gl::popModelView();
    }


    gl::popModelView();

    gl::lineWidth(4);
    Vec2f pos = Vec2f(2, 2);
    boost::format formatter("%+05f");
    mDisplay.mOn = blue;
    mDisplay.mOff = darkBlue;
    pos.y += 2 + mDisplay.drawString("Acc " + (formatter % mShip.mAcc).str() + "m/s/s", pos, 1).y;
    pos.y += 2 + mDisplay.drawString("Vel " + (formatter % mShip.mVel).str() + "m/s  ", pos, 1).y;
    pos.y += 2 + mDisplay.drawString("Alt " + (formatter % mShip.mPos.z).str() + "km   ", pos, 1).y;
}

CINDER_APP_NATIVE( AlienLanderApp, RendererGl )
