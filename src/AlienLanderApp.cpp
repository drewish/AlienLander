#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Utilities.h"
#include "cinder/Perlin.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class AlienLanderApp : public AppNative {
public:
	void prepareSettings( Settings *settings ) override;
	void setup();
	void resize();
	void mouseMove( MouseEvent event );
	void keyDown( KeyEvent event );
	void update();
	void draw();


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
	settings->enableHighDensityDisplay();
	settings->setWindowSize(800, 800);
}

void AlienLanderApp::setup()
{
	try {
		mMap = Channel32f(loadImage(loadAsset("usa.png")));
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
	mMargin = 20;//round( perCube * 0.45 );
	mPoints = (width - (2 * mMargin)) / 20;
	mLines = (height - (2 * mMargin)) / 20;

	mCam.setOrtho( 0, width, height, 0, -1000, 1000 );
	gl::setMatrices( mCam );
}

void AlienLanderApp::mouseMove( MouseEvent event )
{
}


void AlienLanderApp::keyDown( KeyEvent event )
{
	float offset = 5;
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

void AlienLanderApp::update()
{

}

void AlienLanderApp::draw()
{
	gl::clear( Color::gray(0) );

	//  gl::enableWireframe();

	gl::pushModelView();

	gl::translate(mMargin, 2 * mMargin);

	//  gl::drawRange(mMesh);


	float yScale = (getWindowHeight() - (2 * mMargin)) / mLines;
	float xScale = (getWindowWidth() - (0 * mMargin)) / mPoints;
	Channel32f::Iter iter = mMap.getIter(Area((int)mX, (int)mY, (int)mX + mPoints, (int)mY + mLines));
	while( iter.line() ) {
		PolyLine<Vec2f> line;
		Shape2d mask;
		Vec2f point;

		gl::pushModelView();

		gl::translate(0, (iter.y() - mY) * yScale, 0);
		gl::rotate(Vec3f(60,0,0));

		point = Vec2f(0 * xScale, 0);
		mask.moveTo(point);

		while( iter.pixel() ) {
			float val = 8 * pow(10 * iter.v(), 2);
			point = Vec2f((iter.x() - mX) * xScale, -val);
			mask.lineTo( point );
			line.push_back( point );
		}

		point = Vec2f((mPoints - 1) * xScale, 0);
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

	//  gl::disableWireframe();
}

CINDER_APP_NATIVE( AlienLanderApp, RendererGl )