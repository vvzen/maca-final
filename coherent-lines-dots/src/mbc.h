#include "ofMain.h"

// I wrote a tutorial on Mitchell Best Candidate sampling technique here:
// http://vvzen.it/archives/2d-unequal-circles-packing

class PackedCircle {
    public:
        glm::vec2 pos;
        int radius;

        void setup(glm::vec2 pos, int radius);
};

class MBC {
    public:

        void setup(const ofImage * img, int circle_radius);
        void setup(const ofImage * img, int circle_radius, ofFbo * in_fbo, ofColor col);
        float get_distance_to_nearest_sample(PackedCircle candidate);
        void place_new_sample();
        void generate_all_samples(int num);
        void draw_dots(ofFbo &fbo);
        PackedCircle get_mitchell_best_candidate();
        
        const ofImage * target_image;
        vector<PackedCircle> placed_circles;
        int target_radius;
        ofFbo * fbo;
        ofColor color;
};