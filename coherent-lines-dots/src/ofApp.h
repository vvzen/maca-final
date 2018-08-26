#pragma once

#include "ofMain.h"
#include "ofxSerial.h"
#include "ofEvents.h"
#include "ofxOsc.h"
#include "ofxCv.h"

class ofApp : public ofBaseApp{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void exit() override;

	// PS3 EYE CAMERA
	const int cam_width = 640;
	const int cam_height = 480;

	// OPENCV
	void run_coherent_line_drawing(ofImage& in, ofImage& out, ofFbo& dots_fbo);

	ofImage input_image, output_image;
	ofFbo dots_fbo;
	// coherent line drawing parameters
	int halfw = 6;
	int smooth_passes = 1;
	float sigma1 = 4.50; // degree of coherence
	float sigma2 = 0.95905;
	float tau = 0.98;
	int black = -8;
	int threshold = 164;
	vector<glm::vec2> dots;

	// SERIAL
	const int BAUD_RATE = 9600;
	void init_serial_devices(ofxIO::SLIPPacketSerialDevice& device1, ofxIO::SLIPPacketSerialDevice& device2);
	void send_current_command(int i); // used to send commands to the paintball machine
	int current_command_index; // keeps track of the current command that we're sending

	ofxIO::SLIPPacketSerialDevice cnc_device;
	ofxIO::SLIPPacketSerialDevice cam_servo_device;

	// ofxSerial events
	void onSerialBuffer(const ofxIO::SerialBufferEventArgs& args);
	void onSerialError(const ofxIO::SerialBufferErrorEventArgs& args);

private:
	// OSC STUFF
	 // add our osc message to the osc bundle
	void append_message(ofxOscMessage& message, osc::OutboundPacketStream& p);
	 // send the osc bundle via serial
	void send_osc_bundle(ofxOscMessage &m, ofxIO::SLIPPacketSerialDevice& device, int buffer_size);
};
