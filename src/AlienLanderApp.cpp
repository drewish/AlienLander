#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AlienLanderApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void AlienLanderApp::setup()
{
}

void AlienLanderApp::mouseDown( MouseEvent event )
{
}

void AlienLanderApp::update()
{
}

void AlienLanderApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( AlienLanderApp, RendererGl )
