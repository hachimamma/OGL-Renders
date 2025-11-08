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
#include <fstream>
#include <sstream>
#include <cmath>

std::string loadShader(const char* path){
    std::ifstream file(path);
    if(!file.is_open()){
        std::cerr<<"Failed to load shader: "<<path<<std::endl;
        return "";
    }
    std::stringstream buf;
    buf<<file.rdbuf();
    return buf.str();
}

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
uniform vec3 cam;
uniform vec2 res;

const float G = 1.0;
const float M = 1.0;
const float c = 1.0;
const float rs = 2.0*G*M/(c*c);
const float STEP = 0.04;
const int MAX_STEPS = 180;

float h(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}

float n(vec2 p){
    vec2 i=floor(p),f=fract(p);
    f=f*f*(3.0-2.0*f);
    return mix(mix(h(i),h(i+vec2(1,0)),f.x),mix(h(i+vec2(0,1)),h(i+vec2(1,1)),f.x),f.y);
}

float fbm(vec2 p){
    float v=0,a=.5;
    for(int i=0;i<5;i++){v+=a*n(p);p*=2.;a*=.5;}
    return v;
}

vec3 accel(vec3 pos){
    float r=length(pos);
    float r2=r*r;
    return -1.5*rs*pos/(r2*r);
}

bool intersectDisk(vec3 o,vec3 d,out vec3 hit,out float dist){
    float t=-o.y/d.y;
    if(t<0.)return false;
    hit=o+d*t;
    float r=length(hit.xz);
    dist=r;
    return r>2.5*rs&&r<8.0*rs;
}

vec3 diskColor(vec3 hit){
    float r=length(hit.xz);
    float nr=(r-2.5*rs)/(8.0*rs-2.5*rs);
    float ang=atan(hit.z,hit.x);
    
    float rotSpeed=1.0/sqrt(nr+.1);
    ang+=t*rotSpeed*.6;
    
    float turb=fbm(vec2(ang*8.,nr*18.));
    float sp=sin(ang*15.-nr*22.+t*5.)*.5+.5;
    
    vec3 c;
    float tmp=1.-nr;
    if(tmp>.75)c=mix(vec3(.15,.3,.85),vec3(.4,.5,.9),(tmp-.75)*4.);
    else if(tmp>.5)c=mix(vec3(.35,.5,.6),vec3(.45,.6,.7),(tmp-.5)*4.);
    else if(tmp>.3)c=mix(vec3(.65,.5,.25),vec3(.75,.6,.35),(tmp-.3)*5.);
    else c=mix(vec3(.5,.2,.1),vec3(.7,.4,.2),tmp*3.33);
    
    float br=1.2*(1.-nr*.7)*(1.+sp*.4);
    br*=(.6+turb*.4);
    
    return c*br;
}

vec3 trace(vec3 o,vec3 d){
    vec3 pos=o;
    vec3 vel=d;
    vec3 col=vec3(0);
    float alpha=0;
    
    for(int i=0;i<MAX_STEPS;i++){
        float r=length(pos);
        
        if(r<rs){
            return col+vec3(.01,.005,.02)*(1.-alpha);
        }
        
        vec3 hit;
        float dist;
        if(intersectDisk(pos,vel,hit,dist)){
            vec3 dc=diskColor(hit);
            float fade=1.-dist/(8.0*rs);
            float a=fade*.25;
            col+=dc*a*(1.-alpha);
            alpha+=a*(1.-alpha);
            if(alpha>.9)break;
        }
        
        vec3 acc=accel(pos);
        vel=normalize(vel+acc*STEP);
        pos+=vel*STEP;
    }
    
    float st=h(normalize(vel).xy*150.);
    if(st>.997)col+=vec3(.7,.75,.8)*(st-.997)*600.*(1.-alpha);
    
    col+=vec3(.008,.006,.012)*(1.-alpha);
    return col;
}

void main(){
    vec2 uv_=(uv-.5)*2.;
    uv_.x*=res.x/res.y;
    
    vec3 ro=cam;
    vec3 tgt=vec3(0);
    vec3 fwd=normalize(tgt-ro);
    vec3 rt=normalize(cross(vec3(0,1,0),fwd));
    vec3 up=cross(fwd,rt);
    vec3 rd=normalize(fwd+uv_.x*rt+uv_.y*up);
    
    vec3 c=trace(ro,rd);
    c=c/(c+vec3(.4));
    c=pow(c,vec3(.88));
    
    float vig=1.-length(uv_*.4);
    c*=.25+vig*.75;
    
    FragColor=vec4(c,1);
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

int main(){
    if(!glfwInit()){
        std::cerr<<"GLFW fail"<<std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* win=glfwCreateWindow(1600,900,"Black Hole",nullptr,nullptr);
    if(!win){
        std::cerr<<"Window fail"<<std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win,fbResize);
    
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
    
    std::cout<<"Black Hole Simulation with Gravitational Lensing\n";
    std::cout<<"Press ESC to exit\n\n";
    
    while(!glfwWindowShouldClose(win)){
        if(glfwGetKey(win,GLFW_KEY_ESCAPE)==GLFW_PRESS)
            glfwSetWindowShouldClose(win,true);
        
        float tm=glfwGetTime();
        float spd=tm*.35f;
        float inw=tm*.07f;
        float rad=fmax(3.2f-inw,.6f);
        float ang=spd*2.4f;
        float cx=cos(ang)*rad,cz=sin(ang)*rad;
        float cy=.18f+sin(spd*.7f)*.12f;
        
        int w,h;
        glfwGetFramebufferSize(win,&w,&h);
        float asp=(float)w/(float)h;
        glm::mat4 proj=glm::perspective(glm::radians(45.0f),asp,0.1f,100.0f);
        
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        
        glUniform1f(glGetUniformLocation(prog,"t"),tm);
        glUniform3f(glGetUniformLocation(prog,"cam"),cx,cy,cz);
        glUniform2f(glGetUniformLocation(prog,"res"),(float)w,(float)h);
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