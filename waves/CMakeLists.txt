cmake_minimum_required(VERSION 3.10)
project(WaveSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(waves main.cpp)

target_link_libraries(waves 
    OpenGL::GL 
    GLEW::GLEW 
    glfw
    glm::glm
)

target_include_directories(waves PRIVATE ${OPENGL_INCLUDE_DIR})