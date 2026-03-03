#ifndef LOD_WRAPPER_H
#define LOD_WRAPPER_H

#include "rtweekend.h" 
#include "hittable.h"
#include "ray.h"
#include "interval.h"

class lod_wrapper : public hittable {
  public:
    shared_ptr<hittable> object;
    int my_lod_level;
    point3 mask_center;
    double radius_inner;
    double radius_outer;

    lod_wrapper(shared_ptr<hittable> obj, int level, point3 center, double r_in, double r_out)
        : object(obj), my_lod_level(level), mask_center(center), radius_inner(r_in), radius_outer(r_out) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 1. Did the ray actually hit the triangle?
        if (!object->hit(r, ray_t, rec))
            return false;

        // 2. We hit it! Calculate the distance from the intersection point to the mask center.
        double dist = (rec.p - mask_center).length();

        // 3. Which LOD is supposed to exist at this specific point in space?
        int active_lod = 2; // Default to Outside (LOD2)
        if (dist < radius_inner) {
            active_lod = 0; // Inside inner circle
        } else if (dist < radius_outer) {
            active_lod = 1; // Middle ring
        }

        // 4. If this triangle's level doesn't match the active zone, act like we missed!
        if (my_lod_level != active_lod) {
            return false;
        }

        return true;
    }

    aabb bounding_box() const override { return object->bounding_box(); }
};

#endif