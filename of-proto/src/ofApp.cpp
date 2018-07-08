#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    should_grab = false;

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

    video_grabber.setDeviceID(0);
    video_grabber.setDesiredFrameRate(60);
    video_grabber.initGrabber(cam_width, cam_height);

    input_img.allocate(cam_width, cam_height, OF_IMAGE_COLOR);
    dots_fbo.allocate(cam_width, cam_height, GL_RGBA, 8);
    // opencv
    color_img.allocate(cam_width, cam_height);
    thresholded_img.allocate(cam_width, cam_height);
    ofSetVerticalSync(true);
}

//--------------------------------------------------------------
void ofApp::update(){

    ofBackground(255);
    
    video_grabber.update();

    if (video_grabber.isFrameNew()){

        if (should_grab){
            ofPixels & pixels = video_grabber.getPixels();
            color_img.setFromPixels(pixels);

            // TODO: threshold the image
            thresholded_img = color_img;
            thresholded_img.threshold(80);


            // DOTS IMAGE
            // convert the image to a matrix of dots
            dots_fbo.begin();
            // TODO: find biggest circle good for both width and height
            float circle_size = cam_width / 160;
            
            ofClear(255);
            ofBackground(255);
            ofSetColor(ofColor::black);
            ofFill();

            ofPixels thresh_pixels = thresholded_img.getPixels();

            for (float x = circle_size/2; x < cam_width; x += circle_size*2){
                for (float y = circle_size/2; y < cam_height; y += circle_size*2){
                    
                    ofColor c = thresh_pixels.getColor(x, y);
                    ofSetColor(c);

                    // take the color for the circle from the thresholded image
                    ofDrawCircle(x, y, circle_size);
                }
            }
            dots_fbo.end();

            should_grab = false;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    input_img.draw(0, 0);
    video_grabber.draw(cam_width, 0);
    thresholded_img.draw(0, cam_height);
    dots_fbo.draw(cam_width, cam_height);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    should_grab = true;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
