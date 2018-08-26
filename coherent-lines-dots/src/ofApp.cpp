#include "ofApp.h"

// using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup() {
    
	ofBackground(255);

	input_image.load("0162681551.png");
	input_image.setImageType(OF_IMAGE_GRAYSCALE);

	small_dots_fbo.allocate(256, 256, GL_RGBA, 8);
	big_dots_fbo.allocate(256, 256, GL_RGBA, 8);
	
	// halfw 6
	// smoothPasses 1
	// sigma1 2
	// sigma2 0.95905
	// tau 0.98 --> thresholding
	// black -8
	// threshold 164
}

//--------------------------------------------------------------
void ofApp::update(){

	int halfw = 6;
	int smooth_passes = 1;
	float sigma1 = 4.50; // degree of coherence
	float sigma2 = 0.95905;
	float tau = 0.98;
	int black = -8;
	int threshold = 164;

	ofxCv::CLD(input_image, output_image, halfw, smooth_passes, sigma1, sigma2, tau, black);
	ofxCv::invert(output_image);
	ofxCv::threshold(output_image, threshold);
	output_image.update();

	// Draw the smaller dots
	int circle_size = 2;

	small_dots_fbo.begin();

	for (int x = circle_size/2; x < output_image.getWidth(); x+= circle_size*4){
		for (int y = circle_size/2; y < output_image.getHeight(); y+= circle_size*4){
			
			ofColor c = output_image.getColor(x, y);

			if (c.r == 255){
				ofSetColor(ofColor::orange);
				ofDrawCircle(x, y, circle_size);
			}
		}
	}

	small_dots_fbo.end();
	

	// Draw the bigger dots
	circle_size = 5;

	big_dots_fbo.begin();

	for (int x = circle_size/2; x < output_image.getWidth(); x+= circle_size*2){
		for (int y = circle_size/2; y < output_image.getHeight(); y+= circle_size*2){
			
			ofColor c = output_image.getColor(x, y);

			if (c.r == 255){
				ofSetColor(ofColor::orange);
				ofDrawCircle(x, y, circle_size);
			}
		}
	}

	big_dots_fbo.end();
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(ofColor::white);
	ofDrawBitmapStringHighlight("Coherent line drawing", 10, 20);

	input_image.draw(0, 0);
	output_image.draw(256, 0);
	small_dots_fbo.draw(0, 256);
	big_dots_fbo.draw(256, 256);
    
	// ofTranslate(300, 0);
	// for(int i = 0; i < input.size(); i++) {
	// 	// ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	// 	// input[i].draw(i * 256, 0);
	// 	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	// 	output[i].draw(i * 256, 0);
		
	// 	// ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	// 	// input[i].draw(i * 256, 256);
	// 	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	// 	// canny[i].draw(i * 256, 256);
	// }
	
	// ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}
