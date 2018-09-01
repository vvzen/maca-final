#include <mbc.h>

// PACKED CIRCLE
void PackedCircle::setup(glm::vec2 p, int r){
    pos =  p;
    radius = r;
}

// MITCHELL BEST CANDIDATE
void MBC::setup(const ofImage * img, int circle_radius){
    target_image = img;
    target_radius = circle_radius;
    fbo = nullptr;
}

void MBC::setup(const ofImage * img, int circle_radius, ofFbo * in_fbo, ofColor col){
    target_image = img;
    target_radius = circle_radius;
    fbo = in_fbo;
    color = col;
}

// Algorithm is divided in 4 parts
// 1. Method for generating a mitchell best candidate
PackedCircle MBC::get_mitchell_best_candidate(){
    
    float greatestDistance = 0.0f;
    PackedCircle winningSample;
    // Init at (0,0) with radius of 0
    // winningSample.setup(ofPoint(0,0), 0.0f);
    winningSample.setup(glm::vec2(0,0), 0.0f);
    int k = 64;
    // Loop for k times, where k is the number of samples: the higher, the better
    for(int i = 0; i < k; i++){

        ofColor col = ofColor::black;
        glm::vec2 position(0, 0);

        // Create a new random x,y point on a white pixel of the given image
        while (col.r != 255){
            position.x = ofRandom(target_image->getWidth());
            position.y = ofRandom(target_image->getHeight());
            col = target_image->getColor(position.x, position.y);
        }

        PackedCircle candidate;
        candidate.setup(position, target_radius);
        
        float currentDistance;
        // If this is the first dot we're placing, return this as best candidate
        if (placed_circles.size() == 0) return candidate;
        
        // Start from the distance to the nearest placed sample
        currentDistance = get_distance_to_nearest_sample(candidate);
        // Find the greatest distance among all k samples
        if(currentDistance > greatestDistance){
            greatestDistance = currentDistance;
            winningSample = candidate;
        }
    }
    
    //cout << "Winning radius: " << winningSample.radius << endl;
    
    return winningSample;
}

// 2. Method for returning distance from sample and nearest already placed sample
float MBC::get_distance_to_nearest_sample(PackedCircle candidate){
    
    float shortestDistance = 0.0f;
    
    // Loop into all samples
    for(int i = 0; i < placed_circles.size(); i++){
        PackedCircle otherSample = placed_circles[i];
        // Measure current distance between the two points 
        float currentDistance = ofDist(candidate.pos.x, candidate.pos.y, otherSample.pos.x, otherSample.pos.y);
        // Get nearest sample (from previously placed ones)
        if(shortestDistance == 0.0f || currentDistance < shortestDistance){
            shortestDistance = currentDistance;
        }
    }
    return shortestDistance;
}

// 3. Method for placing and drawing samples to screen
void MBC::place_new_sample(){
    
    // Generate new sample
    PackedCircle sample = get_mitchell_best_candidate();
    // Add it to the list of samples
    placed_circles.push_back(sample);

    if (fbo != nullptr){
        ofPushStyle();
        ofFill();
        ofSetColor(color);
        ofDrawCircle(sample.pos.x, sample.pos.y, target_radius);
        ofPopStyle();
    }
}

void MBC::generate_all_samples(int num){

    if (fbo != nullptr) fbo->begin();

    // We want to have num samples at the end
    while(placed_circles.size() < num){
        place_new_sample();
    }
    ofLogNotice("generate_all_samples") << "All samples placed!" << endl;

    if (fbo != nullptr) fbo->end();
}