#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofEnableAlphaBlending();

    std::vector<ofxIO::SerialDeviceInfo> devicesInfo = ofxIO::SerialDeviceUtils::listDevices();

    ofLogNotice("ofApp::setup") << "Connected Devices: ";

    for (std::size_t i = 0; i < devicesInfo.size(); ++i){
        ofLogNotice("ofApp::setup") << "\t" << devicesInfo[i];
    }

    if (!devicesInfo.empty()){

        // Connect to the first matching device.
        bool success = device.setup(devicesInfo[0], 115200);

        if (success){
            device.registerAllEvents(this);
            ofLogNotice("ofApp::setup") << "Successfully setup " << devicesInfo[0];
        }
        else {
            ofLogNotice("ofApp::setup") << "Unable to setup " << devicesInfo[0];
        }
    }
    else {
        ofLogNotice("ofApp::setup") << "No devices connected.";
    }

}

//--------------------------------------------------------------
void ofApp::exit(){
    device.unregisterAllEvents(this);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);

    ofSetColor(255);

    std::stringstream ss;

    ss << "         FPS: " << ofGetFrameRate() << std::endl;
    ss << "Connected to: " << device.port();

    ofDrawBitmapString(ss.str(), ofVec2f(20, 20));

    std::vector<SerialMessage>::iterator iter = serialMessages.begin();

    int x = 20;
    int y = 50;
    int height = 20;

    while (iter != serialMessages.end()){

        iter->fade -= 20;

        if (iter->fade < 0){
            iter = serialMessages.erase(iter);
        }
        else {
            
            ofSetColor(255, ofClamp(iter->fade, 0, 255));
            ofDrawBitmapString(iter->message, ofVec2f(x, y));

            y += height;

            if (!iter->exception.empty())
            {
                ofSetColor(255, 0, 0, ofClamp(iter->fade, 0, 255));
                ofDrawBitmapString(iter->exception, ofVec2f(x + height, y));
                y += height;
            }

            ++iter;
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    switch(key){
        case 'a': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s56");

            // Send the byte buffer.
            // ofxIO::PacketSerialDevice will encode the buffer, send it to the
            // receiver, and send a packet marker.
            ofLogNotice() << "sending s56";
            device.send(buffer);
            break;
        }
        case 's': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s60");

            // Send the byte buffer.
            // ofxIO::PacketSerialDevice will encode the buffer, send it to the
            // receiver, and send a packet marker.
            ofLogNotice() << "sending s60";
            device.send(buffer);
            break;
        }
        case 'd': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s64");

            // Send the byte buffer.
            // ofxIO::PacketSerialDevice will encode the buffer, send it to the
            // receiver, and send a packet marker.
            ofLogNotice() << "sending s64";
            device.send(buffer);
            break;
        }
        case 'f': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s68");

            // Send the byte buffer.
            // ofxIO::PacketSerialDevice will encode the buffer, send it to the
            // receiver, and send a packet marker.
            ofLogNotice() << "sending s68";
            device.send(buffer);
            break;
        }
        case 'g': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s72");

            // Send the byte buffer.
            // ofxIO::PacketSerialDevice will encode the buffer, send it to the
            // receiver, and send a packet marker.
            ofLogNotice() << "sending s72";
            device.send(buffer);
            break;
        }
        case 'h': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s76");

            // Send the byte buffer.
            // ofxIO::PacketSerialDevice will encode the buffer, send it to the
            // receiver, and send a packet marker.
            ofLogNotice() << "sending s76";
            device.send(buffer);
            break;
        }
    }
}

//--------------------------------------------------------------
void ofApp::onSerialBuffer(const ofxIO::SerialBufferEventArgs& args){

    // Decoded serial packets will show up here.
    SerialMessage message;
    message.message = args.buffer().toString();
    serialMessages.push_back(message);

    ofLogNotice("onSerialBuffer") << "got serial buffer : " << message.message;
}


//--------------------------------------------------------------
void ofApp::onSerialError(const ofxIO::SerialBufferErrorEventArgs& args){

    // Errors and their corresponding buffer (if any) will show up here.
    SerialMessage message;
    message.message = args.buffer().toString();
    message.exception = args.exception().displayText();
    message.fade = 500;
    serialMessages.push_back(message);

    ofLogNotice("onSerialError") << "got serial error : " << message.exception;
}