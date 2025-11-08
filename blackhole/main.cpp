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
uniform vec2 res;

const float sr = 0.12;
const float eh = 0.18;
const float di = 0.35;
const float do_ = 4.5;
const int MAX_STEPS = 256;
const int LIGHT_SAMPLES = 4;

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
        freq*=2.1;amp*=.48;
    }
    return val;
}

float fbm3(vec3 p){
    float val=0,amp=.5;
    for(int i=0;i<4;i++){
        val+=amp*(h3(p)-.5);
        p*=2.1;amp*=.5;
    }
    return val*.5+.5;
}

float getDensity(vec3 pos,float r){
    float dh=abs(pos.y);
    float dt=.4+.25*sin(t*.8);
    if(dh>=dt||r<=di||r>=do_)return 0.;
    
    float dn=(r-di)/(do_-di);
    float av=(1./pow(dn+.05,0.4))*4.2;
    float ang=atan(pos.z,pos.x)+t*av;
    
    vec2 tc=vec2(ang*9.+t*1.6,dn*24);
    float turb=fbm(tc);
    
    vec3 p3=pos*4.;
    float turb3d=fbm3(p3+t*.4);
    
    float sp=sin(ang*22-dn*32+t*7.)*.5+.5;
    float sp2=sin(ang*28+dn*22-t*5.5)*.5+.5;
    
    float hf=1.-pow(dh/dt,1.1);
    float dens=turb*turb3d*hf*(.75+sp*.25);
    dens*=(1.-pow(dn,.55));
    
    float hs=n(vec2(ang*14+t*3.5,dn*16));
    float flare=n(vec2(ang*6-t*2.2,dn*9));
    
    if(hs>.75) dens*=7.+sin(t*22.)*2.5;
    if(flare>.83) dens*=5.+sp2*3.5;
    
    return dens;
}

vec3 getColor(vec3 pos,float r,float dens){
    float dn=(r-di)/(do_-di);
    float ang=atan(pos.z,pos.x)+t*3.5;
    
    float turb=fbm(vec2(ang*9.+t*1.2,dn*22));
    float sp=sin(ang*22-dn*32+t*7.)*.5+.5;
    float sp2=sin(ang*28+dn*22-t*5.5)*.5+.5;
    float hs=n(vec2(ang*14+t*3.5,dn*16));
    float flare=n(vec2(ang*6-t*2.2,dn*9));
    
    vec3 c;
    float tmp=1.-dn;
    
    if(tmp>.85){
        c=mix(vec3(.25,.4,1.),vec3(.9,1.,1.2),(tmp-.85)*6.66);
        c+=vec3(.15,.3,1.2)*hs*3.;
        c+=vec3(.4,.6,1.)*flare*2.;
    }else if(tmp>.7){
        c=mix(vec3(.08,.5,1.),vec3(.5,.8,1.),(tmp-.7)*6.66);
        c+=vec3(.2,.9,1.2)*turb*1.2;
        c+=vec3(.3,.6,.9)*sp2*.8;
    }else if(tmp>.55){
        c=mix(vec3(.4,.8,.08),vec3(.8,.9,.35),(tmp-.55)*6.66);
        c+=vec3(.9,1.,.15)*sp*.9;
        c+=vec3(.6,.8,.25)*turb*.7;
    }else if(tmp>.4){
        c=mix(vec3(.8,.5,.03),vec3(.9,.7,.2),(tmp-.4)*6.66);
        c+=vec3(1.,.6,.08)*turb*.9;
        c+=vec3(.8,.5,.1)*sp2*.6;
    }else if(tmp>.25){
        c=mix(vec3(.9,.3,.015),vec3(.95,.55,.08),(tmp-.25)*6.66);
        c+=vec3(1.,.4,.03)*sp*.6;
    }else{
        c=mix(vec3(.5,.015,.03),vec3(.85,.18,.25),tmp*4.);
        c+=vec3(.9,.08,.3)*sp*.5;
        c+=vec3(.6,.03,.2)*flare*.4;
    }
    
    c.r+=turb*.3+sp*.2;
    c.g+=sin(ang*24+t*3.8)*.2;
    c.b+=cos(ang*30-t*4.5)*.25;
    
    return c;
}

