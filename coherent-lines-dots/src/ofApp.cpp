#include "ofApp.h"
#include "mbc.h"

// using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetLogLevel(OF_LOG_VERBOSE);

	// set the logging to a file
	// ofLogToFile("paintball.log");
	
	// FACE TRACKING
    face_tracker.setup();

	// save start time
	start_time = std::chrono::steady_clock::now();

	// init vars
	draw_dots = false;
	start_button_pressed = false;
	button_pressed_time = 0;

	// setup PS3 eye camera
	video_grabber.setDeviceID(0);
	video_grabber.initGrabber(cam_width, cam_height);

	// connect to the 2 arduinos
	init_serial_devices(cnc_device);

	current_command_index = 0;

	dots_fbo.allocate(cam_width, cam_height, GL_RGBA, 8);
	input_image.allocate(cam_width, cam_height, OF_IMAGE_GRAYSCALE);

	ofLogNotice() << "input_image: " << input_image.getWidth() << "x" << input_image.getHeight();

	ofLogNotice() << "10mm in pixels: " << ofMap(10, 0, MACHINE_X_MAX_POS, 0, cam_width, true);
}

//--------------------------------------------------------------
void ofApp::update(){
	
	// update PS3 eye camera
	video_grabber.update();
	if (video_grabber.isFrameNew() && !draw_dots){

		ofPixels & grabber_pixels = video_grabber.getPixels();
		input_image.setFromPixels(grabber_pixels);
		input_image.setImageType(OF_IMAGE_GRAYSCALE);
		input_image.mirror(false, true);

		// track face
        face_tracker.update(ofxCv::toCv(input_image));

        // update the tracked face position
        tracked_face_position = face_tracker.getPosition();
	}

	// Save the time when the button is pressed
	if (start_button_pressed){

		button_pressed_time = (int) ofGetElapsedTimef();
		ofLogNotice() << "button pressed at: " << button_pressed_time << " seconds";
	
		int elapsed_seconds = (int) ofGetElapsedTimef();

		// wait a bit before sending stuff to serial
		while (elapsed_seconds < button_pressed_time + SERIAL_INITIAL_DELAY_TIME){
			// ofLogVerbose() << "elapsed time: " << elapsed_seconds;
			elapsed_seconds = (int) ofGetElapsedTimef();
		}

		ofLogNotice() << "sending home";
			
		ofxOscMessage osc_message;
		osc_message.setAddress("/home");
		osc_message.addIntArg(1);

		send_osc_bundle(osc_message, cnc_device, 1024);

		draw_dots = true;
	}

	start_button_pressed = false;
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(ofColor::white);

	if (!draw_dots){
		input_image.draw(0, 0);
		ofPushStyle();
		ofNoFill();
		ofDrawRectangle(tracked_face_position.x - INTEREST_RADIUS/2, tracked_face_position.y - INTEREST_RADIUS/2, INTEREST_RADIUS, INTEREST_RADIUS);
		ofPopStyle();
	}
	else {
		//output_image.draw(0, 0);
		dots_fbo.draw(0, 0);
	}

	ofDrawBitmapStringHighlight("Coherent line drawing", 10, 20);
	ofDrawBitmapStringHighlight("Number of dots: " + ofToString(sorted_dots.size()), 10, 35);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	if (key == ' '){
		start_button_pressed = true;
		run_coherent_line_drawing(input_image, output_image, dots_fbo);
	}
}

//--------------------------------------------------------------
// SERIAL
//--------------------------------------------------------------
void ofApp::init_serial_devices(ofxIO::SLIPPacketSerialDevice &cnc){

	std::vector<ofxIO::SerialDeviceInfo> devices_info = ofxIO::SerialDeviceUtils::listDevices();

	// log connected devices
    ofLogNotice("ofApp::setup") << "Connected devices: ";
    for (std::size_t i = 0; i < devices_info.size(); ++i){
		ofLogNotice("ofApp::setup") << "\t" << devices_info[i];
	}

	if (!devices_info.empty()){

        // Connect the devices
        bool success_1 = cnc.setup(devices_info[0], BAUD_RATE);

        if (success_1){
            cnc.registerAllEvents(this);
            ofLogNotice("ofApp::setup") << "Successfully setup " << devices_info[0];
        }
        else ofLogError("ofApp::setup") << "Unable to setup " << devices_info[0];
    }
    else ofLogNotice("ofApp::setup") << "No devices connected.";
}

