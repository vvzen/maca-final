#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxPS3EyeGrabber.h"

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;

		void keyPressed(int key) override;

		bool button_pressed;
		bool show_live_feed;
		const int cam_width = 640;
    	const int cam_height = 480;
		const int circle_size = 4; // TODO: find biggest circle good for both width and height
		
		std::shared_ptr<ofVideoGrabber> video_grabber;
		
		ofFbo dots_fbo;
		ofxCvColorImage	color_img;
		ofxCvGrayscaleImage thresholded_img_1;
		ofxCvGrayscaleImage thresholded_img_2;
		
		// used to generate the code for the paintball guns
		vector<glm::vec2> red_dots_positions; 
		vector<glm::vec2> black_dots_positions;
		void export_dots_to_csv(vector<glm::vec2> positions, std::string filename);
};
