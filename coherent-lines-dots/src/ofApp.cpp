#include "ofApp.h"

// using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup() {
    
	ofBackground(255);

	input_image.load("test-camsize.jpg");
	input_image.setImageType(OF_IMAGE_GRAYSCALE);
	
	dots_fbo.allocate(cam_width, cam_height, GL_RGBA, 8);

	run_coherent_line_drawing(input_image, output_image, dots_fbo);
}

//--------------------------------------------------------------
void ofApp::update(){

	// halfw 6
	// smoothPasses 1
	// sigma1 2
	// sigma2 0.95905
	// tau 0.98 --> thresholding
	// black -8
	// threshold 164
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(ofColor::white);
	
	dots_fbo.draw(0, 0);

	ofDrawBitmapStringHighlight("Coherent line drawing", 10, 20);
	ofDrawBitmapStringHighlight("Number of dots: " + ofToString(dots.size()), 10, 35);
}

//--------------------------------------------------------------
// OPENCV
//--------------------------------------------------------------
void ofApp::run_coherent_line_drawing(ofImage &in, ofImage &out, ofFbo& dots_fbo){
	
	dots.clear();

	ofxCv::CLD(input_image, output_image, halfw, smooth_passes, sigma1, sigma2, tau, black);
	ofxCv::invert(output_image);
	ofxCv::threshold(output_image, threshold);
	output_image.update();
	
	// Draw the dots on their fbo
	int circle_size = 5;

	dots_fbo.begin();

	for (int x = circle_size/2; x < output_image.getWidth(); x+= circle_size*2){
		for (int y = circle_size/2; y < output_image.getHeight(); y+= circle_size*2){
			
			ofColor c = output_image.getColor(x, y);

			if (c.r == 255){
				ofSetColor(ofColor::orange);
				ofDrawCircle(x, y, circle_size);
				dots.push_back(glm::vec2(x, y));
			}
		}
	}

	dots_fbo.end();
}

//--------------------------------------------------------------
// OSC
//--------------------------------------------------------------
void ofApp::start_transmission(){
	// TODO:
}

//--------------------------------------------------------------
void ofApp::send_osc_bundle(ofxOscMessage &m, int buffer_size){
    // this code come from ofxOscSender::sendMessage in ofxOscSender.cpp
    // static const int OUTPUT_BUFFER_SIZE = buffer_size;
    char buffer[buffer_size];

    // create the packet stream
    osc::OutboundPacketStream p(buffer, buffer_size);
    
    p << osc::BeginBundleImmediate; // start the bundle
    append_message(m, p);           // add the osc message to the bundle
    p << osc::EndBundle;            // end the bundle

    // send to device
    serial_device.send(ofxIO::ByteBuffer(p.Data(), p.Size()));
}

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
            ofLogError("ofxOscSender") << "append_message(): bad argument type " << message.getArgType( i );
            assert(false);
        }
    }

    p << osc::EndMessage;
}

//--------------------------------------------------------------
// SERIAL
//--------------------------------------------------------------
void ofApp::onSerialBuffer(const ofx::IO::SerialBufferEventArgs& args){
    // Decoded serial packets will show up here.
    ofLogNotice() << "received message: " << args.buffer().toString();
}

//--------------------------------------------------------------
void ofApp::onSerialError(const ofx::IO::SerialBufferErrorEventArgs& args){
    // Errors and their corresponding buffer (if any) will show up here.
    ofLogNotice("ofApp::onSerialError") << "Serial error : " << args.exception().displayText();
}

//--------------------------------------------------------------
// EXIT
//--------------------------------------------------------------
void ofApp::exit(){
	serial_device.unregisterAllEvents(this);
}