#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetLogLevel(OF_LOG_VERBOSE);

    // GUI
    gui.setup();
    gui.add(gui_stepper_x_pos.set("Stepper motor X position", X_MIN_POS, X_MIN_POS, X_MAX_POS));
    gui.add(gui_stepper_y_pos.set("Stepper motor Y position", Y_MIN_POS, Y_MIN_POS, Y_MAX_POS));
    gui.add(gui_send_move_command.setup("Send Command"));
    gui.add(gui_send_gethome_command.setup("Get Home"));
    gui.add(gui_shoot_command.setup("Shoot!"));

    gui_stepper_x_pos.addListener(this, &ofApp::on_stepper_x_pos_changed);
    gui_stepper_y_pos.addListener(this, &ofApp::on_stepper_y_pos_changed);
    gui_send_move_command.addListener(this, &ofApp::on_send_command_pressed);
    gui_send_gethome_command.addListener(this, &ofApp::on_send_gethome_pressed);
    gui_shoot_command.addListener(this, &ofApp::on_shoot_pressed);

    stepper_pos = glm::vec2(0, 0);
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

    int width = X_MAX_POS / 4;
    int height = Y_MAX_POS / 4;
    int pos_x = ofMap(stepper_pos.x, X_MIN_POS, X_MAX_POS, -width/2, width/2);
    int pos_y = ofMap(stepper_pos.y, Y_MIN_POS, Y_MAX_POS, -height/2, height/2);

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

    ofxOscMessage osc_message;
    osc_message.setAddress("/stepper");
    osc_message.addIntArg(stepper_pos.x);
    osc_message.addIntArg(stepper_pos.y);

    ofLogNotice() << "send command pressed!";
    ofLogNotice() << "command: " << "/stepper " << stepper_pos.x << " " << stepper_pos.y;

    send_osc_bundle(osc_message, 1024);
}

//--------------------------------------------------------------
void ofApp::on_send_gethome_pressed(){

    ofxOscMessage osc_message;
    osc_message.setAddress("/home");
    osc_message.addIntArg(1);

    send_osc_bundle(osc_message, 1024);
}

//--------------------------------------------------------------
void ofApp::on_shoot_pressed(){

    ofxOscMessage osc_message;
    osc_message.setAddress("/shoot");
    osc_message.addIntArg(1);

    ofLogNotice() << "command: " << "/shoot " << 1;

    send_osc_bundle(osc_message, 1024);
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
void ofApp::exit(){
	gui_stepper_x_pos.removeListener(this,&ofApp::on_stepper_x_pos_changed);
	gui_stepper_y_pos.removeListener(this,&ofApp::on_stepper_y_pos_changed);
	gui_send_move_command.removeListener(this,&ofApp::on_send_command_pressed);
    serial_device.unregisterAllEvents(this);
}