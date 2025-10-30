// blackhole/main.cpp
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
uniform vec3 cam;

const float sr = 0.15;
const float eh = 0.2;
const float di = 0.4;
const float do_ = 3.5;

float h(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float h3(vec3 p){return fract(sin(dot(p,vec3(127.1,311.7,74.7)))*43758.5453);}

float n(vec2 p){
    vec2 i=floor(p),f=fract(p);
    f=f*f*(3.0-2.0*f);
    float a=h(i),b=h(i+vec2(1,0)),c=h(i+vec2(0,1)),d=h(i+vec2(1,1));
    return mix(mix(a,b,f.x),mix(c,d,f.x),f.y);
}

float fbm(vec2 p){
    float val=0,amp=.5,freq=1;
    for(int i=0;i<6;i++){
        val+=amp*n(p*freq);
        freq*=2;amp*=.5;
    }
    return val;
}

vec3 march(vec3 o,vec3 d){
    vec3 pos=o;
    float td=0;
    vec3 col=vec3(0);
    float alpha=0;
    
    for(int i=0;i<200;i++){
        float r=length(pos);
        if(r<eh) return col;
        
        float dh=abs(pos.y);
        float dt=.25+.15*sin(t*1.2);
        
        if(dh<dt&&r>di&&r<do_){
            float dn=(r-di)/(do_-di);
            float av=(1./sqrt(dn+.1))*2.5;
            float ang=atan(pos.z,pos.x)+t*av;
            
            vec2 tc=vec2(ang*6+t*.8,dn*15);
            float turb=fbm(tc);
            float sp=sin(ang*12-dn*20+t*4)*.5+.5;
            float hf=1.-dh/dt;hf=pow(hf,2);
            float dens=turb*hf*(.6+sp*.4);
            dens*=(1.-dn*.5);
            
            float hs=n(vec2(ang*8+t*2,dn*10));
            if(hs>.82) dens*=4.+sin(t*15);
            
            vec3 c;
            float tmp=1.-dn;
            
            if(tmp>.75){
                c=mix(vec3(.6,.7,1),vec3(1),  (tmp-.75)*4);
                c+=vec3(.4,.5,1.2)*hs*3;
            }else if(tmp>.6){
                c=mix(vec3(.3,.9,1),vec3(.9,.95,1),(tmp-.6)*6.66);
                c+=vec3(.5,1,1.2)*turb*.8;
            }else if(tmp>.45){
                c=mix(vec3(.8,1,.3),vec3(1,1,.7),(tmp-.45)*6.66);
                c+=vec3(1,1,.4)*sp*.6;
            }else if(tmp>.3){
                c=mix(vec3(1,.6,.1),vec3(1,.9,.4),(tmp-.3)*6.66);
                c+=vec3(1.2,.7,.2)*turb*.7;
            }else{
                c=mix(vec3(.5,.05,.1),vec3(1,.3,.5),tmp*3.33);
                c+=vec3(1,.2,.6)*sp*.5;
            }
            
            c.r+=turb*.3;
            c.g+=sin(ang*15+t*2)*.2;
            c.b+=cos(ang*20-t*3)*.25;
            
            float br=dens*(3+tmp*4);
            br*=(1+sin(t*5+ang*8)*.5);
            
            vec3 dc=c*br;
            float a=dens*.15;
            col+=dc*a*(1-alpha);
            alpha+=a*(1-alpha);
            if(alpha>.95)break;
        }
        
        float g=(sr*sr)/(r*r+.01);
        vec3 gd=-normalize(pos);
        d=normalize(d+gd*g*.4);
        
        float step=.03+r*.015;
        if(r>di-.5&&r<do_+.5)step*=.4;
        pos+=d*step;
        td+=step;
        if(td>25)break;
    }
    
    vec3 sd=normalize(d);
    float sn=h(sd.xy*200+sd.z*100);
    if(sn>.997){
        float sb=(sn-.997)*1000;
        vec3 sc=mix(vec3(1,.9,.8),vec3(.8,.9,1),h(sd.yz*123));
        col+=sc*sb*(1-alpha);
    }
    
    float bg=fbm(sd.xy*3+t*.05)*.03;
    col+=vec3(.1,.05,.15)*bg*(1-alpha);
    col+=vec3(.002,.003,.01)*(1-alpha);
    return col;
}

void main(){
    vec2 uv_=(uv-.5)*2;
    vec3 ro=cam;
    vec3 tgt=vec3(0);
    vec3 fwd=normalize(tgt-ro);
    vec3 rt=normalize(cross(vec3(0,1,0),fwd));
    vec3 up=cross(fwd,rt);
    vec3 rd=normalize(fwd+uv_.x*rt+uv_.y*up);
    
    vec3 c=march(ro,rd);
    c=c/(c+vec3(.5));
    c=pow(c,vec3(.9));
    c=pow(c,vec3(1./2.2));
    c.r=pow(c.r,.95);
    c.b=pow(c.b,1.05);
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
        std::cerr<<"GLFW init fail"<<std::endl;
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
    
    while(!glfwWindowShouldClose(win)){
        if(glfwGetKey(win,GLFW_KEY_ESCAPE)==GLFW_PRESS)
            glfwSetWindowShouldClose(win,true);
        
        int w,h;
        glfwGetFramebufferSize(win,&w,&h);
        float asp=(float)w/(float)h;
        glm::mat4 proj=glm::perspective(glm::radians(45.0f),asp,0.1f,100.0f);
        
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        
        float tm=glfwGetTime();
        float spd=tm*.8f,inw=tm*.15f;
        float rad=fmax(5.0f-inw,.8f);
        float ang=spd*2.0f;
        float cx=cos(ang)*rad,cz=sin(ang)*rad;
        float cy=.3f+sin(spd*.5f)*.5f;
        
        glUniform1f(glGetUniformLocation(prog,"t"),tm);
        glUniform3f(glGetUniformLocation(prog,"cam"),cx,cy,cz);
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