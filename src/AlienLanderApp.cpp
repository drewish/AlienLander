/*
TODO list:
 -
 - restore hidden line removal
 - pass color through to frag shader
 - Display height over ground
 - Fix rotation so you land at spot that's in the middle of the board at max
   altitude. Be good to put the lines into the field of view instead of hidden
   behind you as you lower.
 - Detect landing/collision
 - Add concept of fuel
 - Use 2 fingers to adjust thrusters
*/
#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Utilities.h"
#include <boost/format.hpp>
#include "Map.h"
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
//    Map mMap;
    SegmentDisplay mDisplay;


    gl::VboMeshRef	mVboMesh;
    gl::TextureRef	mTexture;
    gl::GlslProgRef	mShader;
    CameraPersp     mCamera;
    Matrix44f       mTexTransform;
    float           mZoom = 0.5;
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
    mTexture = gl::Texture::create( loadImage( loadResource( RES_US_SQUARE ) ) );
  }
  catch( ... ) {
    std::cout << "unable to load the texture file!" << std::endl;
  }
  mTexture->setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

  try {
    mShader = gl::GlslProg::create( loadResource( RES_VERT ), loadResource( RES_FRAG ) );
  }
  catch( gl::GlslProgCompileExc &exc ) {
    std::cout << "Shader compile error: " << std::endl;
    std::cout << exc.what();
  }
  catch( ... ) {
    std::cout << "Unable to load shader" << std::endl;
  }

  int totalVertices = mLines * mPoints;
  int totalLines = (mLines - 1) * mPoints;
  gl::VboMesh::Layout layout;
  layout.setStaticIndices();
  layout.setStaticPositions();
  layout.setStaticTexCoords2d();
  mVboMesh = gl::VboMesh::create( totalVertices, totalLines * 2, layout, GL_LINES );

  vector<uint32_t> indices;
  vector<Vec3f> vertCoords;
  vector<Vec2f> texCoords;
  for( int z = 0; z < mPoints; ++z ) {
    for( int x = 1; x < mLines; ++x ) {
      indices.push_back( z * mLines + (x - 1) );
      indices.push_back( z * mLines + (x + 0) );
    }
    for( int x = 0; x < mLines; ++x ) {
      vertCoords.push_back( Vec3f( x / (float)mLines - 0.5, 0.0, z / (float)mPoints - 0.5) );
      texCoords.push_back( Vec2f( x / (float)mLines, z / (float)mPoints ) );
    }
  }

  mVboMesh->bufferIndices( indices );
  mVboMesh->bufferPositions( vertCoords );
  mVboMesh->bufferTexCoords2d( 0, texCoords );

  mCamera.lookAt( Vec3f( 0.0f, 30.0f, 20.0f ), Vec3f(0.0,-5.0,0.0), Vec3f::yAxis() );

    mShip.setup();
    setFullScreen( true );
}

void AlienLanderApp::resize()
{
    int height = getWindowHeight();
    int width = getWindowWidth();
    mMargin = 10;
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

        mShip.mPos.x += (mDelta1.x + mDelta2.x) / 768.0;
        mShip.mPos.y += (mDelta1.y + mDelta2.y) / 768.0;
    }
}

void AlienLanderApp::keyDown( KeyEvent event )
{
    float rotationThrust = 0.0001;
    float lateralThrust = 0.0002;

    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE:
            quit();
            break;
        case KeyEvent::KEY_SPACE:
            mShip.mMainMotor = 0.00008;
            break;
        case KeyEvent::KEY_DOWN:
            mShip.mThrusters.y = +lateralThrust;
            break;
        case KeyEvent::KEY_UP:
            mShip.mThrusters.y = -lateralThrust;
            break;
//        case KeyEvent::KEY_LEFT:
//            mShip.mThrusters.x = -offset;
//            break;
//        case KeyEvent::KEY_RIGHT:
//            mShip.mThrusters.x = +offset;
//            break;

//        case KeyEvent::KEY_DOWN:
//            mShip.mPos += Vec3f(vec, 0);
//            break;
//        case KeyEvent::KEY_UP:
//            mShip.mPos -= Vec3f(vec, 0);
//            break;
        case app::KeyEvent::KEY_LEFT:
            mShip.mThrusters.w = rotationThrust;
            break;
        case app::KeyEvent::KEY_RIGHT:
            mShip.mThrusters.w = -rotationThrust;
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
            mShip.mThrusters.y = 0.0;
            break;
        case KeyEvent::KEY_UP:
            mShip.mThrusters.y = 0.0;
            break;
//        case KeyEvent::KEY_LEFT:
//            mShip.mThrusters.x = 0;
//            break;
//        case KeyEvent::KEY_RIGHT:
//            mShip.mThrusters.x = 0;
//            break;
        case KeyEvent::KEY_LEFT:
            mShip.mThrusters.w = 0.0;
            break;
        case KeyEvent::KEY_RIGHT:
            mShip.mThrusters.w = 0.0;
            break;
    }
}

