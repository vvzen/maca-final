#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);

		bool button_pressed;
		const int cam_width = 320;
    	const int cam_height = 240;
		const int circle_size = 4; // TODO: find biggest circle good for both width and height
		ofVideoGrabber video_grabber;
		
		ofFbo dots_fbo;
		ofxCvColorImage	color_img;
		ofxCvGrayscaleImage thresholded_img_1;
		ofxCvGrayscaleImage thresholded_img_2;
		
		// used to generate the code for the paintball guns
		vector<glm::vec2> red_dots_positions; 
		vector<glm::vec2> black_dots_positions;
		void export_dots_to_csv(vector<glm::vec2> positions, std::string filename);
};
