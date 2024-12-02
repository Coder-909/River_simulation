#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>

using namespace std;
struct Particle {
    float x, y;     
    float vx, vy,ax,ay;   
    float life;   //have not used it yet..will use later
};

vector<Particle> particles;
float sourceX = -0.9f, sourceY = 0.0f;  
int maxParticles = 1500000;  
float flowSpeed = 0.005f;  //can be set by user
float riverWidth = 0.05f;    

unsigned int VAO, VBO;

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in float aLife;
    out float life;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        life = aLife;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in float life;
    out vec4 FragColor;
    void main() {
        float alpha = life; // Use life for transparency
        FragColor = vec4(0.0, 0.3 + 0.3 * life, 1.0, alpha); // Blue gradient
    }
)";

Particle createParticle() {
    Particle p;
    p.x = sourceX;
    p.y = sourceY + ((rand() % 100) / 100.0f - 0.5f) * riverWidth; 
    p.vx = (flowSpeed + (rand() % 10 - 5) / 10000.0f)*5; 
    p.vy = ((rand() % 50 - 25) / 50000.0f)*5;      
    p.ax = (rand()%10)/10000.0f;
    p.ay =(rand()%10)/1000.0f;  
    p.life = 1.0f;
    return p;
}

void updateParticles() {
    int tempHeight = (rand()%10)/10.0f;
   
    for (auto& p : particles) {
        p.x += p.vx;
        p.y += p.vy;
        p.vx += p.ax;
        // if (rand()%10 >5){
        //     p.ay *= -1;
        // }
        p.vy += p.ay;
        if (p.y < sourceY - tempHeight|| p.y > sourceY + tempHeight) {
            p.ay = -p.ay;
        }

    }

    particles.erase(
        remove_if(particles.begin(), particles.end(),
                       [](Particle& p) { return p.x > 1.0f;}),
        particles.end());

    while (particles.size()<maxParticles) {
        particles.push_back(createParticle());
    }
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        cerr << "Error compiling shader: " << infoLog << endl;
    }
    return shader;
}

unsigned int createShaderProgram() {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        cerr << "Error linking shader program: " << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

int main() {
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW!" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Sediment flow Simulation", nullptr, nullptr);
    if (!window) {
        cerr << "Failed to create GLFW window!" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Failed to initialize GLAD!" << endl;
        return -1;
    }

    unsigned int shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(float) * 3, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window)) {
        updateParticles();

        vector<float> vertexData;
        for (const auto& p : particles) {
            vertexData.push_back(p.x); 
            vertexData.push_back(p.y);
            vertexData.push_back(p.life); 
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(float), vertexData.data());

        //sand :
        glClearColor(0.796f, 0.741f, 0.576f, 1.0f);
        //grey clay: 
        //glClearColor(0.74f, 0.72f, 0.63f, 1.0f);
        //red clay: 
        //glClearColor(0.30f, 0.207f, 0.141f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, particles.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}