#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxPS3EyeGrabber.h"
#include "ofxSerial.h"
#include "ofxFaceTracker.h"

struct SerialMessage{
    std::string message;
    std::string exception;
};


class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;

		bool button_pressed;
		bool show_live_feed;
		const int cam_width = 640;
    	const int cam_height = 480;
		const int circle_size = 2; // TODO: find biggest circle good for both width and height
		
		// the video grabber for the PS3Eye Cam
		// a shared_ptr avoids manual allocation of memory (new/delete)
		// when the reference count of the pointed object reaches 0 memory is freed
		std::shared_ptr<ofVideoGrabber> video_grabber;

		// FACE TRACKING
		ofImage img_for_tracker;
		ofxFaceTracker tracker;
		// ofVec2f tracked_face_position;
		glm::vec2 tracked_face_position;
		bool face_detected;

		// DOTS
		ofFbo dots_fbo;
		ofxCvColorImage	color_img;
		ofxCvGrayscaleImage thresholded_img_1;
		ofxCvGrayscaleImage thresholded_img_2;
		
		// used to generate the code for the paintball guns
		vector<glm::vec2> red_dots_positions; 
		vector<glm::vec2> black_dots_positions;
		void export_dots_to_csv(vector<glm::vec2> positions, std::string filename);

		// serial communication with arduino
		const int BAUD_RATE = 115200;

		void send_current_command(int i);
		int current_command_index;
		std::string sent_command, received_command;

		void onSerialBuffer(const ofxIO::SerialBufferEventArgs& args);
    	void onSerialError(const ofxIO::SerialBufferErrorEventArgs& args);
		ofxIO::PacketSerialDevice device;

		std::deque<SerialMessage> serial_messages;

};
