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
#include <cmath>

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
uniform float t;
uniform vec2 res;
uniform vec2 center;
uniform float zoom;
uniform int fractalMode;

vec3 hsv2rgb(vec3 c){
    vec4 K=vec4(1,2./3.,1./3.,3);
    vec3 p=abs(fract(c.xxx+K.xyz)*6-K.www);
    return c.z*mix(K.xxx,clamp(p-K.xxx,0.,1.),c.y);
}

float mandel(vec2 c,int maxIter){
    vec2 z=vec2(0);
    for(int i=0;i<maxIter;i++){
        float r2=dot(z,z);
        if(r2>4.)return float(i);
        z=vec2(z.x*z.x-z.y*z.y,2.*z.x*z.y)+c;
    }
    return float(maxIter);
}

float julia(vec2 z,vec2 c,int maxIter){
    for(int i=0;i<maxIter;i++){
        float r2=dot(z,z);
        if(r2>4.)return float(i);
        z=vec2(z.x*z.x-z.y*z.y,2.*z.x*z.y)+c;
    }
    return float(maxIter);
}

void main(){
    vec2 uv_=(uv-.5)*2;
    uv_.x*=res.x/res.y;
    
    vec2 coord=uv_*zoom+center;
    
    float iter;
    int maxIter=256;
    
    if(fractalMode==0){
        iter=mandel(coord,maxIter);
    }else{
        vec2 c=vec2(-.4,.6);
        iter=julia(coord,c,maxIter);
    }
    
    vec3 col;
    if(iter>=float(maxIter)){
        col=vec3(0);
    }else{
        float n=iter/float(maxIter);
        float hue=n*.7;
        col=hsv2rgb(vec3(hue,.8,.9));
    }
    
    FragColor=vec4(col,1);
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

float g_zoom = 2.0f;
float g_centerX = -0.5f;
float g_centerY = 0.0f;
int g_mode = 0;

void scroll_callback(GLFWwindow* w, double xoff, double yoff) {
    g_zoom *= (yoff > 0) ? 0.9f : 1.1f;
    g_zoom = fmax(0.0001f, g_zoom);
}

void mouse_button_callback(GLFWwindow* w, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double mx, my;
        glfwGetCursorPos(w, &mx, &my);
        int ww, wh;
        glfwGetWindowSize(w, &ww, &wh);
        
        float nx = (mx / ww - 0.5f) * 2.0f * (float)ww / (float)wh;
        float ny = -(my / wh - 0.5f) * 2.0f;
        
        g_centerX += nx * g_zoom;
        g_centerY += ny * g_zoom;
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
    
    GLFWwindow* win=glfwCreateWindow(1600,900,"Fractal Explorer",nullptr,nullptr);
    if(!win){
        std::cerr<<"Window fail"<<std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win,fbResize);
    glfwSetScrollCallback(win, scroll_callback);
    glfwSetMouseButtonCallback(win, mouse_button_callback);
    
#ifndef __APPLE__
    if(glewInit()!=GLEW_OK){
        std::cerr<<"GLEW fail"<<std::endl;
        return -1;
    }
#endif
    
    glViewport(0,0,1600,900);
    
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
    
    GLuint prog=mkProg();
    glm::mat4 mdl=glm::mat4(1.0f);
    glm::mat4 view=glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-3));
    
    while(!glfwWindowShouldClose(win)){
        if(glfwGetKey(win,GLFW_KEY_ESCAPE)==GLFW_PRESS)
            glfwSetWindowShouldClose(win,true);
        
        float panSpeed = g_zoom * 0.05f;
        if(glfwGetKey(win,GLFW_KEY_W)==GLFW_PRESS) g_centerY -= panSpeed;
        if(glfwGetKey(win,GLFW_KEY_S)==GLFW_PRESS) g_centerY += panSpeed;
        if(glfwGetKey(win,GLFW_KEY_A)==GLFW_PRESS) g_centerX -= panSpeed;
        if(glfwGetKey(win,GLFW_KEY_D)==GLFW_PRESS) g_centerX += panSpeed;
        if(glfwGetKey(win,GLFW_KEY_Q)==GLFW_PRESS) g_zoom *= 0.98f;
        if(glfwGetKey(win,GLFW_KEY_E)==GLFW_PRESS) g_zoom *= 1.02f;
        if(glfwGetKey(win,GLFW_KEY_SPACE)==GLFW_PRESS) {
            static bool pressed = false;
            if(!pressed) {
                g_mode = 1 - g_mode;
                pressed = true;
            }
        } else {
            static bool pressed = false;
            pressed = false;
        }
        if(glfwGetKey(win,GLFW_KEY_R)==GLFW_PRESS) {
            g_zoom = 2.0f;
            g_centerX = -0.5f;
            g_centerY = 0.0f;
        }
        
        int w,h;
        glfwGetFramebufferSize(win,&w,&h);
        float asp=(float)w/(float)h;
        glm::mat4 proj=glm::perspective(glm::radians(45.0f),asp,0.1f,100.0f);
        
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        
        float tm=glfwGetTime();
        
        glUniform1f(glGetUniformLocation(prog,"t"),tm);
        glUniform2f(glGetUniformLocation(prog,"res"),(float)w,(float)h);
        glUniform2f(glGetUniformLocation(prog,"center"),g_centerX,g_centerY);
        glUniform1f(glGetUniformLocation(prog,"zoom"),g_zoom);
        glUniform1i(glGetUniformLocation(prog,"fractalMode"),g_mode);
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
    glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}