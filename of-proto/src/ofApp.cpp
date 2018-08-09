#include "ofApp.h"
#include <sstream>

using namespace ofxCv;

//--------------------------------------------------------------
void ofApp::setup(){

    current_command_index = 0;

    update_servo = true;
    send_servo_start_command = true;
    button_pressed = false;
    face_detected = false;
    show_live_feed = false;

    ofLogNotice() << "circle size:" << circle_size;
    ofLogNotice() << "horizontal dots: " << cam_width / circle_size;
    ofLogNotice() << "vertical dots:   " << cam_height / circle_size;

    // GUI
    gui.setup();
    gui.add(gui_servo_angle.set("servo angle", 10, 10, 90));
    gui_servo_angle.addListener(this, &ofApp::on_servo_angle_changed);

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
    img_for_tracker.allocate(cam_width, cam_height, OF_IMAGE_COLOR);

    // FACE TRACKING
    tracker.setup();

    // SERIAl
    // camera servo
    std::vector<ofxIO::SerialDeviceInfo> devices_info = ofxIO::SerialDeviceUtils::listDevices();
    ofLogNotice("ofApp::setup") << "Connected Devices: ";

    for (std::size_t i = 0; i < devices_info.size(); ++i){
        ofLogNotice("ofApp::setup") << "\t" << devices_info[i];
    }

    if (!devices_info.empty()){

        // Connect to the first matching device.
        bool success = servo_cam_serial_device.setup(devices_info[0], BAUD_RATE);

        if (success){
            servo_cam_serial_device.registerAllEvents(this);
            ofLogNotice("ofApp::setup") << "Successfully setup " << devices_info[0];
        }
        else ofLogNotice("ofApp::setup") << "Unable to setup " << devices_info[0];
    }
    else ofLogNotice("ofApp::setup") << "No devices connected.";

    // cnc machine: connect to the arduino
    /* std::vector<ofxIO::SerialDeviceInfo> devices_info = ofxIO::SerialDeviceUtils::listDevices();
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
    } */

    ofSetVerticalSync(true);
    ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD);
}

