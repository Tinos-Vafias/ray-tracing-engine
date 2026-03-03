#ifndef BVH_H
#define BVH_H

#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

class bvh_node : public hittable {
  public:
    bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {
        // There's a C++ subtlety here. This constructor (without span indices) creates an
        // implicit copy of the hittable list, which we will modify. The lifetime of the copied
        // list only extends until this constructor exits. That's OK, because we only need to
        // persist the resulting bounding volume hierarchy.
    }

    bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
                bbox = aabb::empty;
        for (size_t object_index=start; object_index < end; object_index++)
            bbox = aabb(bbox, objects[object_index]->bounding_box());

        int axis = bbox.longest_axis();

        auto comparator = (axis == 0) ? box_x_compare
                        : (axis == 1) ? box_y_compare
                                      : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1) {
            left = right = objects[start];
        } else if (object_span == 2) {
            left = objects[start];
            right = objects[start+1];
        } else {
            std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

            auto mid = start + object_span/2;
            left = make_shared<bvh_node>(objects, start, mid);
            right = make_shared<bvh_node>(objects, mid, end);
        }
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!bbox.hit(r, ray_t))
            return false;

        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox;

        static bool box_compare(
        const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index
    ) {
        auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
        auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 2);
    }

};

class bvh_spatial_mask : public hittable {
  public:
    shared_ptr<hittable> lod0; // Inner
    shared_ptr<hittable> lod1; // Mid
    shared_ptr<hittable> lod2; // Outer
    point3 center;
    double r_inner;
    double r_outer;
    aabb bbox;

    bvh_spatial_mask(shared_ptr<hittable> l0, shared_ptr<hittable> l1, shared_ptr<hittable> l2, 
                     point3 c, double ri, double ro)
        : lod0(l0), lod1(l1), lod2(l2), center(c), r_inner(ri), r_outer(ro) {
        
        // The master bounding box covers all meshes
        bbox = aabb(lod0->bounding_box(), lod1->bounding_box());
        bbox = aabb(bbox, lod2->bounding_box());
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 1. Standard BVH box check
        if (!bbox.hit(r, ray_t))
            return false;

        // 2. Math to intersect the ray with our masking spheres
        vec3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c_origin = oc.length_squared();

        double t_out_min = 0, t_out_max = 0;
        auto disc_out = half_b * half_b - a * (c_origin - r_outer * r_outer);
        if (disc_out >= 0) {
            auto sqrtd = std::sqrt(disc_out);
            t_out_min = (-half_b - sqrtd) / a;
            t_out_max = (-half_b + sqrtd) / a;
        }

        double t_in_min = 0, t_in_max = 0;
        auto disc_in = half_b * half_b - a * (c_origin - r_inner * r_inner);
        if (disc_in >= 0) {
            auto sqrtd = std::sqrt(disc_in);
            t_in_min = (-half_b - sqrtd) / a;
            t_in_max = (-half_b + sqrtd) / a;
        }

        // 3. Slice the ray into non-overlapping spatial segments
        struct RaySegment {
            double t_start;
            double t_end;
            shared_ptr<hittable> tree;
        };
        std::vector<RaySegment> segments;

        if (disc_out < 0) {
            // Ray completely misses the mask. Traverse LOD2 only.
            segments.push_back({ray_t.min, ray_t.max, lod2});
        } 
        else if (disc_in < 0) {
            // Ray clips the outer sphere, but misses the inner sphere.
            segments.push_back({ray_t.min, t_out_min, lod2});
            segments.push_back({t_out_min, t_out_max, lod1});
            segments.push_back({t_out_max, ray_t.max, lod2});
        } 
        else {
            // Ray punches straight through the middle!
            segments.push_back({ray_t.min, t_out_min, lod2}); // Outside
            segments.push_back({t_out_min, t_in_min, lod1});  // Entering mid shell
            segments.push_back({t_in_min,  t_in_max, lod0});  // Core
            segments.push_back({t_in_max,  t_out_max, lod1}); // Exiting mid shell
            segments.push_back({t_out_max, ray_t.max, lod2}); // Outside
        }

        // 4. Traverse the trees strictly in order of distance (Front to Back)
        for (const auto& seg : segments) {
            
            // Create an interval strictly bound by this spatial zone
            interval active_zone(
                std::max(ray_t.min, seg.t_start), 
                std::min(ray_t.max, seg.t_end)
            );

            // If this is a valid slice of space...
            if (active_zone.min < active_zone.max) {
                
                // ...tell the BVH to ONLY look for triangles inside this slice!
                if (seg.tree->hit(r, active_zone, rec)) {
                    // Because we check front-to-back, the first hit is guaranteed to be the closest.
                    // We immediately return true and SKIP traversing all remaining trees/zones!
                    return true; 
                }
            }
        }

        return false;
    }

    aabb bounding_box() const override { return bbox; }
};

#endif