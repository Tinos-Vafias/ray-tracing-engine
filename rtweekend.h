#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <chrono>


// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}


inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

inline int random_int(int min, int max) {
    // Returns a random integer in [min,max].
    return int(random_double(min, max+1));
}

struct timer {
	std::string name;
	std::chrono::high_resolution_clock::time_point start, end;
	timer(std::string name) : name(name) {
		start = std::chrono::high_resolution_clock::now();
	}

	~timer() {
		end = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds duration =
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << name << " took " << duration.count() / 1000.0 << " seconds" << std::endl;
	}
};


// Common Headers

#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"
#include "triangle.h"
#include "aabb.h"

#endif