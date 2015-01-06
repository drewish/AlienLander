/*
TODO list:
 - standardize on either Y or Z axis for heights
 - fix point of rotation, should move towards closer edge as you descend
 - get zoom scaling from view point rather than edge of texture
 - optimize text display to use VBO
 - Use touch for scaling/rotation/panning
 - Detect landing/collision
 - Compute/Display height over ground
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
    void buildMeshes();
    void resize();
    void mouseMove( MouseEvent event );
    void touchesMoved( TouchEvent event );
    void keyDown( KeyEvent event );
    void keyUp( KeyEvent event );
    void update();
    void draw();


    Vec2f mDelta1, mDelta2;

    int mPoints = 21;
    int mLines = 40;

    Ship mShip;
//    Map mMap;
    SegmentDisplay mDisplay;


    gl::VboMeshRef	mMaskMesh;
    gl::VboMeshRef	mLineMesh;
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

    buildMeshes();

    mShip.setup();

    setFullScreen( true );
    setFrameRate(60);

    gl::enableVerticalSync(false);

    mCamera.setPerspective( 35.0f, 1.0f, 10.0f, 100.0f );
}

void AlienLanderApp::buildMeshes()
{
    int totalVertices = mPoints * mLines;
    int totalIndicies = mPoints * mLines;
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    mLineMesh = gl::VboMesh::create( totalVertices, totalIndicies, layout, GL_LINE_STRIP );

    vector<uint32_t> indices;
    vector<Vec3f> vertCoords;
    vector<Vec2f> texCoords;
    for( int z = 0; z < mLines; ++z ) {
        for( int x = 0; x < mPoints; ++x ) {
            vertCoords.push_back( Vec3f( x / (float)mPoints - 0.5, 0.0, z / (float)mLines - 0.5) );
            texCoords.push_back( Vec2f( x / (float)mPoints, z / (float)mLines ) );
            indices.push_back( z * mPoints + (x + 0) );
        }
    }

    mLineMesh->bufferIndices( indices );
    mLineMesh->bufferPositions( vertCoords );
    mLineMesh->bufferTexCoords2d( 0, texCoords );

    // * * *

    totalVertices = mLines * mPoints * 2;
    totalIndicies = mLines * mPoints * 2;
    gl::VboMesh::Layout maskLayout;
    maskLayout.setStaticIndices();
    maskLayout.setStaticPositions();
    maskLayout.setStaticTexCoords2d();
    mMaskMesh = gl::VboMesh::create( totalVertices, totalIndicies, maskLayout, GL_TRIANGLE_STRIP );

    indices.clear();
    vertCoords.clear();
    texCoords.clear();

    for( int z = 0; z < mLines; ++z ) {
        for( int x = 0; x < mPoints; ++x ) {
            Vec3f vert = Vec3f( x / (float)mPoints - 0.5, 0.0, z / (float)mLines - 0.5);
            Vec2f coord = Vec2f( x / (float)mPoints, z / (float)mLines );

            vertCoords.push_back(vert);
            // the vertex shader just uses the texture value as an offset to
            // the y value so setting one below the first value will create a
            // skewed strip to mask the lines behind it.
            vert.y = -0.5;
            vertCoords.push_back(vert);

            texCoords.push_back( coord );
            texCoords.push_back( coord );
        }
        for( int x = 1; x <= mPoints * 2; x += 2 ) {
            indices.push_back( z * 2 * mPoints + x - 1 );
            indices.push_back( z * 2 * mPoints + x - 0 );
        }
    }

    mMaskMesh->bufferIndices( indices );
    mMaskMesh->bufferPositions( vertCoords );
    mMaskMesh->bufferTexCoords2d( 0, texCoords );
}

void AlienLanderApp::resize()
{
    int height = getWindowHeight();
    int width = getWindowWidth();
    int margin = 20;
    mPoints = (width - (2 * margin)) / 10;
    mLines = (height - (2 * margin)) / 25;
    buildMeshes();
}

void AlienLanderApp::update()
{
    mShip.update();

//    mZoom = math<float>::clamp(mShip.mPos.z, 0.0, 1.0);

    // TODO: Need to change the focus point to remain parallel as we descend
    mCamera.lookAt( Vec3f( 0.0f, 30.0f * mShip.mPos.z, 20.0f ), Vec3f(0.0,0.0,0.0), Vec3f::yAxis() );

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
    Color8u black = Color::black();
    Color8u blue = Color8u(66, 161, 235);
    Color8u darkBlue = Color8u::hex(0x1A3E5A);

    gl::pushMatrices();

    gl::enableDepthRead( true );
    gl::enableDepthWrite( true );
    gl::clear( black, true );

    gl::lineWidth(2);
    Vec2f pos = Vec2f(2, 2);
    boost::format formatter("%+05f");
    mDisplay.mOn = (getAverageFps() > 30) ? blue : Color8u(240,0,0);
    mDisplay.mOff = darkBlue;
//    pos.y += 2 + mDisplay.drawString("Pos " + (formatter % mShip.mPos).str(), pos, 1.0).y;
//    pos.y += 2 + mDisplay.drawString("Acc " + (formatter % mShip.mAcc).str(), pos, 1.0).y;
//    pos.y += 2 + mDisplay.drawString("Vel " + (formatter % mShip.mVel).str(), pos, 1.0).y;
    mDisplay.drawString("FPS " + (formatter % getAverageFps()).str(), pos, 1.0).y;
//  pos.y += 2 + mDisplay.drawString("Alt " + (formatter % mShip.mPos.z).str() + "km   ", pos, 0.75).y;
//  pos.y += 2 + mDisplay.drawString("Alt " + (formatter % (mShip.mPos.z - mMap.valueAt((int)shipPos.x, (int)shipPos.y))).str() + "km   ", pos, 0.75).y;

    gl::setMatrices( mCamera );

    gl::lineWidth(1);

    gl::scale(2* Vec3f( 10, 10, 10 ) );


    mTexture->enableAndBind();
    mShader->bind();
    mShader->uniform( "tex0", 0 );
    mShader->uniform( "texTransform", mTexTransform );
    mShader->uniform( "zoom", mZoom );

    int indiciesInLine = mPoints;
    int indiciesInMask = mPoints * 2;
    // Draw front to back to allow the depth buffer to do its job.
    for (int i = mLines - 1; i >= 0; --i) {
        gl::color( blue );
        gl::drawRange( mLineMesh, i * indiciesInLine, indiciesInLine);

        gl::color( Color::gray(0.1) );
        // gl::enableWireframe();
        gl::drawRange( mMaskMesh, i * indiciesInMask, indiciesInMask);
        // gl::disableWireframe();
    }

    mShader->unbind();
    mTexture->unbind();


    gl::color( Color::gray(0.8) );
    Vec2f vec = Vec2f(cos(mShip.mPos.w), sin(mShip.mPos.w)) * 2.0;
    gl::drawVector(Vec3f(0.0,5.0,0.0), Vec3f(vec.x, 5.0, vec.y));


    gl::popMatrices();
}


void AlienLanderApp::mouseMove( MouseEvent event )
{
    //    int height = getWindowHeight();
    //    mRatio = 1 - (math<float>::clamp(event.getY(), 0, height) / height);
}

void AlienLanderApp::touchesMoved( TouchEvent event )
{
    // TODO treat the two deltas as forces acting on a rigid body.
    // Accelenration becomse translation
    // Torque becomes rotation
    // Compression/tension becomes zooming
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
    float lateralThrust = 0.00001;

    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE:
            quit();
            break;
        case KeyEvent::KEY_SPACE:
            mShip.mThrusters.z = 0.00008;
            break;
        case KeyEvent::KEY_DOWN:
            mShip.mThrusters.y = +lateralThrust;
            break;
        case KeyEvent::KEY_UP:
            mShip.mThrusters.y = -lateralThrust;
            break;
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
            mShip.mThrusters.z = 0;
            break;
        case KeyEvent::KEY_DOWN:
        case KeyEvent::KEY_UP:
            mShip.mThrusters.y = 0.0;
            break;
        case KeyEvent::KEY_LEFT:
        case KeyEvent::KEY_RIGHT:
            mShip.mThrusters.w = 0.0;
            break;
    }
}


CINDER_APP_NATIVE( AlienLanderApp, RendererGl )
