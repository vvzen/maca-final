#include "ofApp.h"
#include <sstream>

//--------------------------------------------------------------
void ofApp::setup(){

    current_command_index = 0;

    button_pressed = false;
    show_live_feed = false;

    ofLogNotice() << "circle size:" << circle_size;
    ofLogNotice() << "horizontal dots: " << cam_width / circle_size;
    ofLogNotice() << "vertical dots:   " << cam_height / circle_size;

    // Load the JSON with the video settings from a configuration file.
    ofJson config = ofLoadJson("settings.json");

    // Create a grabber from the JSON.
    video_grabber = ofxPS3EyeGrabber::fromJSON(config);

    dots_fbo.allocate(cam_width, cam_height, GL_RGBA, 8);

    // OPENCV
    // dots
    color_img.allocate(cam_width, cam_height);
    thresholded_img_1.allocate(cam_width, cam_height);
    thresholded_img_2.allocate(cam_width, cam_height);

    // SERIAl
    // connect to the arduino
    std::vector<ofxIO::SerialDeviceInfo> devices_info = ofxIO::SerialDeviceUtils::listDevices();
    if (!devices_info.empty()){
        // connect to the first matching device
        bool success = device.setup(devices_info[0], BAUD_RATE);
        if (success){
            device.registerAllEvents(this);
            ofLogNotice("ofApp::setup") << "Successfully setup " << devices_info[0];
        }
        else {
            ofLogError("ofApp::setup") << "Unable to setup " << devices_info[0];
        }
    }
    else {
        ofLogError("ofApp::setup") << "No devices connected";
    }

    
    ofSetVerticalSync(true);
}

