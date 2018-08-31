#pragma once

#include "ofMain.h"
#include "tsp.h" // for solving tsp using a genetic algorithm, source: https://github.com/marcoscastro/tsp_genetic

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		
		void keyPressed(int key) override;
		
		vector<glm::vec2> points;
		vector<glm::vec2> points_nn_path;
		vector<glm::vec2> points_ga_tsp_path;

		// Mesh used to draw the points
		ofMesh mesh;

		// To draw the paths of the different algorithms
		int current_algorithm_view;
		const int DEFAULT = 0;
		const int NEAREST_NEIGHBOUR = 1;
		const int TSP_GENETIC = 2;

		 // for measuring the overall length of the different approaches
		float nn_distance;
		float default_distance;
		float ga_tsp_distance;
		void load_points_from_csv(vector<glm::vec2> & vec, std::string path);

		// TSP genetic algorithm approach using external library
		int solve_tsp(const vector<glm::vec2> & in_points, vector<glm::vec2> & out_points);
};