vec3 calcLighting(vec3 pos,vec3 d,float dens){
    vec3 lightDir=normalize(vec3(.3,.8,.5));
    
    float shadow=1.;
    vec3 p=pos;
    for(int i=0;i<4;i++){
        p+=lightDir*.2;
        float r=length(p);
        float sd=getDensity(p,r);
        shadow*=exp(-sd*.3);
        if(shadow<.05)break;
    }
    
    float diff=max(dot(normalize(vec3(0,1,0)),lightDir),0.)*.5+.5;
    float rim=pow(1.-abs(dot(d,normalize(pos))),2.)*.3;
    
    return vec3(diff*shadow+rim);
}

vec3 march(vec3 o,vec3 d){
    vec3 pos=o;
    float td=0;
    vec3 col=vec3(0);
    float alpha=0;
    
    for(int i=0;i<MAX_STEPS;i++){
        float r=length(pos);
        
        if(r<eh){
            col+=vec3(.04,.015,.08)*(1.-alpha);
            return col;
        }
        
        float dens=getDensity(pos,r);
        
        if(dens>.01){
            vec3 c=getColor(pos,r,dens);
            vec3 light=calcLighting(pos,d,dens);
            
            float tmp=1.-(r-di)/(do_-di);
            float br=dens*(3.5+tmp*5.);
            br*=(1.+sin(t*9.+atan(pos.z,pos.x)*15.)*.5);
            br*=(1.+cos(atan(pos.z,pos.x)*8.-t*3.)*.35);
            
            vec3 dc=c*br*light;
            float a=dens*.2;
            col+=dc*a*(1.-alpha);
            alpha+=a*(1.-alpha);
            if(alpha>.98)break;
        }
        
        float g=(sr*sr)/(r*r+.006);
        vec3 gd=-normalize(pos);
        d=normalize(d+gd*g*.5);
        
        float step=.02+r*.01;
        if(r>di-.7&&r<do_+.7)step*=.3;
        pos+=d*step;
        td+=step;
        if(td>35)break;
    }
    
    vec3 sd=normalize(d);
    float sn=h(sd.xy*280+sd.z*150);
    if(sn>.998){
        float sb=(sn-.998)*800;
        vec3 sc=mix(vec3(.9,.92,.85),vec3(.85,.92,.98),h(sd.yz*165));
        float twinkle=.7+.3*sin(sn*1200.+t*6.);
        col+=sc*sb*twinkle*(1.-alpha);
    }
    
    float bg=fbm(sd.xy*5.+t*.025)*.035;
    vec3 nebula=mix(vec3(.04,.02,.08),vec3(.08,.04,.12),bg);
    nebula+=vec3(.015,.008,.02)*fbm(sd.yz*3.-t*.02);
    col+=nebula*(1.-alpha);
    
    col+=vec3(.0005,.001,.004)*(1.-alpha);
    return col;
}

void main(){
    vec2 uv_=(uv-.5)*2;
    uv_.x*=res.x/res.y;
    
    vec3 ro=cam;
    vec3 tgt=vec3(0);
    vec3 fwd=normalize(tgt-ro);
    vec3 rt=normalize(cross(vec3(0,1,0),fwd));
    vec3 up=cross(fwd,rt);
    vec3 rd=normalize(fwd+uv_.x*rt+uv_.y*up);
    
    vec3 col=march(ro,rd);
    
    col*=2.2;
    vec3 a=col*(col+.0245786)-.000090537;
    vec3 b=col*(.983729*col+.4329510)+.238081;
    col=a/b;
    
    col=pow(col,vec3(.82));
    col.r=pow(col.r,.88);
    col.g=pow(col.g,.92);
    col.b=pow(col.b,1.15);
    
    float vig=1.-length(uv_*.28);
    vig=pow(vig,1.4);
    col*=.25+vig*.75;
    
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

int main(){
    if(!glfwInit()){
        std::cerr<<"GLFW init fail"<<std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* win=glfwCreateWindow(1600,900,"Black Hole - Ultra Quality",nullptr,nullptr);
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
    
    std::cout<<"Ultra Quality Black Hole Renderer\n";
    std::cout<<"Features: Volumetric lighting, 2x2 supersampling, ACES tone mapping\n";
    std::cout<<"Press ESC to exit\n\n";
    
    while(!glfwWindowShouldClose(win)){
        if(glfwGetKey(win,GLFW_KEY_ESCAPE)==GLFW_PRESS)
            glfwSetWindowShouldClose(win,true);
        
        float tm=glfwGetTime();
        float spd=tm*.6f,inw=tm*.12f;
        float rad=fmax(5.5f-inw,1.2f);
        float ang=spd*1.8f;
        float cx=cos(ang)*rad,cz=sin(ang)*rad;
        float cy=.45f+sin(spd*.4f)*.25f;
        
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