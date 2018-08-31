#include "ofApp.h"

#include <algorithm>

//--------------------------------------------------------------
void ofApp::setup(){

    current_algorithm_view = 0;


    // init the distances
    default_distance = 0;
    nn_distance = 0;
    ga_tsp_distance = 0;
    
    load_points_from_csv(points, "dots.csv");

    // Fill the mesh with the points
    for (auto p : points){
        mesh.addVertex(glm::vec3(p.x, p.y, 0));
    }

    // try to create an 'optimized' path that goes through all of them
    // in the smallest time possible 

    // 0. BENCHMARK --> the default path we get by going from (0, 0) to (width, height)
    for (int i = 0; i < points.size()-1; i++){
        auto p = points.at(i);
        auto next_p = points.at(i+1);
        default_distance += ofDist(p.x, p.y, next_p.x, next_p.y); 
    }

    // CURRENTLY STILL TODO: AND WIP
    // 1. EASY ATTEMPT --> nearest neightbour
    // loop through each one of them (O(n**2)) and for each one find the nearest point
    points_nn_path.push_back(points.at(0));

    vector<glm::vec2> nn_visited_points = points;
    vector<glm::vec2> tmp_points = points;

    // Start from the first point
    auto p = points.at(0);
    int c = 0;

    while (c < points.size()){

        // Remove current point from the vector so that when finding the distance we don't compare it to itself
        tmp_points.erase(std::remove(tmp_points.begin(), tmp_points.end(), p), tmp_points.end());

        // Find the nearest point to the current point
        // (sort the points based on their distance from this current point)
        std::sort(tmp_points.begin(), tmp_points.end(), [p](const glm::vec2 &lhs, const glm::vec2 &rhs){
            return ofDist(p.x, p.y, lhs.x, lhs.y) < ofDist(p.x, p.y, rhs.x, rhs.y);
        });

        auto closest_p = tmp_points.at(0);

        // Check if we already visited this one
        if(std::find(nn_visited_points.begin(), nn_visited_points.end(), closest_p) != v.end()) {
            /* v contains x */
        } else {
            // make this point the next point
            p = closest_p;
            // add it to the visited points
            nn_visited_points.push_back(closest_p);
        }

        nn_distance += ofDist(p.x, p.y, tmp_points.at(0).x, tmp_points.at(0).y);

        points_nn_path.push_back(p);

        c++;

        // ofLogNotice() << "current:  " << ofToString(p);
        // ofLogNotice() << "closest:  " << ofToString(tmp_points.at(0));
        // ofLogNotice() << "farthest: " << ofToString(tmp_points.at(tmp_points.size()-1));
    }

    glPointSize(4);

    // 2. SMARTER ATTEMPT --> TSP using genetic algorithms
    ga_tsp_distance = solve_tsp(points, points_ga_tsp_path);

    ofLogNotice() << "default distance: " << default_distance;
    ofLogNotice() << "nn distance:      " << nn_distance;
    ofLogNotice() << "ga tsp distance:  " << ga_tsp_distance;
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(ofColor::white);

    mesh.drawVertices();

    ofSetColor(ofColor::black);

    // draw the path among the points
    if (current_algorithm_view == DEFAULT){
        ofDrawBitmapStringHighlight("default", 20, 20);
        ofDrawBitmapStringHighlight(ofToString(default_distance), 20, 35);
        for (int i = 0; i < points.size()-1; i++){
            auto p = points.at(i);
            auto next_p = points.at(i+1);
            ofDrawLine(p.x, p.y, next_p.x, next_p.y);
        }
    }
    else if (current_algorithm_view == NEAREST_NEIGHBOUR){
        ofDrawBitmapStringHighlight("nearest neighbour", 20, 20);
        ofDrawBitmapStringHighlight(ofToString(nn_distance), 20, 35);
        for (int i = 0; i < points_nn_path.size()-1; i++){
            auto p = points_nn_path.at(i);
            auto next_p = points_nn_path.at(i+1);
            ofDrawLine(p.x, p.y, next_p.x, next_p.y);
        }    
    }
    else if (current_algorithm_view == TSP_GENETIC){
        ofDrawBitmapStringHighlight("tsp genetic", 20, 20);
        ofDrawBitmapStringHighlight(ofToString(ga_tsp_distance), 20, 35);
        for (int i = 0; i < points_ga_tsp_path.size()-1; i++){
            auto p = points_ga_tsp_path.at(i);
            auto next_p = points_ga_tsp_path.at(i+1);
            ofDrawLine(p.x, p.y, next_p.x, next_p.y);
        }    
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key){
        case '1':{
            current_algorithm_view = DEFAULT;
            break;
        }
        case '2':{
            current_algorithm_view = NEAREST_NEIGHBOUR;
            break;
        }
        case '3':{
            current_algorithm_view = TSP_GENETIC;
            break;
        }
    }
}

//--------------------------------------------------------------
void ofApp::load_points_from_csv(vector<glm::vec2> & vec, std::string path){

    // load the points from the csv
    ofBuffer buffer = ofBufferFromFile(path);

    int i = 0;
    for (auto line : buffer.getLines()){
        //ofLogNotice() << i;

        std::istringstream ss(line);
        std::string token;

        glm::vec2 p;

        // split string using comma
        int c = 0;
        while(std::getline(ss, token, ',')) {
            int num = std::atoi(token.c_str());
            if (c == 0) p.x = num;
            if (c == 1) p.y = num;
            c++;
        }

        points.push_back(p);

        i++;
    }

    ofLogNotice() << "points size: " << points.size();
    ofLogNotice() << "first point: " << points.at(0);
}

//--------------------------------------------------------------
int ofApp::solve_tsp(const vector<glm::vec2> & in_points, vector<glm::vec2> & out_points){

    //srand(time(NULL)); // random numbers
	
	// creates the graph1 with parameters: number of vertexes and initial vertex
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
	Genetic genetic(graph, 10, 1000, 5, false);

	const clock_t begin_time = clock(); // gets time
	genetic.run(); // runs the genetic algorithm
	ofLogNotice() << "Genetic algorithm, elapsed time: " << float(clock () - begin_time) /  CLOCKS_PER_SEC << " seconds."; // shows time in seconds
    
    // add the resulting points
    const vector<int>& points_vec = genetic.population[0].first;
	for(int i = 0; i < graph->V; i++){

        //ofLogNotice() << "point at " << i << ": " << points_vec.at(i);

        glm::vec2 p = in_points.at(points_vec.at(i));
        out_points.push_back(p);
    }

    // free memory
    delete graph;

    // return the best solution
    return genetic.getCostBestSolution();
}