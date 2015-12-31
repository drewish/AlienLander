/*
TODO list:
 - fix point of rotation/scaling, should move towards closer edge as you descend
 - Figure out why craft x & y aren't displayed
 - Figure out why texture's x coordinates are flipped
 - standardize on either Y or Z axis for heights
 - Shader to draw thicker lines
 - Use touch for scaling/rotation/panning
 - Compute point in texture, extract height
 - Display height over ground
 - Detect landing/collision
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
#include "Ship.h"
#include "UnionJack.h"

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

    uint mPoints = 50;
    uint mLines = 50;
    bool mShowHud = true;
    bool mShowCompass = false;

    Ship mShip;
    vector<UnionJack> mDisplays;

    gl::BatchRef    mLineBatch;
    gl::BatchRef    mMaskBatch;

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
    settings->setMultiTouchEnabled( true );
    settings->setHighDensityDisplayEnabled();
    settings->setWindowSize( 800, 800 );
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

    mDisplays.push_back( UnionJack( 10 ).position( vec2( 5 ) ).scale( 2 ) );
    mDisplays.push_back( UnionJack( 10 ).rightOf( mDisplays.back() ) );
    mDisplays.push_back( UnionJack( 35 ).below( mDisplays.front() ) );
    mDisplays.push_back( UnionJack( 35 ).below( mDisplays.back() ) );

    for ( auto &display : mDisplays ) {
        display.colors( ColorA( mBlue, 0.8 ), ColorA( mDarkBlue, 0.4 ) );
    }

//    setFullScreen( true );
    setFrameRate(60);
    gl::enableVerticalSync(true);
}

void AlienLanderApp::buildMeshes()
{
    vector<vec3> lineCoords;
    vector<vec3> maskCoords;

    for( uint z = 0; z < mLines; ++z ) {
        for( uint x = 0; x < mPoints; ++x ) {
            vec3 vert = vec3( x / (float) mPoints, 1, z / (float) mLines );

            lineCoords.push_back( vert );

            // To speed up the vertex shader it only does the texture lookup
            // for vertexes with y values greater than 0. This way we can build
            // a strip: 1 1 1  that will become: 2 9 3
            //          |\|\|                    |\|\|
            //          0 0 0                    0 0 0
            maskCoords.push_back( vert );
            vert.y = 0.0;
            maskCoords.push_back( vert );
        }
    }
    gl::VboMeshRef lineMesh = gl::VboMesh::create( lineCoords.size(), GL_LINE_STRIP, {
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
    });
    lineMesh->bufferAttrib( geom::Attrib::POSITION, lineCoords );
    mLineBatch = gl::Batch::create( lineMesh, mShader );

    gl::VboMeshRef maskMesh = gl::VboMesh::create( maskCoords.size(), GL_TRIANGLE_STRIP, {
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
    });
    maskMesh->bufferAttrib( geom::Attrib::POSITION, maskCoords );
    mMaskBatch = gl::Batch::create( maskMesh, mShader );
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
    float scale = math<float>::clamp( mShip.mPos.z, 0.2, 1.0 );
    mTextureMatrix = glm::translate( vec3( 0.5, 0.5, 0 ) );
    mTextureMatrix = glm::rotate( mTextureMatrix, mShip.mPos.w, vec3( 0, 0, 1 ) );
    mTextureMatrix = glm::scale( mTextureMatrix, vec3( scale, scale, 0.25 ) );
    mTextureMatrix = glm::translate( mTextureMatrix, vec3( mShip.mPos.x, mShip.mPos.y, 0 ) );
    mTextureMatrix = glm::translate( mTextureMatrix, vec3( -0.5, -0.5, 0 ) );

    // TODO: Need to change the focus point to remain parallel as we descend
    mCamera.setPerspective( 40.0f, 1.0f, 0.5f, 3.0f );
    mCamera.lookAt( vec3( 0.0f, 1.5f * mShip.mPos.z, 1.0f ), vec3(0.0,0.1,0.0), vec3( 0, 1, 0 ) );

    if ( mShowHud ) {
        const vec4 &vel = mShip.mVel;
        const vec4 &acc = mShip.mAcc;
        float fps = getAverageFps();
        boost::format zeroToOne( "%+07.5f" );
        boost::format shortForm( "%+08.4f" );

        mDisplays[0].display( "ALT " + (zeroToOne % mShip.mPos.z).str() );
        mDisplays[1]
            .display( "FPS " + (shortForm % fps).str() )
            .colors( ColorA( fps < 50 ? mRed : mBlue, 0.8 ), ColorA( mDarkBlue, 0.8 ) );
        mDisplays[2].display(
            " X " + (shortForm % vel.x).str() + " " +
            " Y " + (shortForm % vel.y).str() + " " +
            " R " + (shortForm % vel.w).str()
        );
        mDisplays[3].display(
            "dX " + (shortForm % acc.x).str() + " " +
            "dY " + (shortForm % acc.y).str() + " " +
            "dR " + (shortForm % acc.w).str()
        );
    }
}

void AlienLanderApp::draw()
{
    gl::clear( mBlack, true );

    {
        gl::ScopedMatrices matrixScope;
        gl::setMatrices( mCamera );

        gl::ScopedDepth depthScope(true);

        mShader->uniform( "textureMatrix", mTextureMatrix );

        // Center the model
        gl::translate(-0.5, 0.0, -0.5);

        uint indiciesInLine = mPoints;
        uint indiciesInMask = mPoints * 2;
        // Draw front to back to take advantage of the depth buffer.
        for (int i = mLines - 1; i >= 0; --i) {
            gl::color( mBlack );
            // Draw masks with alternating colors for debugging
            // gl::color( Color::gray( i % 2 == 1 ? 0.5 : 0.25) );
            mMaskBatch->draw( i * indiciesInMask, indiciesInMask );

            gl::color( mBlue );
            mLineBatch->draw( i * indiciesInLine, indiciesInLine );
        }
    }

    // Compass vector pointing north
    if ( mShowCompass ) {
        gl::color( mRed );
        vec3 origin = vec3( 0.5, 0.2, 0.5 );
        vec3 heading = glm::rotateY( vec3(0.01, 0, 0), mShip.mPos.w );
        gl::drawVector( origin, origin + heading, 0.05, 0.01);
    }

    if ( mShowHud ) {
        for ( auto display = mDisplays.begin(); display != mDisplays.end(); ++display ) {
            display->draw();
        }
    }
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
        case KeyEvent::KEY_h:
            mShowHud = !mShowHud;
            break;
        case KeyEvent::KEY_c:
            mShowCompass = !mShowCompass;
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
