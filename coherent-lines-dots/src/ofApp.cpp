#include "ofApp.h"

// using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup() {
    
	ofBackground(255);

	// set the logging to a file
	ofLogToFile("paintball.log");

	// load input image
	input_image.load("test-camsize.jpg");
	input_image.setImageType(OF_IMAGE_GRAYSCALE);
	
	// connect to the 2 arduinos
	init_serial_devices(cnc_device, cam_servo_device);

	current_command_index = 0;

	dots_fbo.allocate(cam_width, cam_height, GL_RGBA, 8);

	// filter the image
	run_coherent_line_drawing(input_image, output_image, dots_fbo);

	// start to send the values over serial
	send_current_command(current_command_index);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(ofColor::white);
	
	dots_fbo.draw(0, 0);

	ofDrawBitmapStringHighlight("Coherent line drawing", 10, 20);
	ofDrawBitmapStringHighlight("Number of dots: " + ofToString(dots.size()), 10, 35);
}

//--------------------------------------------------------------
// SERIAL
//--------------------------------------------------------------
void ofApp::init_serial_devices(ofxIO::SLIPPacketSerialDevice& device1, ofxIO::SLIPPacketSerialDevice& device2){
	auto devices_info = ofxIO::SerialDeviceUtils::listDevices();

	// log connected devices
    ofLogNotice("ofApp::setup") << "Connected devices: ";
    for (std::size_t i = 0; i < devices_info.size(); ++i){ ofLogVerbose("ofApp::setup") << "\t" << devices_info[i]; }

	if (!devices_info.empty()){

        // Connect to the matching device.
        bool success = device1.setup(devices_info[0], BAUD_RATE);

        if (success){
            device1.registerAllEvents(this);
            ofLogVerbose("ofApp::setup") << "Successfully setup " << devices_info[0];
        }
        else ofLogError("ofApp::setup") << "Unable to setup " << devices_info[0];
    }
    else ofLogNotice("ofApp::setup") << "No devices connected.";
}

//--------------------------------------------------------------
void ofApp::send_current_command(int i){

    glm::vec2 pos = dots.at(i);

    ofxOscMessage osc_message;
	osc_message.setAddress("/stepper");
	osc_message.addIntArg(pos.x);
	osc_message.addIntArg(pos.y);

    ofLogNotice("send_current_command") << "/stepper " << pos.x << " " << pos.y << " - " << current_command_index+1 << "/" << ofToString(dots.size());

    // check onSerialBuffer to see what happens after we sent a command
}

//--------------------------------------------------------------
// OPENCV
//--------------------------------------------------------------
void ofApp::run_coherent_line_drawing(ofImage &in, ofImage &out, ofFbo& dots_fbo){
	
	// Reset deque
	dots.clear();

	// Do the coherent line drawing magic
	ofxCv::CLD(input_image, output_image, halfw, smooth_passes, sigma1, sigma2, tau, black);
	ofxCv::invert(output_image);
	ofxCv::threshold(output_image, threshold);
	output_image.update();
	
	// Draw the dots on their fbo
	int circle_size = 5;
	int sampling_size = circle_size * 2;

	dots_fbo.begin();

	// Sample the pixels from the coherent line image
	// and add dots if we found a white pixel
	for (int x = circle_size/2; x < output_image.getWidth(); x+= sampling_size){
		for (int y = circle_size/2; y < output_image.getHeight(); y+= sampling_size){
			
			ofColor c = output_image.getColor(x, y);

			if (c.r == 255){
				ofSetColor(ofColor::orange);
				ofDrawCircle(x, y, circle_size);
				dots.push_back(glm::vec2(x, y));
			}
		}
	}

	dots_fbo.end();

	ofLogNotice("run_coherent_line_drawing()") << "completed";
}