//--------------------------------------------------------------
void ofApp::update(){

    ofBackground(255);
    
    video_grabber->update();

    if (video_grabber->isFrameNew()){

        if (!button_pressed){

            red_dots_positions.clear();
            black_dots_positions.clear();

            ofPixels & pixels = video_grabber->getGrabber<ofxPS3EyeGrabber>()->getPixels();

            color_img.setFromPixels(pixels);

            // threshold the image using two different levels
            // blue dots
            thresholded_img_1 = color_img;
            thresholded_img_1.threshold(80);

            // red dots
            thresholded_img_2 = color_img;
            thresholded_img_2.threshold(110);

            // create the DOTS IMAGE
            // convert the image to a matrix of dots
            dots_fbo.begin();

            int num_dots = 0;
            
            ofClear(255);
            ofBackground(255);
            ofFill();

            ofPixels thresh_pixels_1 = thresholded_img_1.getPixels();
            ofPixels thresh_pixels_2 = thresholded_img_2.getPixels();

            for (float x = circle_size/2; x < cam_width; x += circle_size*2){
                for (float y = circle_size/2; y < cam_height; y += circle_size*2){
                    
                    ofColor c = thresh_pixels_1.getColor(x, y);
                    
                    // take the color for the circle from the first thresholded image
                    // if color is white then look at the other thresholded image
                    // else use blue
                    if (c.getLightness() == 255){

                        c = thresh_pixels_2.getColor(x, y);

                        // if thresh pixels 2 are white then use white (which will be the bg)
                        if (c.getLightness() == 255){
                            ofSetColor(ofColor::white);
                            ofDrawCircle(x, y, circle_size);
                        }
                        // otherwise use red dots
                        else {
                            ofSetColor(ofColor::red);
                            ofDrawCircle(x, y, circle_size);
                            num_dots++;
                            glm::vec2 current_pos(x, y);
                            red_dots_positions.push_back(current_pos);
                        }
                    }
                    // use blue dots
                    else {
                        ofSetColor(ofColor::blue);
                        ofDrawCircle(x, y, circle_size);
                        num_dots++;
                        glm::vec2 current_pos(x, y);
                        black_dots_positions.push_back(current_pos);
                    }
                }
            }

            dots_fbo.end();
            // ofLogNotice() << "num dots: " << num_dots;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    //color_img.draw(cam_width, 0);
    // thresholded_img_1.draw(0, 0);
    // thresholded_img_2.draw(cam_width, 0);
    dots_fbo.draw(0, 0);

    // show the live feed is "s" is pressed
    if (show_live_feed) video_grabber->draw(0, 0, 320, 240);

    std::stringstream ss;

    ss << " App FPS: " << ofGetFrameRate() << std::endl;
    ss << " Cam FPS: " << video_grabber->getGrabber<ofxPS3EyeGrabber>()->getFPS() << std::endl;
    ss << "Real FPS: " << video_grabber->getGrabber<ofxPS3EyeGrabber>()->getActualFPS() << std::endl;
    ss << "      id: 0x" << ofToHex(video_grabber->getGrabber<ofxPS3EyeGrabber>()->getDeviceId());

    ofDrawBitmapStringHighlight(ss.str(), ofPoint(10, 15));
}

//--------------------------------------------------------------
void ofApp::exit(){
    device.unregisterAllEvents(this);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'b'){
        button_pressed = !button_pressed;
        
        if (button_pressed){
            // export_dots_to_csv(red_dots_positions, "red_dots.csv");
            // export_dots_to_csv(black_dots_positions, "black_dots.csv");
            ofLogNotice() << "button pressed!";

            // people pressed the red button, fun is coming!
            // 1. let's start by telling the arduino we're starting

            /* send one command at a time using recursion instead of a loop
            * goes like that:
            * send the current message
            * check for arduino response
            * send next message
            */
            send_current_command(current_command_index);
        }
    }
    else if (key == 's') show_live_feed = !show_live_feed;
}

//--------------------------------------------------------------
void ofApp::send_current_command(int i){

    auto pos = red_dots_positions.at(i);

    // Create a byte buffer.
    // ofx::IO::ByteBuffer buffer('M' + ofToString(pos.x) + ',' + ofToString(pos.y));

    std::ostringstream ss;
    // add 3 leading zeros
    ss << "MX" << std::setw(3) << std::setfill('0') << pos.x;
    ss << "Y" << std::setfill('0') << pos.y;

    sent_command = ss.str();

    ofLogNotice() << "sending " << sent_command << ", " << current_command_index+1 << "/" << ofToString(red_dots_positions.size());

    ofx::IO::ByteBuffer buffer(sent_command);

    // Send the byte buffer.
    // ofx::IO::PacketSerialDevice will encode the buffer, send it to the
    // receiver, and send a packet marker.
    device.send(buffer);

    // check onSerialBuffer to see what happens after we sent a command
}


//--------------------------------------------------------------
void ofApp::onSerialBuffer(const ofx::IO::SerialBufferEventArgs& args){
    
    // Decoded serial packets will show up here.
    received_command = args.buffer().toString();

    ofLogNotice() << "received message: " << received_command;

    // check if the received message is the one we sent
    // if yes, then send the next message
    
    if (current_command_index <= red_dots_positions.size() - 1){
        if (sent_command == received_command){
            send_current_command(++current_command_index);
        }
    }
    else {
        ofLogNotice() << "sent all commands!";
        current_command_index = 0;
    }
}

//--------------------------------------------------------------
void ofApp::onSerialError(const ofx::IO::SerialBufferErrorEventArgs& args){
    // Errors and their corresponding buffer (if any) will show up here.
    SerialMessage message;
    message.message = args.buffer().toString();
    message.exception = args.exception().displayText();
    serial_messages.push_back(message);
}

//--------------------------------------------------------------
void ofApp::export_dots_to_csv(vector<glm::vec2> positions, std::string filename){
    
    ofFile csv_file(ofToDataPath(filename), ofFile::WriteOnly, false);

    csv_file << "start" << endl;
    
    for (auto pos : positions){
        csv_file << pos.x << "," << pos.y << endl;
    }

    csv_file << "end" << endl;
}