//--------------------------------------------------------------
void ofApp::send_current_command(int i){

    glm::vec2 pos = sorted_dots.at(i);

    ofxOscMessage osc_message;
	osc_message.setAddress("/stepper");
	osc_message.addIntArg(pos.x);
	osc_message.addIntArg(pos.y);

    ofLogNotice("send_current_command") << "/stepper " << pos.x << " " << pos.y << " - " << current_command_index+1 << "/" << ofToString(sorted_dots.size());

    // check onSerialBuffer() to see what happens after we sent a command
	send_osc_bundle(osc_message, cnc_device, 1024);
}

//--------------------------------------------------------------
// TSP
//--------------------------------------------------------------
// use evolutionary algorithms to evolve a good path for the cnc
// @args: 	in_points  --> the points used to compute the path optimization
// 	  		out_points --> a vector that will be filled with the sorted points
//--------------------------------------------------------------
int ofApp::solve_tsp(const vector<glm::vec2> & in_points, vector<glm::vec2> & out_points){

	// creates the graph, parameters: number of vertexes and initial vertex
	// TODO: use a smart pointer
	Graph * graph = new Graph(in_points.size(), 0);

    // for each point compute the distance to every other point
    for (int i = 0; i < in_points.size(); i++){
        auto p = in_points.at(i);
        
        for (int j = 0; j < in_points.size(); j++){
            
            if (i != j){
                auto next_p = in_points.at(j);
                int cost = round(ofDist(p.x, p.y, next_p.x, next_p.y));
                graph->addEdge(i, j, cost);
            }
        }
    }
	
	// parameters: the graph, population size, generations and mutation rate
	// optional parameters: show_population
	Genetic genetic(graph, 20, 2000, 10, false);

	const clock_t begin_time = clock(); // gets time
	genetic.run(); // runs the genetic algorithm
	ofLogNotice() << "Genetic algorithm, elapsed time: " << float(clock () - begin_time) /  CLOCKS_PER_SEC << " seconds"; // shows time in seconds
    
    // add the resulting points to the given vector
    const vector<int>& points_vec = genetic.population[0].first;
	for(int i = 0; i < graph->V; i++){
        glm::vec2 p = in_points.at(points_vec.at(i));
        out_points.push_back(p);
    }

    // free memory
    delete graph;

    // return the length of the optimised path
    return genetic.getCostBestSolution();
}

//--------------------------------------------------------------
// OPENCV
//--------------------------------------------------------------
// run the coherent line algorithm on a given image
// @args:	in			--> the input image
// @args:	out			--> the output image (b & w) obtained from CLD
// @args:	dots_fbo	--> an fbo on top of which we will draw the dots
//--------------------------------------------------------------
void ofApp::run_coherent_line_drawing(const ofImage &in, ofImage &out, ofFbo &dots_fbo){
	
	// Reset vector
	dots.clear();
	sorted_dots.clear();

	// Do the coherent line drawing magic
	ofxCv::CLD(input_image, output_image, halfw, smooth_passes, sigma1, sigma2, tau, black);
	ofxCv::invert(output_image);
	ofxCv::threshold(output_image, threshold);
	output_image.update();
	
	// Draw the dots on their fbo
	int circle_size = ofMap(20, 0, MACHINE_X_MAX_POS, 0, cam_width);
	ofLogNotice() << "circle size: " << circle_size;
	int sampling_size = circle_size;

	// dots_fbo.begin();

	// MBC mbc;
	// mbc.setup(&output_image, circle_size/2, &dots_fbo, ofColor::orange);
	// int c = 0;
	
	// mbc.generate_all_samples(300);

	// ofLogNotice() << "placed samples: " << mbc.placed_circles.size();

	// for (auto pc : mbc.placed_circles){
	// 	dots.push_back(glm::vec2(pc.pos.x, pc.pos.y));
	// }

	// dots_fbo.end();

	
	dots_fbo.begin();

	// Since I can only load ~300 shots on the gun, for safety reasons I'm constraining the max number of dots
	int max_dots = 300;


	// Sample the pixels from the coherent line image
	// and add dots if we found a white pixel
	for (int x = circle_size/2; x < output_image.getWidth(); x+= sampling_size){
		for (int y = circle_size/2; y < output_image.getHeight(); y+= sampling_size){

			// randomize sampling size
			// sampling_size = floor(ofRandom(0,6)) ? circle_size * 2 : circle_size;
			
			cout << "sampling size: " << sampling_size << endl;

			if (dots.size() < max_dots){
				if (ofDist(x, y, tracked_face_position.x, tracked_face_position.y) < INTEREST_RADIUS){
					ofColor c = output_image.getColor(x, y);

					if (c.r == 255){
						ofSetColor(ofColor::orange);
						ofDrawCircle(x, y, circle_size/2);
						// ofDrawRectangle(x, y, circle_size, circle_size);
						// map the position from pixels to mm
						glm::vec2 mapped_pos;
						mapped_pos.x = ofMap(x, 0, cam_width, 0, MACHINE_X_MAX_POS, true);
						mapped_pos.y = ofMap(y, 0, cam_height, 0, MACHINE_Y_MAX_POS, true);
						dots.push_back(glm::vec2(round(mapped_pos.x), round(mapped_pos.y)));
					}
				}
			}
			else {
				break;
			}
		}
	}

	dots_fbo.end();
	

	// Optimize the path using genetic algorithms
	ofLogNotice("run_coherent_line_drawing()") << "optimizing path using genetic algorithms";
	int overall_path_length = solve_tsp(dots, sorted_dots);

	ofLogNotice("run_coherent_line_drawing") << "overall length of the portrait: " << overall_path_length / 1000 << "m";

	// for debug, save the points to a csv file
	ofFile dots_file("dots.csv", ofFile::WriteOnly);
	for (auto d : sorted_dots){
		dots_file << d.x << ',' << d.y << endl;
	}

	// just add a final dot on the bottom left corner - the artist signature!
	float bottom_left_y = ofMap(cam_height, 0, cam_height, 0, MACHINE_Y_MAX_POS, true);
	dots.push_back(glm::vec2(0, bottom_left_y));

	ofLogNotice("run_coherent_line_drawing()") << "completed";

	ofLogNotice("dots size: ") << sorted_dots.size();
}

