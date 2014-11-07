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
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Perlin.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include <boost/format.hpp>
#include "Resources.h"
#include "SegmentDisplay.h"

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
    void update();
    void draw();

    Vec2f mDelta1, mDelta2;

    CameraOrtho mCam;
    int mPoints = 32;
    int mLines = 42;
    int mMargin = 20;
    float mX = 0;
    float mY = 0;

    // Only for vertical:
    float mAcc = 0;
    float mVel = 0;

    float mRatio = 0.5;
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
}

void AlienLanderApp::resize()
{
    int height = getWindowHeight();
    int width = getWindowWidth();
    mMargin = 15;
    mPoints = (width - (2 * mMargin)) / mMargin;
    mLines = (height - (2 * mMargin)) / mMargin;

//    mCam.setOrtho( 0, width, height, 0, -990, 990 );
//    gl::setMatrices( mCam );
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

        mX += (mDelta1.x + mDelta2.x) / 8;
        mY += (mDelta1.y + mDelta2.y) / 8;
    }
}

void AlienLanderApp::keyDown( KeyEvent event )
{
    float offset = 1;
    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE:
            quit();
            break;
        case KeyEvent::KEY_SPACE:
            mAcc += 0.00008;
            break;
        case KeyEvent::KEY_DOWN:
            mY += offset;
            break;
        case KeyEvent::KEY_UP:
            mY -= offset;
            break;
        case KeyEvent::KEY_LEFT:
            mX -= offset;
            break;
        case KeyEvent::KEY_RIGHT:
            mX += offset;
            break;
    }
}

void AlienLanderApp::update()
{
    mAcc += -0.00001; // gravity
    mVel += mAcc;
    mRatio += mVel;
    mRatio = math<float>::clamp(mRatio, 0, 1);

    // Consider this landed...
    if (mRatio <= 0) mVel = 0;

    // HACKY: Render some display info. Better to do these as vectors.
    TextLayout simple;
    simple.setFont( Font( "Arial", 24 ) );
    simple.setColor( Color::white() );
    simple.addLine( string("Acc: ") + to_string(mAcc) );
    simple.addLine( string("Vel: ") + to_string(mVel) );
    simple.addLine( "Alt: " + to_string(mRatio) );
}

void AlienLanderApp::draw()
{

    Color8u blue = Color8u(66, 161, 235);
    Color8u red = Color8u(205, 138, 55);

    gl::clear( Color::gray(0.1) );

    gl::pushModelView();

    gl::translate(mMargin, (1.0 - mRatio) * getWindowHeight() / 2);
    gl::rotate(Vec3f((1.0 - mRatio) * 90,0,0));

    float xScale = (getWindowWidth() - (1.0 * mMargin)) / mPoints;
    float yScale = (getWindowHeight() - (6.0 * mMargin)) / mLines;
    Channel32f::Iter iter = mMap.getIter(Area((int)mX, (int)mY, (int)mX + mPoints, (int)mY + mLines));
    while( iter.line() ) {
        PolyLine<Vec2f> line;
        Shape2d mask;
        Vec2f point;

        gl::pushModelView();

        gl::translate(0, (iter.y() - mY) * yScale, 0);
        gl::rotate(Vec3f((1.0 - mRatio) * -90,0,0));

        point = Vec2f((iter.x() - mX) * xScale, 0);
        mask.moveTo(point);

        while( iter.pixel() ) {
            float val = 200 * pow(1 * iter.v(), 2);
            point = Vec2f((iter.x() - mX) * xScale, -val);
            mask.lineTo( point );
            line.push_back( point );
        }

        point = Vec2f((iter.x() - mX) * xScale, 0);
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
    mDisplay.mOff = Color8u::hex(0x1A3E5A);
    pos.y += 2 + mDisplay.drawString("Acc " + (formatter % mAcc).str() + "m/s/s", pos, 1).y;
    pos.y += 2 + mDisplay.drawString("Vel " + (formatter % mVel).str() + "m/s  ", pos, 1).y;
    pos.y += 2 + mDisplay.drawString("Alt " + (formatter % mRatio).str() + "km   ", pos, 1).y;

    // Reset the acceleration for the next pass
    mAcc = 0;
}

CINDER_APP_NATIVE( AlienLanderApp, RendererGl )
