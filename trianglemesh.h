#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H


#include "tiny_obj_loader.h"
#include "rtweekend.h"
#include "triangle.h"
#include "hittable_list.h"
#include <iostream>
#include <vector>
#include <string>

class triangle_mesh {
  public:
    triangle_mesh(const char* filepath, shared_ptr<material> mat) {
        std::string inputfile = filepath;
        
        // Find base path for MTL files
        std::string mtlbasepath = "";
        std::string s_path = inputfile;
        size_t pos = s_path.find_last_of("/\\");
        if (pos != std::string::npos) {
            mtlbasepath = s_path.substr(0, pos + 1);
        }

        tinyobj::attrib_t attributes;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warnings;
        std::string errors;

        bool ret = tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, inputfile.c_str(), mtlbasepath.c_str());
        
        if (!warnings.empty()) std::cout << "TinyObj Warning: " << warnings << std::endl;
        if (!errors.empty())   std::cerr << "TinyObj Error: " << errors << std::endl;
        if (!ret)              exit(1);

        // Loop over shapes in the OBJ file
        for (size_t s = 0; s < shapes.size(); s++) {
            size_t index_offset = 0;
            // Loop over faces (triangles)
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                int fv = shapes[s].mesh.num_face_vertices[f];

                // Ensure the face is a triangle
                if (fv == 3) {
                    point3 vertices[3];
                    for (size_t v = 0; v < 3; v++) {
                        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                        vertices[v] = point3(
                            attributes.vertices[3 * idx.vertex_index + 0],
                            attributes.vertices[3 * idx.vertex_index + 1],
                            attributes.vertices[3 * idx.vertex_index + 2]
                        );
                    }
                    // Create your triangle object and add to the mesh list
                    mesh_tris.add(make_shared<triangle>(vertices[0], vertices[1], vertices[2], mat));
                }
                index_offset += fv;
            }
        }
        // comment this to run the ppm file (it won't read with these commented lines)
        std::cout << "> Successfully loaded " << inputfile << " with " << mesh_tris.objects.size() << " triangles.\n";
    }

    // Returns the list of triangles to be added to the world
    hittable_list& get_triangles() { return mesh_tris; }

  private:
    hittable_list mesh_tris;
};

#endif