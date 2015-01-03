/*
TODO list:
 - Detect landing/collision
 - Compute/Display height over ground
 - Use touch for scaling/rotation/panning
 - Add concept of fuel
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

    int mPoints = 5;
    int mLines = 2;

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
//    setFullScreen( true );
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
    mMaskMesh = gl::VboMesh::create( totalVertices, totalIndicies, maskLayout, GL_QUAD_STRIP );

    indices.clear();
    vertCoords.clear();
    texCoords.clear();

    for( int z = 0; z < mLines; ++z ) {
        for( int x = 0; x < mPoints; ++x ) {
            Vec3f vert = Vec3f( x / (float)mPoints - 0.5, 0.0, z / (float)mLines - 0.5);
            Vec2f coord = Vec2f( x / (float)mPoints, z / (float)mLines );

            vertCoords.push_back(vert);
            vert.y = -0.2; // the vertex shader just offsets so if we move it
                           // down it'll just skew the strip.
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
    int margin = 10;
    mPoints = (width - (2 * margin)) / margin;
    mLines = (height - (2 * margin)) / margin;
    buildMeshes();
}

void AlienLanderApp::update()
{
    mShip.update();

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
    gl::pushMatrices();

    Color8u black = Color::gray(0.3);
    Color8u blue = Color8u(66, 161, 235);
    Color8u darkBlue = Color8u::hex(0x1A3E5A);

    gl::clear( black ); // Color::black()


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

    gl::scale(2* Vec3f( 10, 10, 10 ) );


    mTexture->enableAndBind();
    mShader->bind();
    mShader->uniform( "tex0", 0 );
    mShader->uniform( "texTransform", mTexTransform );
    mShader->uniform( "zoom", mZoom );

    int indiciesInLine = mPoints;
    int indiciesInMask = mPoints * 2;
    for (int i = 0; i < mLines; ++i) {
        gl::color( black );
//        gl::enableWireframe();
        gl::drawRange( mMaskMesh, i * indiciesInMask, indiciesInMask);
//        gl::disableWireframe();
        gl::color( blue );
        gl::drawRange( mLineMesh, i * indiciesInLine, indiciesInLine);
    }

    mShader->unbind();
    mTexture->unbind();

    gl::popMatrices();
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


CINDER_APP_NATIVE( AlienLanderApp, RendererGl )
