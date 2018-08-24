#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOpenCv.h"
#include "ofxPS3EyeGrabber.h"
#include "ofxSerial.h"
#include "ofxFaceTracker.h"
#include "ofEvents.h"

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
		const int circle_size = 4; // TODO: find biggest circle good for both width and height
		
		// turn on this flag to get additional debug logs
		const bool APP_DEBUG = false;

		// the video grabber for the PS3Eye Cam
		// a shared_ptr avoids manual allocation of memory (new/delete)
		// when the reference count of the pointed object reaches 0 memory is freed
		std::shared_ptr<ofVideoGrabber> video_grabber;

		// GUI
		ofxPanel gui;
		ofParameter<int> gui_servo_angle;
		void on_servo_angle_changed(int & servo_angle);

		// FACE TRACKING
		ofImage img_for_tracker;
		ofxFaceTracker tracker;
		glm::vec2 tracked_face_position;
		bool face_detected, update_servo;
		const int FACE_DISTANCE_THRESHOLD = 10;
		
		// OPENCV
		ofFbo dots_fbo;
		ofxCvColorImage	color_img;
		ofxCvGrayscaleImage thresholded_img_1;
		ofxCvGrayscaleImage thresholded_img_2;

		// current settings for the hatchlab
		const int FIRST_THRESHOLD = 90; // things darker than this become blue
		const int SECOND_THRESHOLD = 190; // things darker than this become orange
		
		// used to generate the code for the paintball guns
		vector<glm::vec2> midtones_dots_positions; 
		vector<glm::vec2> darker_dots_positions;

		// SERIAL
		// serial communication with arduino
		const int BAUD_RATE = 115200;

		void send_current_command(int i); // used to send commands to the paintball machine
		int current_command_index;
		std::string sent_command, received_command;
		// ofxSerial methods
		void onSerialBuffer(const ofxIO::SerialBufferEventArgs& args);
    	void onSerialError(const ofxIO::SerialBufferErrorEventArgs& args);

		ofxIO::PacketSerialDevice device; // communication with the paintball machine
		std::deque<SerialMessage> serial_messages;

		// FACE TRACKING SERVO
		ofxIO::SLIPPacketSerialDevice servo_cam_serial_device; // communication with the servo holding the ps3eye camera
		bool send_servo_start_command;
		unsigned int seconds_elapsed;
		const int SERVO_START_POSITION = 80;
		const short SERVO_UPDATE_DELAY = 2; // seconds to wait before moving the servo again
};
