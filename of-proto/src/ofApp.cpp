#include "ofApp.h"
#include <sstream>

// using namespace ofxCv;

//--------------------------------------------------------------
void ofApp::setup(){

    current_command_index = 0;

    // face tracking vars
    update_servo = true;
    send_servo_start_command = true;
    face_detected = false;
    // painting process vars
    button_pressed = false;
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

    // FACE TRACKING
    tracker.setup();

    // SERIAl
    // connect to the paintball machine

    // connect to the arduino
    std::vector<ofxIO::SerialDeviceInfo> devices_info = ofxIO::SerialDeviceUtils::listDevices();

    // log available devices
    ofLogNotice() << "serial devices:";
    for (auto device : devices_info){
        ofLogNotice() << "\t" << device;
    }

    if (!devices_info.empty()){

        // first device is the arduino for the paintball, second is the one for the face tracking
        bool success_1 = device.setup(devices_info[0], BAUD_RATE);
        bool success_2 = servo_cam_serial_device.setup(devices_info[1], BAUD_RATE);
        
        if (success_1){
            device.registerAllEvents(this);
            ofLogNotice("ofApp::setup") << "Successfully setup " << devices_info[0];
        }
        else {
            ofLogError("ofApp::setup") << "Unable to setup " << devices_info[0];
        }

        if (success_2){
            servo_cam_serial_device.registerAllEvents(this);
            ofLogNotice("ofApp::setup") << "Successfully setup " << devices_info[1];
        }
        else {
            ofLogError("ofApp::setup") << "Unable to setup " << devices_info[1];
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

    seconds_elapsed = (int) (floor(ofGetElapsedTimef()) + 1) % 60; // I don't need the actual amount, but just the timing

    // SERVO
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
        // if (!button_pressed){

            midtones_dots_positions.clear();
            darker_dots_positions.clear();

            // ofPixels & pixels = video_grabber->getGrabber<ofxPS3EyeGrabber>()->getPixels();

            color_img.setFromPixels(grabber_pixels);

            // threshold the image using two different levels
            // blue dots
            thresholded_img_1 = color_img;
            thresholded_img_1.threshold(FIRST_THRESHOLD);

            // orange dots
            thresholded_img_2 = color_img;
            thresholded_img_2.threshold(SECOND_THRESHOLD);

            // create the DOTS IMAGE
            // convert the image to a matrix of dots
            dots_fbo.begin();

            int num_dots = 0;
            
            ofClear(255);
            ofBackground(255);
            ofFill();

            ofPixels thresh_pixels_1 = thresholded_img_1.getPixels();
            ofPixels thresh_pixels_2 = thresholded_img_2.getPixels();

            int search_radius = 300;

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
                            // otherwise use orange dots
                            else {
                                ofSetColor(ofColor::orange);
                                ofDrawCircle(x, y, circle_size);
                                num_dots++;
                                glm::vec2 current_pos(x, y);
                                midtones_dots_positions.push_back(current_pos);
                            }
                        }
                        // use blue dots
                        else {
                            ofSetColor(ofColor::blue);
                            ofDrawCircle(x, y, circle_size);
                            num_dots++;
                            glm::vec2 current_pos(x, y);
                            darker_dots_positions.push_back(current_pos);
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

    // show the live feed is "s" is pressed
    if (show_live_feed) 
    // if (show_live_feed) video_grabber->draw(0, 0, 320, 240);

    if (APP_DEBUG) ofDrawBitmapStringHighlight("elapsed seconds: " + ofToString(seconds_elapsed), ofPoint(300, 60));

    if (APP_DEBUG) ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 100, 20);

    // if a face is detected
    if(tracker.getFound()) {

        dots_fbo.draw(0, 0);

        face_detected = true;

        ofSetColor(255);
        ofNoFill();
        
        int width = 200;
        int height = 200;

        if (APP_DEBUG) ofDrawRectangle(tracked_face_position.x-width/2, tracked_face_position.y-height/2, width, height);
        
        glm::vec2 screen_center = glm::vec2(ofGetWidth()/2, ofGetHeight()/2);

        float current_distance = tracked_face_position.y - screen_center.y;

        ofDrawLine(tracked_face_position, screen_center);

        ofDrawBitmapStringHighlight("face position: " + ofToString(tracked_face_position), ofPoint(100, 15));
        // ofDrawBitmapStringHighlight("required angle: " + ofToString(required_angle), ofPoint(300, 30));
        ofDrawBitmapStringHighlight("current distance: " + ofToString(current_distance), ofPoint(100, 30));
        ofDrawBitmapStringHighlight("orange dots: " + ofToString(midtones_dots_positions.size()), ofPoint(100, 45));
        ofDrawBitmapStringHighlight("grid size: " + ofToString(cam_width / circle_size*2), ofPoint(100, 60));

        // every 2 seconds, but first wait to have moved the servo for the first time
        if (seconds_elapsed % SERVO_UPDATE_DELAY == 0 && update_servo && !send_servo_start_command){

            ofLogNotice() << "\tupdate servo: " << update_servo << " at: " << seconds_elapsed << " seconds";
            
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
                /* stringstream ss;
                ss << "s" << gui_servo_angle;
                ofxIO::ByteBuffer servo_command(ss.str());
                ofLogNotice() << "sending " << ss.str();
                servo_cam_serial_device.send(servo_command); */
            }
            update_servo = false;
        }
    }
    else {
        face_detected = false;
        img_for_tracker.draw(0, 0);
    }

    gui.draw();

    // we want to update the servo only 1 time every 2 seconds, not 60 times every 2 seconds
    if (seconds_elapsed % SERVO_UPDATE_DELAY == 1) update_servo = true;

    /* std::stringstream ss;

    ss << " App FPS: " << ofGetFrameRate() << std::endl;
    ss << " Cam FPS: " << video_grabber->getGrabber<ofxPS3EyeGrabber>()->getFPS() << std::endl;
    ss << "Real FPS: " << video_grabber->getGrabber<ofxPS3EyeGrabber>()->getActualFPS() << std::endl;
    ss << "      id: 0x" << ofToHex(video_grabber->getGrabber<ofxPS3EyeGrabber>()->getDeviceId());

    ofDrawBitmapStringHighlight(ss.str(), ofPoint(10, 15)); */
}

//--------------------------------------------------------------
void ofApp::exit(){
    device.unregisterAllEvents(this);
    servo_cam_serial_device.unregisterAllEvents(this);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'b'){
        button_pressed = !button_pressed;
        
        if (button_pressed){
            // export_dots_to_csv(midtones_dots_positions, "red_dots.csv");
            // export_dots_to_csv(darker_dots_positions, "black_dots.csv");
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

    auto pos = midtones_dots_positions.at(i);

    // Create a byte buffer.
    // ofx::IO::ByteBuffer buffer('M' + ofToString(pos.x) + ',' + ofToString(pos.y));

    std::ostringstream ss;
    // add 3 leading zeros
    ss << "MX" << std::setw(3) << std::setfill('0') << pos.x;
    ss << "Y" << std::setfill('0') << pos.y;

    sent_command = ss.str();

    ofLogNotice() << "sending " << sent_command << ", " << current_command_index+1 << "/" << ofToString(midtones_dots_positions.size());

    ofx::IO::ByteBuffer buffer(sent_command);

    // Send the byte buffer.
    // ofx::IO::PacketSerialDevice will encode the buffer, send it to the
    // receiver, and send a packet marker.
    device.send(buffer);

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
    
    if (current_command_index <= midtones_dots_positions.size() - 1){
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