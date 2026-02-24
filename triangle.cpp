#include "triangle.h"
#include "interval.h"  // <--- This is likely what's missing!
#include "rtweekend.h"

bool triangle::hit(const ray& r, interval ray_t, hit_record& rec) const {
    const double EPSILON = 0.0000001;

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    
    // Use () to call the direction member function
    vec3 h = cross(r.direction(), edge2);
    double a = dot(edge1, h);

    // If a is near zero, the ray is parallel to the triangle
    if (a > -EPSILON && a < EPSILON)
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

    // At this stage we can compute t to find out where the intersection point is on the line.
    double t = f * dot(edge2, q);

    // Use the interval class (from hittable.h) to check if t is in the valid range
    if (!ray_t.contains(t))
        return false;

    // Populate the hit_record reference passed as an argument
    rec.t = t;
    rec.p = r.at(t);
    
    // Calculate the normal (perpendicular) to the triangle face
    // We normalize the cross product of the two edges
    vec3 outward_normal = unit_vector(cross(edge1, edge2));
    rec.set_face_normal(r, outward_normal);
    

    return true;
}

aabb triangle::bounding_box() const {
    vec3 minimum(
        std::min({v0.x(), v1.x(), v2.x()}),
        std::min({v0.y(), v1.y(), v2.y()}),
        std::min({v0.z(), v1.z(), v2.z()})
    );

    vec3 maximum(
        std::max({v0.x(), v1.x(), v2.x()}),
        std::max({v0.y(), v1.y(), v2.y()}),
        std::max({v0.z(), v1.z(), v2.z()})
    );

    return aabb(minimum, maximum);
}