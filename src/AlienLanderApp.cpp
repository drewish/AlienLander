/*
TODO list:
 - standardize on either Y or Z axis for heights
 - fix point of rotation, should move towards closer edge as you descend
 - Use touch for scaling/rotation/panning
 - Detect landing/collision
 - Compute/Display height over ground
*/
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"

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

class AlienLanderApp : public App {
public:
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
    SegmentDisplay mDisplay = SegmentDisplay(16, vec2(5), 2);

    gl::BatchRef    mLineBatch;
    gl::BatchRef    mMaskBatch;

    gl::VboMeshRef	mMaskMesh;
    gl::VboMeshRef	mLineMesh;
    gl::TextureRef	mTexture;
    gl::GlslProgRef	mShader;
    CameraPersp     mCamera;
    mat4            mTextureMatrix;

    Color mBlack = Color::black();
    Color mBlue = Color8u(66, 161, 235);
    Color mDarkBlue = Color8u::hex(0x1A3E5A);
    Color mRed = Color8u(240,0,0);

};

void prepareSettings( App::Settings *settings )
{
    settings->setHighDensityDisplayEnabled();
    settings->setWindowSize(800, 800);
}

void AlienLanderApp::setup()
{
    try {
        mTexture = gl::Texture::create( loadImage( loadResource( RES_US_SQUARE ) ) );
        mTexture->bind( 0 );

    }
    catch( ... ) {
        console() << "unable to load the texture file!" << std::endl;
    }
    mTexture->setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);


//    mShader = gl::getStockShader( gl::ShaderDef().texture() );
    try {
        mShader = ci::gl::GlslProg::create(
            ci::app::loadResource( RES_VERT ),
            ci::app::loadResource( RES_FRAG )
        );
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
    gl::enableVerticalSync(true);

    mCamera.setPerspective( 40.0f, 1.0f, 0.5f, 3.0f );
}

void AlienLanderApp::buildMeshes()
{
    vector<vec3> vertCoords;
    vector<vec2> texCoords;

    for( uint z = 0; z < mLines; ++z ) {
        for( uint x = 0; x < mPoints; ++x ) {
            vertCoords.push_back( vec3( x / (float)mPoints - 0.5, 0.0, z / (float)mLines - 0.5) );
            texCoords.push_back( vec2( x / (float)mPoints, z / (float)mLines ) );
        }
    }
	mLineMesh = gl::VboMesh::create( vertCoords.size(), GL_LINE_STRIP, {
		gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
		gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::TEX_COORD_0, 2 ),
	});
    mLineMesh->bufferAttrib( geom::Attrib::POSITION, vertCoords );
    mLineMesh->bufferAttrib( geom::Attrib::TEX_COORD_0, texCoords );
    mLineBatch = gl::Batch::create( mLineMesh, mShader );

    // * * *

    vertCoords.clear();
    texCoords.clear();

    for( uint z = 0; z < mLines; ++z ) {
        for( uint x = 0; x < mPoints; ++x ) {
            vec3 vert = vec3( x / (float)mPoints - 0.5, 0.0, z / (float)mLines - 0.5);
            vec2 coord = vec2( x / (float)mPoints, z / (float)mLines );

            vertCoords.push_back(vert);
            // the vertex shader just uses the texture value as an offset to
            // the y value so setting one below the first value will create a
            // skewed strip to mask the lines behind it.
            vert.y = -0.5;
            vertCoords.push_back(vert);

            texCoords.push_back( coord );
            texCoords.push_back( coord );
        }
    }
	mMaskMesh = gl::VboMesh::create( vertCoords.size(), GL_TRIANGLE_STRIP, {
		gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
		gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::TEX_COORD_0, 2 ),
	});
    mMaskMesh->bufferAttrib( geom::Attrib::POSITION, vertCoords );
    mMaskMesh->bufferAttrib( geom::Attrib::TEX_COORD_0, texCoords );

    mMaskBatch = gl::Batch::create( mMaskMesh, mShader );
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

    // TODO Need to figure our what to do with the scale... should probably
    // affect the distance to the points rather than being handled by moving
    // the camera...
    float scale = mShip.mPos.z;
    mTextureMatrix = glm::translate( vec3( 0.5, 0.5, 0 ) );
    mTextureMatrix = glm::rotate( mTextureMatrix, mShip.mPos.w, vec3( 0, 0, 1 ) );
    mTextureMatrix = glm::scale( mTextureMatrix, vec3( scale, scale, 1.0f ) );
    mTextureMatrix = glm::translate( mTextureMatrix, vec3( mShip.mPos.x, mShip.mPos.y, 0 ) );
    mTextureMatrix = glm::translate( mTextureMatrix, vec3( -0.5, -0.5, 0 ) );

    float fps = getAverageFps();
    boost::format formatter( "%+05f" );
    mDisplay
        .colors( ColorA( fps > 30 ? mBlue : mRed, 0.8 ), ColorA( mDarkBlue, 0.8 ) )
        .display( "FPS " + (formatter % fps).str() );

    float z = math<float>::clamp(mShip.mPos.z, 0.0, 1.0);
    // TODO: Need to change the focus point to remain parallel as we descend
    mCamera.lookAt( vec3( 0.0f, 1.5f * z, 1.0f ), vec3(0.0,0.1,0.0), vec3( 0, 1, 0 ) );
}

void AlienLanderApp::draw()
{

    gl::clear( mBlack, true );

    {
        gl::ScopedMatrices matrixScope;
        gl::setMatrices( mCamera );

        mShader->uniform( "textureMatrix", mTextureMatrix );

        uint indiciesInLine = mPoints;
        uint indiciesInMask = mPoints * 2;
        // { int i = (getElapsedFrames()) % mLines; // stepping
        // for (int i = mLines - 1; i >= 0; --i) { // front to back
        for (int i = 0; i <= mLines; ++i) { // back to front
            gl::color( mBlue );
            mLineBatch->draw( i * indiciesInLine, indiciesInLine);

            gl::color( Color::gray( i % 2 == 1 ? 0.5 : 0.25) );
            mMaskBatch->draw( i * indiciesInMask, indiciesInMask);
        }

        // FIXME: Direction of rotation seems backwards
//        // Vector pointing north
//        gl::color( mRed );
//        vec2 vec = vec2(cos(mShip.mPos.w), sin(mShip.mPos.w)) / vec2(40.0);
//        gl::drawVector(vec3(0.0,1/10.0,0.0), vec3(vec.x,1/10.0, vec.y), 1/20.0, 1/100.0);
    }

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
    vec2 mDelta1, mDelta2;

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


CINDER_APP( AlienLanderApp, RendererGl( RendererGl::Options().msaa( 16 ) ), prepareSettings )
