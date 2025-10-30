cmake_minimum_required(VERSION 3.10)
project(BlackHoleRenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Add executable
add_executable(blackhole main.cpp)

# Link libraries
target_link_libraries(blackhole 
    OpenGL::GL 
    GLEW::GLEW 
    glfw
    glm::glm
)

# Include directories
target_include_directories(blackhole PRIVATE ${OPENGL_INCLUDE_DIR})