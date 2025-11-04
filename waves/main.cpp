// waves/main.cpp
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

const int GRID_W = 256;
const int GRID_H = 256;

const char* vtx = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 uv;
uniform mat4 m,v,p;
void main() {
    gl_Position = p*v*m*vec4(aPos,1.0);
    uv = aTexCoord;
}
)";

const char* frag = R"(
#version 330 core
out vec4 FragColor;
in vec2 uv;
uniform sampler2D waveTex;
uniform float t;

vec3 hsv2rgb(vec3 c){
    vec4 K=vec4(1,2./3.,1./3.,3);
    vec3 p=abs(fract(c.xxx+K.xyz)*6-K.www);
    return c.z*mix(K.xxx,clamp(p-K.xxx,0.,1.),c.y);
}

void main(){
    float h = texture(waveTex, uv).r;
    
    // height-based coloring
    vec3 col;
    if(h > 0.){
        float hue = 0.55 + h * 0.15;
        col = hsv2rgb(vec3(hue, 0.8, 0.9 + h * 0.1));
    } else {
        float hue = 0.6 + h * 0.2;
        col = hsv2rgb(vec3(hue, 0.7, 0.7 + abs(h) * 0.3));
    }
    
    // add specular highlights
    float spec = pow(max(h, 0.), 3.) * 0.5;
    col += vec3(spec);
    
    FragColor = vec4(col, 1);
}
)";

GLuint compShader(GLenum type,const char* src){
    GLuint s=glCreateShader(type);
    glShaderSource(s,1,&src,nullptr);
    glCompileShader(s);
    int ok;char log[512];
    glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){
        glGetShaderInfoLog(s,512,nullptr,log);
        std::cerr<<"Shader err:\n"<<log<<std::endl;
    }
    return s;
}

GLuint mkProg(){
    GLuint vs=compShader(GL_VERTEX_SHADER,vtx);
    GLuint fs=compShader(GL_FRAGMENT_SHADER,frag);
    GLuint prog=glCreateProgram();
    glAttachShader(prog,vs);
    glAttachShader(prog,fs);
    glLinkProgram(prog);
    int ok;char log[512];
    glGetProgramiv(prog,GL_LINK_STATUS,&ok);
    if(!ok){
        glGetProgramInfoLog(prog,512,nullptr,log);
        std::cerr<<"Link err:\n"<<log<<std::endl;
    }
    glDeleteShader(vs);glDeleteShader(fs);
    return prog;
}

void fbResize(GLFWwindow* w,int width,int height){
    glViewport(0,0,width,height);
}

// wave simulation buffers
std::vector<float> wave(GRID_W * GRID_H, 0.0f);
std::vector<float> prevWave(GRID_W * GRID_H, 0.0f);

void updateWave(float dt) {
    const float c = 0.5f; // wave speed
    const float damping = 0.998f;
    
    std::vector<float> newWave(GRID_W * GRID_H);
    
    for(int y = 1; y < GRID_H - 1; y++) {
        for(int x = 1; x < GRID_W - 1; x++) {
            int idx = y * GRID_W + x;
            
            // laplacian (neighbors)
            float laplacian = 
                wave[(y-1)*GRID_W + x] +
                wave[(y+1)*GRID_W + x] +
                wave[y*GRID_W + (x-1)] +
                wave[y*GRID_W + (x+1)] -
                4.0f * wave[idx];
            
            // wave equation: u_tt = c^2 * laplacian
            newWave[idx] = 2.0f * wave[idx] - prevWave[idx] + c * c * laplacian;
            newWave[idx] *= damping;
        }
    }
    
    prevWave = wave;
    wave = newWave;
}

void addRipple(int x, int y, float strength) {
    if(x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return;
    
    int radius = 8;
    for(int dy = -radius; dy <= radius; dy++) {
        for(int dx = -radius; dx <= radius; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if(nx >= 0 && nx < GRID_W && ny >= 0 && ny < GRID_H) {
                float dist = sqrt(dx*dx + dy*dy);
                if(dist < radius) {
                    float falloff = 1.0f - (dist / radius);
                    wave[ny * GRID_W + nx] += strength * falloff;
                }
            }
        }
    }
}

bool mouseDown = false;
float lastMouseX = -1, lastMouseY = -1;

void mouse_button_callback(GLFWwindow* w, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT) {
        mouseDown = (action == GLFW_PRESS);
        if(!mouseDown) {
            lastMouseX = lastMouseY = -1;
        }
    }
}

