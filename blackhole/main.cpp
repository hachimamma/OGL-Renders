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

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform float time;
uniform vec3 cameraPos;

const float schwarzschildRadius = 0.15;
const float eventHorizon = 0.2;
const float accretionDiskInner = 0.4;
const float accretionDiskOuter = 3.5;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float hash3(vec3 p) {
    return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    for(int i = 0; i < 6; i++) {
        value += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float noise3d(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float n = hash3(i);
    return n;
}

vec3 rayMarch(vec3 origin, vec3 direction) {
    vec3 pos = origin;
    float totalDist = 0.0;
    const int maxSteps = 200;
    const float maxDist = 25.0;
    vec3 accumulatedColor = vec3(0.0);
    float accumulatedAlpha = 0.0;
    
    for(int i = 0; i < maxSteps; i++) {
        float dist = length(pos);
        
        if(dist < eventHorizon) {
            return accumulatedColor + vec3(0.0);
        }
        
        float diskHeight = abs(pos.y);
        float diskThickness = 0.25 + 0.15 * sin(time * 1.2);
        
        if(diskHeight < diskThickness && dist > accretionDiskInner && dist < accretionDiskOuter) {
            float diskDist = dist - accretionDiskInner;
            float diskNorm = diskDist / (accretionDiskOuter - accretionDiskInner);
            
            float angularVelocity = (1.0 / sqrt(diskNorm + 0.1)) * 2.5;
            float angle = atan(pos.z, pos.x) + time * angularVelocity;
            
            vec2 turbCoord = vec2(angle * 6.0 + time * 0.8, diskNorm * 15.0);
            float turbulence = fbm(turbCoord);
            
            float spiral = sin(angle * 12.0 - diskNorm * 20.0 + time * 4.0) * 0.5 + 0.5;
            
            float heightFade = 1.0 - (diskHeight / diskThickness);
            heightFade = pow(heightFade, 2.0);
            
            float density = turbulence * heightFade * (0.6 + spiral * 0.4);
            density *= (1.0 - diskNorm * 0.5);
            
            float hotspot = noise(vec2(angle * 8.0 + time * 2.0, diskNorm * 10.0));
            if(hotspot > 0.82) {
                density *= 4.0 + sin(time * 15.0) * 1.0;
            }
            
            vec3 color;
            float temp = 1.0 - diskNorm;
            
            if(temp > 0.75) {
                color = mix(vec3(0.6, 0.7, 1.0), vec3(1.0, 1.0, 1.0), (temp - 0.75) * 4.0);
                color += vec3(0.4, 0.5, 1.2) * hotspot * 3.0;
            }
            else if(temp > 0.6) {
                color = mix(vec3(0.3, 0.9, 1.0), vec3(0.9, 0.95, 1.0), (temp - 0.6) * 6.66);
                color += vec3(0.5, 1.0, 1.2) * turbulence * 0.8;
            }
            else if(temp > 0.45) {
                color = mix(vec3(0.8, 1.0, 0.3), vec3(1.0, 1.0, 0.7), (temp - 0.45) * 6.66);
                color += vec3(1.0, 1.0, 0.4) * spiral * 0.6;
            }
            else if(temp > 0.3) {
                color = mix(vec3(1.0, 0.6, 0.1), vec3(1.0, 0.9, 0.4), (temp - 0.3) * 6.66);
                color += vec3(1.2, 0.7, 0.2) * turbulence * 0.7;
            }
            else {
                color = mix(vec3(0.5, 0.05, 0.1), vec3(1.0, 0.3, 0.5), temp * 3.33);
                color += vec3(1.0, 0.2, 0.6) * spiral * 0.5;
            }
            
            color.r += turbulence * 0.3;
            color.g += sin(angle * 15.0 + time * 2.0) * 0.2;
            color.b += cos(angle * 20.0 - time * 3.0) * 0.25;
            
            float brightness = density * (3.0 + temp * 4.0);
            brightness *= (1.0 + sin(time * 5.0 + angle * 8.0) * 0.5);
            
            vec3 diskColor = color * brightness;
            
            float alpha = density * 0.15;
            accumulatedColor += diskColor * alpha * (1.0 - accumulatedAlpha);
            accumulatedAlpha += alpha * (1.0 - accumulatedAlpha);
            
            if(accumulatedAlpha > 0.95) {
                break;
            }
        }
        
        float gravity = (schwarzschildRadius * schwarzschildRadius) / (dist * dist + 0.01);
        vec3 gravityDir = -normalize(pos);
        direction = normalize(direction + gravityDir * gravity * 0.4);
        
        float stepSize = 0.03 + dist * 0.015;
        if(dist > accretionDiskInner - 0.5 && dist < accretionDiskOuter + 0.5) {
            stepSize *= 0.4;
        }
        
        pos += direction * stepSize;
        totalDist += stepSize;
        
        if(totalDist > maxDist) {
            break;
        }
    }
    
    vec3 starDir = normalize(direction);
    float starNoise = hash(starDir.xy * 200.0 + starDir.z * 100.0);
    if(starNoise > 0.997) {
        float starBrightness = (starNoise - 0.997) * 1000.0;
        vec3 starColor = mix(vec3(1.0, 0.9, 0.8), vec3(0.8, 0.9, 1.0), hash(starDir.yz * 123.0));
        accumulatedColor += starColor * starBrightness * (1.0 - accumulatedAlpha);
    }
    
    float bgGlow = fbm(starDir.xy * 3.0 + time * 0.05) * 0.03;
    vec3 nebulaColor = vec3(0.1, 0.05, 0.15) * bgGlow;
    accumulatedColor += nebulaColor * (1.0 - accumulatedAlpha);
    
    accumulatedColor += vec3(0.002, 0.003, 0.01) * (1.0 - accumulatedAlpha);
    
    return accumulatedColor;
}

void main() {
    vec2 uv = (TexCoord - 0.5) * 2.0;
    
    vec3 rayOrigin = cameraPos;
    
    vec3 target = vec3(0.0, 0.0, 0.0);
    vec3 forward = normalize(target - rayOrigin);
    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), forward));
    vec3 up = cross(forward, right);
    
    vec3 rayDir = normalize(forward + uv.x * right + uv.y * up);
    
    vec3 color = rayMarch(rayOrigin, rayDir);
    
    color = color / (color + vec3(0.5));
    color = pow(color, vec3(0.9));
    color = pow(color, vec3(1.0/2.2));
    
    color.r = pow(color.r, 0.95);
    color.b = pow(color.b, 1.05);
    
    FragColor = vec4(color, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
    }
    return shader;
}

GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed:\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if(!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1600, 900, "Black Hole Renderer", nullptr, nullptr);
    if(!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    
#ifndef __APPLE__
    if(glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
#endif
    
    glViewport(0, 0, 1600, 900);
    
    float vertices[] = {
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    GLuint shaderProgram = createShaderProgram();
    
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    
    while(!glfwWindowShouldClose(window)) {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspectRatio = (float)width / (float)height;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        float time = glfwGetTime();
        
        float spiralSpeed = time * 0.8f;
        float inwardSpeed = time * 0.15f;
        
        float radius = 5.0f - inwardSpeed;
        radius = fmax(radius, 0.8f);
        
        float angle = spiralSpeed * 2.0f;
        float camX = cos(angle) * radius;
        float camZ = sin(angle) * radius;
        float camY = 0.3f + sin(spiralSpeed * 0.5f) * 0.5f;
        
        float distToCenter = sqrt(camX * camX + camZ * camZ);
        
        glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);
        glUniform3f(glGetUniformLocation(shaderProgram, "cameraPos"), camX, camY, camZ);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}