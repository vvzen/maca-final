#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	// ofApp * app = new ofApp();
	// ofSetupOpenGL(app->cam_width, app->cam_height, OF_WINDOW);			// <-------- setup the GL context
	ofSetupOpenGL(500, 500, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	// ofRunApp(app);
	return ofRunApp(std::make_shared<ofApp>());
}
