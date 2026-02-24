cmake_minimum_required(VERSION 3.10)

# 1. Project name
project(RayTracingEngine)

# 2. Specify the C++ standard (The tutorial uses modern C++ features)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. Add the executable 
# You only need to list .cpp files; .h files are included via #include
add_executable(inOneWeekend main.cpp tiny_obj_loader.cpp)

# 4. (Optional) Helpful for VS Code to find your headers easily
target_include_directories(inOneWeekend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})