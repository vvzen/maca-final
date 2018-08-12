#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    // GUI
    gui.setup();
    gui.add(gui_stepper_x_pos.set("Stepper motor X position", 0, 0, 100));
    gui.add(gui_stepper_y_pos.set("Stepper motor Y position", 150, 0, 150));
    gui.add(gui_send_command.setup("Send Command!"));

    gui_stepper_x_pos.addListener(this, &ofApp::on_stepper_x_pos_changed);
    gui_stepper_y_pos.addListener(this, &ofApp::on_stepper_y_pos_changed);
    gui_send_command.addListener(this, &ofApp::on_send_command_pressed);

    stepper_pos = glm::vec2(0, 150);
    send_command_pressed = false;

    // SERIAL
    std::vector<ofxIO::SerialDeviceInfo> devices_info = ofxIO::SerialDeviceUtils::listDevices();

    // log connected devices
    ofLogNotice("ofApp::setup") << "Connected devices: ";
    for (std::size_t i = 0; i < devices_info.size(); ++i){ ofLogNotice("ofApp::setup") << "\t" << devices_info[i]; }

    if (!devices_info.empty()){

        // Connect to the matching device.
        bool success = serial_device.setup(devices_info[0], BAUD_RATE);

        if (success){
            serial_device.registerAllEvents(this);
            ofLogNotice("ofApp::setup") << "Successfully setup " << devices_info[0];
        }
        else ofLogNotice("ofApp::setup") << "Unable to setup " << devices_info[0];
    }
    else ofLogNotice("ofApp::setup") << "No devices connected.";

    ofLogNotice("ofApp::setup") << "exiting setup()";
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(ofColor(35));

    int width = 200;
    int height = 300;
    int pos_x = ofMap(stepper_pos.x, 0, 100, -width/2, width/2);
    int pos_y = ofMap(stepper_pos.y, 0, 150, -height/2, height/2);

    ofPushMatrix();

    ofTranslate(glm::vec2(ofGetWidth()/2, ofGetHeight()/2));

    ofNoFill();
    ofSetColor(ofColor::red);
    ofDrawRectangle(glm::vec2(-width/2, -height/2), width, height);
    
    ofFill();
    ofSetColor(255);
    ofDrawEllipse(pos_x, pos_y, 10, 10);
    
    ofPopMatrix();

    gui.draw();

    if (send_command_pressed) ofDrawBitmapStringHighlight("Sending command!", glm::vec2(ofGetWidth()/2, ofGetHeight()/2), ofColor::black, ofColor::white);

    send_command_pressed = false;
}

//--------------------------------------------------------------
void ofApp::on_stepper_x_pos_changed(int & pos){
    stepper_pos.x = pos;
    // ofLogNotice() << "stepper x pos changed: " << pos;
}

//--------------------------------------------------------------
void ofApp::on_stepper_y_pos_changed(int & pos){
    stepper_pos.y = pos;
    // ofLogNotice() << "stepper y pos changed: " << pos;
}

//--------------------------------------------------------------
void ofApp::on_send_command_pressed(){
    
    send_command_pressed = true;

    // Format the command
    std::ostringstream command;
    command << "MX" << std::setw(3) << std::setfill('0') << stepper_pos.x; // setw and setfill are used to add 3 leading zeroes
    command << "Y" << std::setfill('0') << stepper_pos.y;

    // Send it to the arduino
    ofx::IO::ByteBuffer buffer(command.str());
    serial_device.send(buffer);

    ofLogNotice() << "send command pressed!";
    ofLogNotice() << "command: " << command.str();
}

//--------------------------------------------------------------
void ofApp::onSerialBuffer(const ofx::IO::SerialBufferEventArgs& args){
    
    // Decoded serial packets will show up here.
    //std::string received_command = args.buffer().toString();

    SerialMessage message;
    message.message = args.buffer().toString();
    //serial_messages.push_back(message);

    ofLogNotice("onSerialBuffer") << "received message:\t" << message.message;
}

//--------------------------------------------------------------
void ofApp::onSerialError(const ofx::IO::SerialBufferErrorEventArgs& args){
    // Errors and their corresponding buffer (if any) will show up here.
    SerialMessage message;
    message.message = args.buffer().toString();
    message.exception = args.exception().displayText();
    //serial_messages.push_back(message);

    ofLogNotice("onSerialError") << "got serial error : " << message.exception;
}

//--------------------------------------------------------------
void ofApp::exit(){
	gui_stepper_x_pos.removeListener(this,&ofApp::on_stepper_x_pos_changed);
	gui_stepper_y_pos.removeListener(this,&ofApp::on_stepper_y_pos_changed);
	gui_send_command.removeListener(this,&ofApp::on_send_command_pressed);
    serial_device.unregisterAllEvents(this);
}