//--------------------------------------------------------------
void ofApp::update(){

    ofBackground(255);

    seconds_elapsed = floor(ofGetElapsedTimef()) + 1;
    ofLogNotice() << "seconds elapsed: " << seconds_elapsed;

    // wait 10 seconds and send the first command to the servo
    if ((seconds_elapsed % 8 == 0) && send_servo_start_command){
        
        gui_servo_angle = SERVO_START_POSITION;
        
        std::stringstream servo_start_command;
        servo_start_command << 's' << ofToString(gui_servo_angle);
        
        ofxIO::ByteBuffer buffer(servo_start_command.str());
        servo_cam_serial_device.send(buffer);
        
        ofLogNotice() << "SERVO START COMMAND";

        send_servo_start_command = false;
    }

    // VIDEO
    video_grabber->update();

    if (video_grabber->isFrameNew()){

        // convert video grabber to ofImage and send it to the face tracker
        ofPixels & grabber_pixels = video_grabber->getGrabber<ofxPS3EyeGrabber>()->getPixels();
        img_for_tracker.setFromPixels(grabber_pixels);
        tracker.update(toCv(img_for_tracker));
        
        // update the tracked face position
        tracked_face_position = tracker.getPosition();
        
        if (face_detected){

            red_dots_positions.clear();
            black_dots_positions.clear();

            color_img.setFromPixels(grabber_pixels);

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

            int search_radius = 200;

            for (float x = circle_size/2; x < cam_width; x += circle_size*2){
                for (float y = circle_size/2; y < cam_height; y += circle_size*2){

                    // only care for pixels close to the current tracked face
                    if (ofDist(x, y, tracked_face_position.x, tracked_face_position.y) <= search_radius) {

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

    img_for_tracker.draw(0, 0);

    ofDrawBitmapStringHighlight("elapsed seconds: " + ofToString(seconds_elapsed), ofPoint(300, 60));

    gui.draw();

    ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 100, 20);
    
    // if a face is detected
    if(tracker.getFound()) {

        face_detected = true;

        ofSetColor(255);
        ofNoFill();
        
        int width = 200;
        int height = 200;

        ofDrawRectangle(tracked_face_position.x-width/2, tracked_face_position.y-height/2, width, height);
        
        glm::vec2 screen_center = glm::vec2(ofGetWidth()/2, ofGetHeight()/2);

        float current_distance = tracked_face_position.y - screen_center.y;

        ofDrawLine(tracked_face_position, screen_center);

        ofDrawBitmapStringHighlight("face position: " + ofToString(tracked_face_position), ofPoint(300, 15));
        // ofDrawBitmapStringHighlight("required angle: " + ofToString(required_angle), ofPoint(300, 30));
        ofDrawBitmapStringHighlight("current distance: " + ofToString(current_distance), ofPoint(300, 45));

        // every 2 seconds, but first wait to have moved the servo for the first time
        if (seconds_elapsed % SERVO_UPDATE_DELAY == 0 && update_servo && !send_servo_start_command){

            cout << "HERE!\t" << "update servo: " << update_servo << endl;
            
            // if the face is too distant from the center
            if (abs(current_distance) >= FACE_DISTANCE_THRESHOLD){
                bool is_negative = current_distance < 0;
                
                if (is_negative){
                    // increase current angle
                    gui_servo_angle += 2;
                }
                else {
                    // decrease current angle
                    gui_servo_angle -= 2;
                }

                // prepend an s to the command (ie: s40)
                stringstream ss;
                ss << "s" << gui_servo_angle;
                ofxIO::ByteBuffer servo_command(ss.str());
                ofLogNotice() << "sending " << ss.str();
                servo_cam_serial_device.send(servo_command);
            }
            update_servo = false;
        }
    }
    else {
        face_detected = false;
    }

    // we want to update the servo only 1 time every 2 seconds, not 60 times every 2 seconds
    if (seconds_elapsed % SERVO_UPDATE_DELAY == 1) update_servo = true;

    /* dots_fbo.draw(0, 0);

    // show the live feed if "s" is pressed
    if (show_live_feed) video_grabber->draw(0, 0, 320, 240);

    std::stringstream ss;

    ss << " App FPS: " << ofGetFrameRate() << std::endl;
    ss << " Cam FPS: " << video_grabber->getGrabber<ofxPS3EyeGrabber>()->getFPS() << std::endl;
    ss << "Real FPS: " << video_grabber->getGrabber<ofxPS3EyeGrabber>()->getActualFPS() << std::endl;
    ss << "      id: 0x" << ofToHex(video_grabber->getGrabber<ofxPS3EyeGrabber>()->getDeviceId());

    ofDrawBitmapStringHighlight(ss.str(), ofPoint(10, 15)); */
}

//--------------------------------------------------------------
void ofApp::exit(){
    servo_cam_serial_device.unregisterAllEvents(this);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    // FIXME: use smile detection instead of button press
    switch(key){
        case 'b':{
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
                // send_current_command(current_command_index);
            }
            break;
        }
        // case 's':{
        //     show_live_feed = !show_live_feed;
        //     break;
        // }
        case 'r': {
            tracker.reset();
            break;
        }
        case 'a': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s56");
            ofLogNotice() << "sending s56";
            
            gui_servo_angle = 56;
            servo_cam_serial_device.send(buffer);
            break;
        }
        case 's': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s60");
            ofLogNotice() << "sending s60";
            gui_servo_angle = 60;
            servo_cam_serial_device.send(buffer);
            break;
        }
        case 'd': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s64");
            ofLogNotice() << "sending s64";
            servo_cam_serial_device.send(buffer);
            break;
        }
        case 'f': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s68");
            ofLogNotice() << "sending s68";
            servo_cam_serial_device.send(buffer);
            break;
        }
        case 'g': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s72");
            ofLogNotice() << "sending s72";
            servo_cam_serial_device.send(buffer);
            break;
        }
        case 'h': {
            // Create a byte buffer.
            ofxIO::ByteBuffer buffer("s76");
            ofLogNotice() << "sending s76";
            servo_cam_serial_device.send(buffer);
            break;
        }
    }
    
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
    // device.send(buffer);

    // check onSerialBuffer to see what happens after we sent a command
}

//--------------------------------------------------------------
void ofApp::on_servo_angle_changed(int & servo_angle){

    // std::stringstream servo_command;
    // servo_command << "s" << servo_angle;

    // ofxIO::ByteBuffer buffer(servo_command.str());
    // ofLogNotice() << "sending " << servo_command.str();
    // servo_cam_serial_device.send(buffer);
}

//--------------------------------------------------------------
void ofApp::onSerialBuffer(const ofx::IO::SerialBufferEventArgs& args){
    
    // Decoded serial packets will show up here.
    received_command = args.buffer().toString();

    ofLogNotice() << "received message: " << received_command;

    // check if the received message is the one we sent
    // if yes, then send the next message
    
    /* if (current_command_index <= red_dots_positions.size() - 1){
        if (sent_command == received_command){
            send_current_command(++current_command_index);
        }
    }
    else {
        ofLogNotice() << "sent all commands!";
        current_command_index = 0;
    } */
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
