#include "rtweekend.h"

#include "bvh.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "triangle.h"
#include "trianglemesh.h"
#include "lod_wrapper.h"
#include "bvh_screen_mask.h"


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

    //auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    //world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

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

    auto material1 = make_shared<dielectric>(1.5);
    // world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    // world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    // world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    // auto mat_tri = make_shared<lambertian>(color(0.8, 0.2, 0.1));
    // world.add(make_shared<triangle>(
    //     point3(-2, 0, 0), 
    //     point3(0, 2, 0), 
    //     point3(2, 0, 0), 
    //     mat_tri
    // ));

    // // 1. Create a material for the mesh
    auto mesh_mat = make_shared<lambertian>(color(0.4, 0.2, 0.1));

    // // 2. Load the mesh
    // triangle_mesh my_model("models/BunnyLOD2.obj", mesh_mat);

    // // 3. Add all triangles from the mesh to the world
    // world.add(make_shared<hittable_list>(my_model.get_triangles()));

    // // 4. Wrap everything in a BVH node for speed
    // world = hittable_list(make_shared<bvh_node>(world));
    
    //create_triangle_box(world);
    
    
    //world = hittable_list(make_shared<bvh_node>(world));

    // 1. Create your materials
    auto mesh_mat_high = make_shared<lambertian>(color(0.8, 0.2, 0.2)); // Red for LOD0
    auto mesh_mat_med  = make_shared<lambertian>(color(0.2, 0.8, 0.2)); // Green for LOD1
    auto mesh_mat_low  = make_shared<lambertian>(color(0.2, 0.2, 0.8)); // Blue for LOD2

    // 2. Load the three individual meshes
    triangle_mesh lod0("models/bunnyLOD1.obj", mesh_mat_high);
    triangle_mesh lod1("models/bunnyLOD2.obj", mesh_mat_med);
    triangle_mesh lod2("models/bunnyLOD3.obj", mesh_mat_low);

    // 1. Build your three separate trees directly
    auto bvh_high = make_shared<bvh_node>(lod0.get_triangles());
    auto bvh_med  = make_shared<bvh_node>(lod1.get_triangles());
    auto bvh_low  = make_shared<bvh_node>(lod2.get_triangles());

    // 2. Lock your camera coordinates into variables
    point3 target_cam_pos = point3(13, 2, 3);
    point3 target_look_at = point3(0, 0, 0);

    // 3. Build the screen mask (3 degree inner, 7 degree outer)
    auto screen_mask = make_shared<bvh_screen_mask>(
        bvh_high, bvh_med, bvh_low, target_cam_pos, target_look_at, 3.0, 7.0 
    );
    world.add(screen_mask);

    // 4. Set up the camera
    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 50;
    cam.max_depth         = 5;

    cam.vfov     = 20;
    cam.lookfrom = target_cam_pos; // Must perfectly match the mask!
    cam.lookat   = target_look_at; // Must perfectly match the mask!
    cam.vup      = vec3(0, 1, 0);

    // 5. CRITICAL: Turn OFF Depth of Field to stop the blurring and ray scatter!
    cam.defocus_angle = 0.0; 
    cam.focus_dist    = 10.0;

    cam.render(world);
}