#include "ofApp.h"

//========================================================================
int main( ){

	ofSetupOpenGL(512, 512, OF_WINDOW);			// <-------- setup the GL context
	ofRunApp(std::make_shared<ofApp>());

}
