#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxCv.h"

class ofApp : public ofBaseApp{
public:
	void setup();
	void update();
	void draw();
	
	ofImage input_image, output_image;
	ofFbo small_dots_fbo, big_dots_fbo;
};
