#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "rtweekend.h"
#include "hittable.h"
#include "vec3.h"
#include "ray.h"
#include "interval.h"
#include "aabb.h"

#include <algorithm> // Required for std::min/std::max with initializer lists

class triangle : public hittable {
  public:
    triangle(const point3& v0, const point3& v1, const point3& v2, shared_ptr<material> mat)
        : v0(v0), v1(v1), v2(v2), mat(mat) 
    {
        // Pre-calculate the bounding box at construction
        vec3 min_p(
            std::min({v0.x(), v1.x(), v2.x()}),
            std::min({v0.y(), v1.y(), v2.y()}),
            std::min({v0.z(), v1.z(), v2.z()})
        );

        vec3 max_p(
            std::max({v0.x(), v1.x(), v2.x()}),
            std::max({v0.y(), v1.y(), v2.y()}),
            std::max({v0.z(), v1.z(), v2.z()})
        );

        bbox = aabb(min_p, max_p);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        const double EPSILON = 0.0000001;

        vec3 edge1 = v1 - v0;
        vec3 edge2 = v2 - v0;
        
        vec3 h = cross(r.direction(), edge2);
        double a = dot(edge1, h);

        // If a is near zero, the ray is parallel to the triangle
        if (std::fabs(a) < EPSILON)
            return false;

        double f = 1.0 / a;
        vec3 s = r.origin() - v0;
        double u = f * dot(s, h);

        if (u < 0.0 || u > 1.0)
            return false;

        vec3 q = cross(s, edge1);
        double v = f * dot(r.direction(), q);

        if (v < 0.0 || u + v > 1.0)
            return false;

        double t = f * dot(edge2, q);

        // Check if t is within the valid range using the interval
        if (!ray_t.contains(t))
            return false;

        // Populate the hit record
        rec.t = t;
        rec.p = r.at(t);
        
        // Calculate the normal and determine front-facing status
        vec3 outward_normal = unit_vector(cross(edge1, edge2));
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    point3 v0, v1, v2;
    shared_ptr<material> mat;
    aabb bbox;
};

#endif