//--------------------------------------------------------------
// OSC
//--------------------------------------------------------------
void ofApp::append_message(ofxOscMessage &message, osc::OutboundPacketStream &p){

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
void ofApp::send_osc_bundle(ofxOscMessage &m, ofxIO::SLIPPacketSerialDevice &device, int buffer_size){
    // this codes come from ofxOscSender::sendMessage in ofxOscSender.cpp
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
void ofApp::onSerialBuffer(const ofx::IO::SerialBufferEventArgs &args){
    
	std::string received_command = args.buffer().toString();
	ofLogNotice("onSerialBuffer") << "received message --> " << received_command;

	// remove \r , \n and \0 chars from received message
	received_command.erase(std::remove(received_command.begin(), received_command.end(), '\n'), received_command.end());
	received_command.erase(std::remove(received_command.begin(), received_command.end(), '\r'), received_command.end());
	received_command.erase(std::remove(received_command.begin(), received_command.end(), '\0'), received_command.end());

	// filter out the switch debugging messages
	if (received_command.substr(0, 6) == "switch"){
		// TODO:
	}
	else {
		if (received_command == "home"){
			
			ofLogNotice("onSerialBuffer") << "homing done, starting";
			// start the painting by sending the values over serial
			send_current_command(current_command_index);
		}
		else {
			// check if we need to send more messages
			if (current_command_index <= sorted_dots.size() - 2){

				// The arduino sends us back a string formatted like that: "stepperx:valuey:value"
				// so we recreate artificially a similar string and we check if it's equal to the arduino message
				std::string sent_message = "";
				glm::vec2 current_pos = sorted_dots.at(current_command_index);
				sent_message += "stepperx:" + ofToString(current_pos.x) + "y:" + ofToString(current_pos.y);

				ofLogNotice("onSerialBuffer") << "current index: " << current_command_index;
				ofLogNotice("onSerialBuffer") << "sent:     " << sent_message;
				ofLogNotice("onSerialBuffer") << "received: " << received_command;

				// if arduino received the same message that we sent then send the next message
				if (received_command == sent_message){
					ofLogNotice("onSerialBuffer") << "all good";
					send_current_command(++current_command_index);
				}
				else {
					ofLogError("onSerialBuffer") << "we received a different message than the sent one";
				}
			}
			// END of the painting
			else {
				ofLogNotice("onSerialBuffer") << "sent all commands!";
				current_command_index = 0;

				// get elapsed time
				auto end_time = std::chrono::steady_clock::now();
				auto elapsed_micros = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
				
				int elapsed_seconds = (elapsed_micros / 1000000);
				int elapsed_minutes = elapsed_seconds / 60;
				ofLogNotice() << "elapsed time: " << elapsed_minutes << ":" << elapsed_seconds % 60;

				std::exit(0);
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::onSerialError(const ofx::IO::SerialBufferErrorEventArgs &args){
    // Errors and their corresponding buffer (if any) will show up here.
    ofLogError("onSerialError") << "Serial error : " << args.exception().displayText();
}

//--------------------------------------------------------------
// EXIT
//--------------------------------------------------------------
void ofApp::exit(){
	cnc_device.unregisterAllEvents(this);
	// cam_servo_device.unregisterAllEvents(this);
}