void cursor_pos_callback(GLFWwindow* w, double xpos, double ypos) {
    if(mouseDown) {
        int ww, wh;
        glfwGetWindowSize(w, &ww, &wh);
        
        int gridX = (int)(xpos / ww * GRID_W);
        int gridY = (int)(ypos / wh * GRID_H);
        
        addRipple(gridX, gridY, 0.8f);
        
        // interpolate for smooth drawing
        if(lastMouseX >= 0) {
            float dx = gridX - lastMouseX;
            float dy = gridY - lastMouseY;
            float dist = sqrt(dx*dx + dy*dy);
            int steps = (int)dist;
            for(int i = 0; i < steps; i++) {
                float t = (float)i / steps;
                int ix = (int)(lastMouseX + dx * t);
                int iy = (int)(lastMouseY + dy * t);
                addRipple(ix, iy, 0.5f);
            }
        }
        
        lastMouseX = gridX;
        lastMouseY = gridY;
    }
}

int main(){
    if(!glfwInit()){
        std::cerr<<"GLFW init fail"<<std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* win=glfwCreateWindow(1024,1024,"Wave Simulation",nullptr,nullptr);
    if(!win){
        std::cerr<<"Window fail"<<std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win,fbResize);
    glfwSetMouseButtonCallback(win, mouse_button_callback);
    glfwSetCursorPosCallback(win, cursor_pos_callback);
    
#ifndef __APPLE__
    if(glewInit()!=GLEW_OK){
        std::cerr<<"GLEW fail"<<std::endl;
        return -1;
    }
#endif
    
    glViewport(0,0,1024,1024);
    
    float verts[]={
        -1,-1,0, 0,0,
         1,-1,0, 1,0,
         1, 1,0, 1,1,
        -1, 1,0, 0,1
    };
    unsigned int idx[]={0,1,2,2,3,0};
    
    GLuint vao,vbo,ebo;
    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&vbo);
    glGenBuffers(1,&ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(idx),idx,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // create texture for wave data
    GLuint waveTex;
    glGenTextures(1, &waveTex);
    glBindTexture(GL_TEXTURE_2D, waveTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    GLuint prog=mkProg();
    glm::mat4 mdl=glm::mat4(1.0f);
    glm::mat4 view=glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-3));
    
    // add initial ripples
    addRipple(GRID_W/2, GRID_H/2, 3.0f);
    addRipple(GRID_W/4, GRID_H/4, 2.0f);
    addRipple(3*GRID_W/4, 3*GRID_H/4, 2.0f);
    
    double lastTime = glfwGetTime();
    
    while(!glfwWindowShouldClose(win)){
        double currentTime = glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;
        
        if(glfwGetKey(win,GLFW_KEY_ESCAPE)==GLFW_PRESS)
            glfwSetWindowShouldClose(win,true);
        
        if(glfwGetKey(win,GLFW_KEY_R)==GLFW_PRESS) {
            std::fill(wave.begin(), wave.end(), 0.0f);
            std::fill(prevWave.begin(), prevWave.end(), 0.0f);
        }
        
        if(glfwGetKey(win,GLFW_KEY_SPACE)==GLFW_PRESS) {
            addRipple(rand() % GRID_W, rand() % GRID_H, 2.0f);
        }
        
        // update wave physics
        for(int i = 0; i < 3; i++) {
            updateWave(0.016f);
        }
        
        int w,h;
        glfwGetFramebufferSize(win,&w,&h);
        float asp=(float)w/(float)h;
        glm::mat4 proj=glm::perspective(glm::radians(45.0f),asp,0.1f,100.0f);
        
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        
        // upload wave data to texture
        glBindTexture(GL_TEXTURE_2D, waveTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, GRID_W, GRID_H, 0, GL_RED, GL_FLOAT, wave.data());
        
        float tm=glfwGetTime();
        glUniform1f(glGetUniformLocation(prog,"t"),tm);
        glUniform1i(glGetUniformLocation(prog,"waveTex"),0);
        glUniformMatrix4fv(glGetUniformLocation(prog,"m"),1,GL_FALSE,glm::value_ptr(mdl));
        glUniformMatrix4fv(glGetUniformLocation(prog,"v"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(prog,"p"),1,GL_FALSE,glm::value_ptr(proj));
        
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1,&vao);
    glDeleteBuffers(1,&vbo);
    glDeleteBuffers(1,&ebo);
    glDeleteTextures(1,&waveTex);
    glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}