#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Utilities.h"
#include "cinder/Perlin.h"
#include "Resources.h"


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
//    void update();
    void draw();

    Vec2f mDelta1, mDelta2;

    CameraOrtho mCam;
    int mPoints = 32;
    int mLines = 42;
    int mMargin = 20;
    float mX = 0;
    float mY = 0;
    Perlin mPerlin = Perlin(16);
    Channel32f mMap = Channel32f(1024, 1024);
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

    mCam.setOrtho( 0, width, height, 0, -990, 990 );
    gl::setMatrices( mCam );
}

void AlienLanderApp::mouseMove( MouseEvent event )
{
}

void AlienLanderApp::touchesMoved( TouchEvent event )
{
    //  console() << "Moved: " << event << std::endl;
    const vector<TouchEvent::Touch>&touches = event.getTouches();
    if (touches.size() == 2) {
        mDelta1 = touches[0].getPrevPos() - touches[0].getPos();
        mDelta2 = touches[1].getPrevPos() - touches[1].getPos();

        mX += (mDelta1.x + mDelta2.x) / 8;
        mY += (mDelta1.y + mDelta2.y) / 8;
    }
    //  for( vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt )
    //    mActivePoints[touchIt->getId()].addPoint( touchIt->getPos() );
}

void AlienLanderApp::keyDown( KeyEvent event )
{
    float offset = 1;
    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE:
            quit();
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

void AlienLanderApp::draw()
{
    gl::clear( Color::gray(0) );

    gl::pushModelView();

    gl::translate(mMargin, 4 * mMargin);

    float xScale = (getWindowWidth() - (1.0 * mMargin)) / mPoints;
    // sqrt(2) accouts for the 45 degree rotation
    float yScale = (getWindowHeight() * sqrt(2) - (6.0 * mMargin)) / mLines;
    Channel32f::Iter iter = mMap.getIter(Area((int)mX, (int)mY, (int)mX + mPoints, (int)mY + mLines));
    while( iter.line() ) {
        PolyLine<Vec2f> line;
        Shape2d mask;
        Vec2f point;

        gl::pushModelView();

        gl::rotate(Vec3f(45,0,0));
        gl::translate(0, (iter.y() - mY) * yScale, 0);

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

        gl::color( Color::white() );
        gl::lineWidth(2);
        gl::draw(line);

        gl::popModelView();
    }

    /*
     // Save a frame in the home directory.
     if (getElapsedFrames() == 1) {
     writeImage( getHomeDirectory() / "AlienLanderAppOutput.png", copyWindowSurface() );
     }
     */
    gl::popModelView();
}

CINDER_APP_NATIVE( AlienLanderApp, RendererGl )
