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
uniform int renderMode;

const float sr = 0.12;
const float eh = 0.18;
const float di = 0.35;
const float do_ = 4.0;

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
    for(int i=0;i<8;i++){
        val+=amp*n(p*freq);
        freq*=2.1;amp*=.48;
    }
    return val;
}

float sdTorus(vec3 p,vec2 t){
    vec2 q=vec2(length(p.xz)-t.x,p.y);
    return length(q)-t.y;
}

vec3 march(vec3 o,vec3 d){
    vec3 pos=o;
    float td=0;
    vec3 col=vec3(0);
    float alpha=0;
    
    for(int i=0;i<256;i++){
        float r=length(pos);
        
        if(r<eh){
            col+=vec3(.05,.02,.1)*(1.-alpha);
            return col;
        }
        
        float dh=abs(pos.y);
        float dt=.3+.18*sin(t*.9);
        
        if(dh<dt&&r>di&&r<do_){
            float dn=(r-di)/(do_-di);
            float av=(1./pow(dn+.08,0.4))*3.2;
            float ang=atan(pos.z,pos.x)+t*av;
            
            vec2 tc=vec2(ang*7.5+t*1.2,dn*18);
            float turb=fbm(tc);
            
            float sp=sin(ang*15-dn*25+t*5.5)*.5+.5;
            float sp2=sin(ang*23+dn*18-t*4.)*.5+.5;
            
            float hf=1.-pow(dh/dt,1.5);
            float dens=turb*hf*(.65+sp*.35);
            dens*=(1.-pow(dn,.7));
            
            float hs=n(vec2(ang*10+t*2.5,dn*12));
            float flare=n(vec2(ang*4-t*1.8,dn*6));
            
            if(hs>.8) dens*=5.+sin(t*18)*1.5;
            if(flare>.88) dens*=3.+sp2*2.;
            
            vec3 c;
            float tmp=1.-dn;
            
            if(tmp>.8){
                c=mix(vec3(.5,.6,1.2),vec3(1.2,1.2,1.4),(tmp-.8)*5);
                c+=vec3(.3,.5,1.5)*hs*4.;
                c+=vec3(.8,.9,1.2)*flare*2.5;
            }else if(tmp>.65){
                c=mix(vec3(.2,.8,1.2),vec3(.8,1.,1.2),(tmp-.65)*6.66);
                c+=vec3(.4,1.2,1.5)*turb*1.2;
                c+=vec3(.6,.8,1.)*sp2*.8;
            }else if(tmp>.5){
                c=mix(vec3(.7,1.,.2),vec3(1.,1.,.6),(tmp-.5)*6.66);
                c+=vec3(1.,1.2,.3)*sp*.9;
                c+=vec3(.9,1.,.5)*turb*.7;
            }else if(tmp>.35){
                c=mix(vec3(1.,.65,.08),vec3(1.,.95,.35),(tmp-.35)*6.66);
                c+=vec3(1.3,.8,.15)*turb*1.;
                c+=vec3(1.,.7,.2)*sp2*.6;
            }else if(tmp>.2){
                c=mix(vec3(1.,.4,.05),vec3(1.,.7,.15),(tmp-.2)*6.66);
                c+=vec3(1.2,.5,.1)*sp*.7;
            }else{
                c=mix(vec3(.6,.03,.08),vec3(1.,.25,.4),tmp*5);
                c+=vec3(1.,.15,.5)*sp*.6;
                c+=vec3(.8,.1,.3)*flare*.5;
            }
            
            c.r+=turb*.4+sp*.2;
            c.g+=sin(ang*18+t*2.8)*.25;
            c.b+=cos(ang*24-t*3.5)*.3;
            
            float br=dens*(3.5+tmp*5.);
            br*=(1.+sin(t*6.5+ang*10.)*.6);
            br*=(1.+cos(ang*5.-t*2.)*.4);
            
            vec3 dc=c*br;
            float a=dens*.18;
            col+=dc*a*(1.-alpha);
            alpha+=a*(1.-alpha);
            if(alpha>.97)break;
        }
        
        float g=(sr*sr)/(r*r+.008);
        vec3 gd=-normalize(pos);
        d=normalize(d+gd*g*.45);
        
        float step=.025+r*.012;
        if(r>di-.6&&r<do_+.6)step*=.35;
        pos+=d*step;
        td+=step;
        if(td>30)break;
    }
    
    vec3 sd=normalize(d);
    float sn=h(sd.xy*250+sd.z*130);
    if(sn>.9975){
        float sb=(sn-.9975)*1500;
        vec3 sc=mix(vec3(1.,.95,.85),vec3(.85,.95,1.),h(sd.yz*145));
        float twinkle=.7+.3*sin(sn*1000.+t*5.);
        col+=sc*sb*twinkle*(1.-alpha);
    }
    
    float bg=fbm(sd.xy*4.+t*.03)*.04;
    vec3 nebula=mix(vec3(.08,.04,.12),vec3(.12,.06,.18),bg);
    col+=nebula*(1.-alpha);
    
    col+=vec3(.001,.002,.008)*(1.-alpha);
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
    
    c=c/(c+vec3(.4));
    c=pow(c,vec3(.88));
    c=pow(c,vec3(1./2.15));
    
    c.r=pow(c.r,.92);
    c.g=pow(c.g,.96);
    c.b=pow(c.b,1.08);
    
    float vig=1.-length(uv_*.35);
    vig=pow(vig,1.2);
    c*=.3+vig*.7;
    
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

float gCamDist=5.5f;
float gCamAngle=0.0f;
float gCamHeight=0.4f;
bool gAutoCam=true;

void scroll_callback(GLFWwindow* w,double x,double y){
    gCamDist*=(y>0)?0.9f:1.1f;
    gCamDist=fmax(1.5f,fmin(gCamDist,12.0f));
}

void key_callback(GLFWwindow* w,int key,int scan,int action,int mods){
    if(action==GLFW_PRESS){
        if(key==GLFW_KEY_C) gAutoCam=!gAutoCam;
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
    
    GLFWwindow* win=glfwCreateWindow(1600,900,"Black Hole",nullptr,nullptr);
    if(!win){
        std::cerr<<"Window fail"<<std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win,fbResize);
    glfwSetScrollCallback(win,scroll_callback);
    glfwSetKeyCallback(win,key_callback);
    
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
        
        float tm=glfwGetTime();
        
        if(gAutoCam){
            gCamAngle=tm*.25f;
        }else{
            if(glfwGetKey(win,GLFW_KEY_A)==GLFW_PRESS)gCamAngle-=.02f;
            if(glfwGetKey(win,GLFW_KEY_D)==GLFW_PRESS)gCamAngle+=.02f;
        }
        
        if(glfwGetKey(win,GLFW_KEY_W)==GLFW_PRESS)gCamHeight+=.01f;
        if(glfwGetKey(win,GLFW_KEY_S)==GLFW_PRESS)gCamHeight-=.01f;
        gCamHeight=fmax(-2.0f,fmin(gCamHeight,2.0f));
        
        float cx=cos(gCamAngle)*gCamDist;
        float cz=sin(gCamAngle)*gCamDist;
        float cy=gCamHeight+sin(tm*.3f)*.15f;
        
        int w,h;
        glfwGetFramebufferSize(win,&w,&h);
        float asp=(float)w/(float)h;
        glm::mat4 proj=glm::perspective(glm::radians(45.0f),asp,0.1f,100.0f);
        
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        
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