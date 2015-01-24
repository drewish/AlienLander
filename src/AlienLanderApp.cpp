/*
TODO list:
 - standardize on either Y or Z axis for heights
 - fix point of rotation, should move towards closer edge as you descend
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
    void buildMeshes();
    void resize();
    void mouseMove( MouseEvent event );
    void touchesMoved( TouchEvent event );
    void keyDown( KeyEvent event );
    void keyUp( KeyEvent event );
    void update();
    void draw();

    uint mPoints = 21;
    uint mLines = 40;

    Ship mShip;
    SegmentDisplay mDisplay = SegmentDisplay(16, Vec2i(0, 0), 2);

    gl::VboMeshRef	mMaskMesh;
    gl::VboMeshRef	mLineMesh;
    gl::TextureRef	mTexture;
    gl::GlslProgRef	mShader;
    CameraPersp     mCamera;

    Color mBlack = Color::black();
    Color mBlue = Color8u(66, 161, 235);
    Color mDarkBlue = Color8u::hex(0x1A3E5A);
    Color mRed = Color8u(240,0,0);

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
        console() << "unable to load the texture file!" << std::endl;
    }
    mTexture->setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

    try {
        mShader = gl::GlslProg::create( loadResource( RES_VERT ), loadResource( RES_FRAG ) );
    }
    catch( gl::GlslProgCompileExc &exc ) {
        console() << "Shader compile error: " << std::endl;
        console() << exc.what();
    }
    catch( ... ) {
        console() << "Unable to load shader" << std::endl;
    }

    buildMeshes();

    mShip.setup();
    mDisplay.setup();

//    setFullScreen( true );
    setFrameRate(60);
    gl::enableVerticalSync(false);

    mCamera.setPerspective( 40.0f, 1.0f, 0.5f, 3.0f );
}

void AlienLanderApp::buildMeshes()
{
    uint totalVertices = mPoints * mLines;
    uint totalIndicies = mPoints * mLines;
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    mLineMesh = gl::VboMesh::create( totalVertices, totalIndicies, layout, GL_LINE_STRIP );

    vector<uint32_t> indices;
    vector<Vec3f> vertCoords;
    vector<Vec2f> texCoords;
    for( uint z = 0; z < mLines; ++z ) {
        for( uint x = 0; x < mPoints; ++x ) {
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

    for( uint z = 0; z < mLines; ++z ) {
        for( uint x = 0; x < mPoints; ++x ) {
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
        for( uint x = 1; x <= mPoints * 2; x += 2 ) {
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
    uint height = getWindowHeight();
    uint width = getWindowWidth();
    uint margin = 20;
    mPoints = (width - (2 * margin)) / 10.0;
    mLines = (height - (2 * margin)) / 25.0;
    buildMeshes();
}

void AlienLanderApp::update()
{
    mShip.update();

    float fps = getAverageFps();
    boost::format formatter("%+05f");
    mDisplay.update("FPS " + (formatter % fps).str(), (fps > 30) ? mBlue : mRed, mDarkBlue);

    float z = math<float>::clamp(mShip.mPos.z, 0.0, 1.0);
    // TODO: Need to change the focus point to remain parallel as we descend
    mCamera.lookAt( Vec3f( 0.0f, 1.5f * z, 1.0f ), Vec3f(0.0,0.1,0.0), Vec3f::yAxis() );
}

void AlienLanderApp::draw()
{
    gl::pushMatrices();

    gl::enableDepthRead( true );
    gl::enableDepthWrite( true );
    gl::clear( mBlack, true );

    gl::setMatrices( mCamera );

    gl::lineWidth(1);
    mTexture->enableAndBind();
    mShader->bind();
    mShader->uniform( "tex0", 0 );
    mShader->uniform( "zoom", 0.5f /*mZoom*/ );

    // Transform the height map via the texture matrix
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();

    float scale = mShip.mPos.z;
    gl::translate( 0.5, 0.5 );
    gl::rotate( mShip.mPos.w * 180 / M_PI );
    gl::scale( scale, scale );
    gl::translate( mShip.mPos.xy() );
    gl::translate( -0.5, -0.5 );

    uint indiciesInLine = mPoints;
    uint indiciesInMask = mPoints * 2;
    // Draw front to back to allow the depth buffer to do its job.
    for (int i = mLines - 1; i >= 0; --i) {
        gl::color( mBlue );
        gl::drawRange( mLineMesh, i * indiciesInLine, indiciesInLine);

        gl::color( Color::gray(0.1) );
        gl::drawRange( mMaskMesh, i * indiciesInMask, indiciesInMask);
    }

    glLoadIdentity();
    glMatrixMode( GL_MODELVIEW );

    mShader->unbind();
    mTexture->unbind();

/*
    // Vector pointing north
    gl::lineWidth(2);
    gl::color( mRed );
    Vec2f vec = Vec2f(cos(mShip.mPos.w), sin(mShip.mPos.w)) / 40;
    gl::drawVector(Vec3f(0.0,1 / 10.0,0.0), Vec3f(vec.x, 1 / 10.0, vec.y), 1/20.0, 1/100.0);
*/

    gl::disableDepthRead( );
    gl::disableDepthWrite( );


    gl::popMatrices();


    gl::lineWidth(2);
    mDisplay.draw();
}


void AlienLanderApp::mouseMove( MouseEvent event )
{
//    int height = getWindowHeight();
//    int width = getWindowWidth();
//    mZoom = 1 - (math<float>::clamp(event.getY(), 0, height) / height);
//    mAngle = (math<float>::clamp(event.getX(), 0, width) / width);
//    2 * M_PI *
}

void AlienLanderApp::touchesMoved( TouchEvent event )
{
    return;
    Vec2f mDelta1, mDelta2;

    // TODO treat the two deltas as forces acting on a rigid body.
    // Acceleration becomes translation
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
    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE:
            quit();
            break;
        default:
            mShip.keyDown(event);
            break;
    }
}

void AlienLanderApp::keyUp( KeyEvent event )
{
    mShip.keyUp(event);
}


CINDER_APP_NATIVE( AlienLanderApp, RendererGl )
