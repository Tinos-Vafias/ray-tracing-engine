#ifndef BVH_SCREEN_MASK_H
#define BVH_SCREEN_MASK_H

#include "rtweekend.h"
#include "hittable.h"

class bvh_screen_mask : public hittable {
  public:
    shared_ptr<hittable> lod0;
    shared_ptr<hittable> lod1;
    shared_ptr<hittable> lod2;
    point3 cam_pos;
    vec3 cam_forward;
    double cos_inner;
    double cos_outer;
    aabb bbox;

    bvh_screen_mask(shared_ptr<hittable> l0, shared_ptr<hittable> l1, shared_ptr<hittable> l2, 
                     point3 c_pos, point3 look_at, double angle_inner_deg, double angle_outer_deg)
        : lod0(l0), lod1(l1), lod2(l2), cam_pos(c_pos) {
        
        cam_forward = unit_vector(look_at - cam_pos);
        cos_inner = std::cos(degrees_to_radians(angle_inner_deg));
        cos_outer = std::cos(degrees_to_radians(angle_outer_deg));

        bbox = aabb(lod0->bounding_box(), lod1->bounding_box());
        bbox = aabb(bbox, lod2->bounding_box());
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!bbox.hit(r, ray_t))
            return false;

        vec3 sample_dir;
        
        // Safety net expanded to 4.0 to catch ALL primary camera rays
        if ((r.origin() - cam_pos).length_squared() < 4.0) {
            sample_dir = unit_vector(r.direction());
        } else {
            sample_dir = unit_vector(r.origin() - cam_pos);
        }

        double cos_theta = dot(sample_dir, cam_forward);

        if (cos_theta > cos_inner) {
            return lod0->hit(r, ray_t, rec);
        } else if (cos_theta > cos_outer) {
            return lod1->hit(r, ray_t, rec);
        } else {
            return lod2->hit(r, ray_t, rec);
        }
    }

    aabb bounding_box() const override { return bbox; }
};

#endif