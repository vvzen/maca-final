#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    button_pressed = false;

    // get back a list of grabbing devices
    vector<ofVideoDevice> devices = video_grabber.listDevices();

    for (ofVideoDevice device : devices){
        if(device.bAvailable){
            // log the device
            ofLogNotice() << device.id << " : " << device.deviceName << " - available";
        }
        else {
            // log the device and note it as unavailable
            ofLogNotice() << device.id << ": " << device.deviceName << " - unavailable ";
        }
    }

    ofLogNotice() << "circle size:" << circle_size;
    ofLogNotice() << "horizontal dots: " << cam_width / circle_size;
    ofLogNotice() << "vertical dots:   " << cam_height / circle_size;

    // input video
    video_grabber.setDeviceID(0);
    video_grabber.setDesiredFrameRate(60);
    video_grabber.initGrabber(cam_width, cam_height);

    dots_fbo.allocate(cam_width, cam_height, GL_RGBA, 8);

    // OPENCV
    // dots
    color_img.allocate(cam_width, cam_height);
    thresholded_img_1.allocate(cam_width, cam_height);
    thresholded_img_2.allocate(cam_width, cam_height);
    
    ofSetVerticalSync(true);
}

//--------------------------------------------------------------
void ofApp::update(){

    ofBackground(255);
    
    video_grabber.update();

    if (video_grabber.isFrameNew()){

        if (!button_pressed){

            red_dots_positions.clear();
            black_dots_positions.clear();

            ofPixels & pixels = video_grabber.getPixels();

            color_img.setFromPixels(pixels);
            color_img.mirror(false, true);

            // threshold the image using two different levels
            thresholded_img_1 = color_img;
            thresholded_img_1.threshold(60);

            thresholded_img_2 = color_img;
            thresholded_img_2.threshold(100);

            // DOTS IMAGE
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
                    // if color is white then look at the other threholded image
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
                    // use black dots
                    else {
                        ofSetColor(ofColor::black);
                        ofDrawCircle(x, y, circle_size);
                        num_dots++;
                        glm::vec2 current_pos(x, y);
                        black_dots_positions.push_back(current_pos);
                    }
                }
            }

            dots_fbo.end();
            ofLogNotice() << "num dots: " << num_dots;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    //color_img.draw(cam_width, 0);
    // thresholded_img_1.draw(0, 0);
    // thresholded_img_2.draw(cam_width, 0);
    dots_fbo.draw(0, 0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' '){
        button_pressed = !button_pressed;
        if (button_pressed){
            export_dots_to_csv(red_dots_positions, "red_dots.csv");
            export_dots_to_csv(black_dots_positions, "black_dots.csv");
        }
    }
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