//--------------------------------------------------------------
// OSC
//--------------------------------------------------------------
void ofApp::append_message(ofxOscMessage& message, osc::OutboundPacketStream& p){

    p << osc::BeginMessage(message.getAddress().data());

    for (int i = 0; i < message.getNumArgs(); ++i){

        if ( message.getArgType(i) == OFXOSC_TYPE_INT32)
            p << message.getArgAsInt32(i);

        else if ( message.getArgType(i) == OFXOSC_TYPE_INT64 )
            p << (osc::int64)message.getArgAsInt64( i );

        else if ( message.getArgType(i) == OFXOSC_TYPE_FLOAT )
            p << message.getArgAsFloat(i);

        else if ( message.getArgType(i) == OFXOSC_TYPE_DOUBLE )
            p << message.getArgAsDouble( i );

        else if ( message.getArgType( i ) == OFXOSC_TYPE_STRING || message.getArgType( i ) == OFXOSC_TYPE_SYMBOL)
            p << message.getArgAsString( i ).data();

        else if ( message.getArgType( i ) == OFXOSC_TYPE_CHAR )
            p << message.getArgAsChar( i );

        else if ( message.getArgType( i ) == OFXOSC_TYPE_TRUE || message.getArgType( i ) == OFXOSC_TYPE_FALSE )
            p << message.getArgAsBool( i );

        else if ( message.getArgType( i ) == OFXOSC_TYPE_TRIGGER )
            p << message.getArgAsTrigger( i );

        else if ( message.getArgType( i ) == OFXOSC_TYPE_TIMETAG )
            p << (osc::int64)message.getArgAsTimetag( i );

        else if ( message.getArgType( i ) == OFXOSC_TYPE_BLOB ){
            ofBuffer buff = message.getArgAsBlob(i);
            osc::Blob b(buff.getData(), (unsigned long)buff.size());
            p << b;

        }
        else {
            ofLogError("ofxOscSender") << "append_message(): bad argument type " << message.getArgType(i);
            assert(false);
        }
    }

    p << osc::EndMessage;
}

//--------------------------------------------------------------
void ofApp::send_osc_bundle(ofxOscMessage &m, ofxIO::SLIPPacketSerialDevice& device, int buffer_size){
    // this code come from ofxOscSender::sendMessage in ofxOscSender.cpp
    // static const int OUTPUT_BUFFER_SIZE = buffer_size;
    char buffer[buffer_size];

    // create the packet stream
    osc::OutboundPacketStream p(buffer, buffer_size);
    
    p << osc::BeginBundleImmediate; // start the bundle
    append_message(m, p);           // add the osc message to the bundle
    p << osc::EndBundle;            // end the bundle

    // send to device
    device.send(ofxIO::ByteBuffer(p.Data(), p.Size()));
}

//--------------------------------------------------------------
// SERIAL
//--------------------------------------------------------------
void ofApp::onSerialBuffer(const ofx::IO::SerialBufferEventArgs& args){
    
	std::string received_command = args.buffer().toString();
	ofLogNotice("onSerialBuffer") << "received message: " << received_command;

	// The arduino sends us back a string formatted like that: "stepperx:valuey:value"
	// so we recreate artificially a similar string and we check if it's equal to the arduino message
	std::ostringstream sent_message_values;
	glm::vec2 current_pos = dots.at(current_command_index);
	sent_message_values << "stepperx:" << current_pos.x << "y:" << current_pos.y;

	// check if we need to send more messages
	if (current_command_index <= dots.size() - 1){
		// if arduino received the same message that we sent then send the next message
        if (received_command == sent_message_values.str()) send_current_command(++current_command_index);
    }
    else {
        ofLogNotice() << "sent all commands!";
        current_command_index = 0;
    }
}

//--------------------------------------------------------------
void ofApp::onSerialError(const ofx::IO::SerialBufferErrorEventArgs& args){
    // Errors and their corresponding buffer (if any) will show up here.
    ofLogError("onSerialError") << "Serial error : " << args.exception().displayText();
}

//--------------------------------------------------------------
// EXIT
//--------------------------------------------------------------
void ofApp::exit(){
	cnc_device.unregisterAllEvents(this);
	cam_servo_device.unregisterAllEvents(this);
}