void AlienLanderApp::update()
{
    mShip.update();

    Matrix44f center = Matrix44f(1, 0, 0, +0.5,
                                 0, 1, 0, +0.5,
                                 0, 0, 1, 0,
                                 0, 0, 0, 1);
    Matrix44f goback = Matrix44f(1, 0, 0, -0.5,
                                 0, 1, 0, -0.5,
                                 0, 0, 1, 0,
                                 0, 0, 0, 1);
    Matrix44f r = Matrix44f::createRotation(Vec3f::zAxis(), mShip.mPos.w);
    Matrix44f s = Matrix44f::createScale(mZoom);
    Matrix44f t = Matrix44f(1, 0, 0, mShip.mPos.x,
                            0, 1, 0, mShip.mPos.y,
                            0, 0, 1, 0,
                            0, 0, 0, 1);

    mTexTransform = goback * r * t * s * center;
}

void AlienLanderApp::draw()
{
    gl::pushMatrices();

    Color8u blue = Color8u(66, 161, 235);
    Color8u darkBlue = Color8u::hex(0x1A3E5A);

    gl::clear( Color::black() ); // Color::gray(0.1)


    gl::lineWidth(2);
    Vec2f pos = Vec2f(2, 2);
    boost::format formatter("%+05f");
    mDisplay.mOn = blue;
    mDisplay.mOff = darkBlue;
    pos.y += 2 + mDisplay.drawString("Pos " + (formatter % mShip.mPos).str(), pos, 1.0).y;
    pos.y += 2 + mDisplay.drawString("Acc " + (formatter % mShip.mAcc).str(), pos, 1.0).y;
    pos.y += 2 + mDisplay.drawString("Vel " + (formatter % mShip.mVel).str(), pos, 1.0).y;
//  pos.y += 2 + mDisplay.drawString("Alt " + (formatter % mShip.mPos.z).str() + "km   ", pos, 0.75).y;
//  pos.y += 2 + mDisplay.drawString("Alt " + (formatter % (mShip.mPos.z - mMap.valueAt((int)shipPos.x, (int)shipPos.y))).str() + "km   ", pos, 0.75).y;

    gl::setMatrices( mCamera );

    gl::lineWidth(1);

    Vec2f vec = Vec2f(cos(mShip.mPos.w), sin(mShip.mPos.w)) * 10;
    gl::drawVector(Vec3f(0.0,5.0,0.0), Vec3f(vec.x, 5.0, vec.y));

    mTexture->enableAndBind();
    mShader->bind();
    mShader->uniform( "tex0", 0 );
    mShader->uniform( "texTransform", mTexTransform );
    mShader->uniform( "zoom", mZoom );

    gl::scale(2* Vec3f( 10, 10, 10 ) );
    gl::draw( mVboMesh );

    mShader->unbind();
    mTexture->unbind();
////    gl::translate(mMargin, (mShip.mPos.z) * getWindowHeight() / 2);
////    gl::rotate(Vec3f(mShip.cameraRotation() * 90,0,0));
//    gl::translate(0, 0, -scaleZ(mShip.mPos.z));
//    gl::rotate(Vec3f(45,0,0));
//
//    float xScale = (getWindowWidth() - (1.0 * mMargin)) / mPoints;
//    float yScale = (getWindowHeight() - (6.0 * mMargin)) / mLines;
//    Vec3f shipPos = mShip.mPos;
//    Channel32f::Iter iter = mMap.viewFrom(shipPos, Vec2i(mPoints, mLines));
//    while( iter.line() ) {
//        PolyLine<Vec2f> line;
//        Shape2d mask;
//        Vec2f point;
//
//        gl::pushModelView();
//
//        gl::translate(0, (iter.y() - shipPos.y) * yScale, 0);
//        gl::rotate(Vec3f(-45,0,0));
//
//        point = Vec2f((iter.x() - shipPos.x) * xScale, 0);
//        mask.moveTo(point);
//
//        while( iter.pixel() ) {
//            float val = scaleZ(iter.v());
//            point = Vec2f((iter.x() - shipPos.x) * xScale, -val);
//            mask.lineTo( point );
//            line.push_back( point );
//        }
//
//        point = Vec2f((iter.x() - shipPos.x) * xScale, 0);
//        mask.lineTo(point);
//        mask.close();
//
//        gl::color( Color::black() );
//        gl::drawSolid(mask);
//
//        gl::color(blue);
//        gl::lineWidth(2);
//        gl::draw(line);
//
//        gl::popModelView();
//    }
//


    gl::popMatrices();
}

CINDER_APP_NATIVE( AlienLanderApp, RendererGl )
