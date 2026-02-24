#include "rtweekend.h"

#include "bvh.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "triangle.h"
#include "trianglemesh.h"


void create_triangle_box(hittable_list& world) {
    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));

    // A simple 3D triangle "pyramid"
    point3 v0(0, 0, 0);
    point3 v1(2, 0, 0);
    point3 v2(1, 0, 2);
    point3 peak(1, 2, 1);

    world.add(make_shared<triangle>(v0, v1, peak, red));
    world.add(make_shared<triangle>(v1, v2, peak, green));
    world.add(make_shared<triangle>(v2, v0, peak, white));
}

int main() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    // for (int a = -11; a < 11; a++) {
    //     for (int b = -11; b < 11; b++) {
    //         auto choose_mat = random_double();
    //         point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

    //         if ((center - point3(4, 0.2, 0)).length() > 0.9) {
    //             shared_ptr<material> sphere_material;

    //             if (choose_mat < 0.8) {
    //                 // diffuse
    //                 auto albedo = color::random() * color::random();
    //                 sphere_material = make_shared<lambertian>(albedo);
    //                 auto center2 = center + vec3(0, random_double(0,.5), 0);
    //                 world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));                } else if (choose_mat < 0.95) {
    //                 // metal
    //                 auto albedo = color::random(0.5, 1);
    //                 auto fuzz = random_double(0, 0.5);
    //                 sphere_material = make_shared<metal>(albedo, fuzz);
    //                 world.add(make_shared<sphere>(center, 0.2, sphere_material));
    //             } else {
    //                 // glass
    //                 sphere_material = make_shared<dielectric>(1.5);
    //                 world.add(make_shared<sphere>(center, 0.2, sphere_material));
    //             }
    //         }
    //     }
    // }

    // auto material1 = make_shared<dielectric>(1.5);
    // world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    // auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    // world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    // auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    // world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    // auto mat_tri = make_shared<lambertian>(color(0.8, 0.2, 0.1));
    // world.add(make_shared<triangle>(
    //     point3(-2, 0, 0), 
    //     point3(0, 2, 0), 
    //     point3(2, 0, 0), 
    //     mat_tri
    // ));

    // 1. Create a material for the mesh
    auto mesh_mat = make_shared<lambertian>(color(0.4, 0.2, 0.1));

    // 2. Load the mesh
    triangle_mesh my_model("models/dino.obj", mesh_mat);

    // 3. Add all triangles from the mesh to the world
    world.add(make_shared<hittable_list>(my_model.get_triangles()));

    // 4. Wrap everything in a BVH node for speed
    world = hittable_list(make_shared<bvh_node>(world));
    
    create_triangle_box(world);
    
    
    world = hittable_list(make_shared<bvh_node>(world));





    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);
}