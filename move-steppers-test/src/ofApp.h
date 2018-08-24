#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxSerial.h"
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

		// GUI
		ofxPanel gui;
		ofParameter<int> gui_stepper_x_pos;
		ofParameter<int> gui_stepper_y_pos;
		ofxButton gui_send_move_command;
		ofxButton gui_send_gethome_command;

		const int Y_MIN_POS = 0;
		const int Y_MAX_POS = 1000;
		const int X_MIN_POS = 0;
		const int X_MAX_POS = 800;

		void on_stepper_x_pos_changed(int & pos);
		void on_stepper_y_pos_changed(int & pos);
		void on_send_command_pressed();
		void on_send_gethome_pressed();

		glm::vec2 stepper_pos;
		bool send_command_pressed;
		
		// SERIAL
		const int BAUD_RATE = 9600;

		std::string sent_command;
		
		void onSerialBuffer(const ofxIO::SerialBufferEventArgs& args);
		void onSerialError(const ofxIO::SerialBufferErrorEventArgs& args);

		ofxIO::SLIPPacketSerialDevice serial_device;
		std::vector<SerialMessage> serial_messages;
};