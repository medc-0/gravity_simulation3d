#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

const char* vertexShaderSource = R"glsl(#version 330 core
layout(location = 0) in vec3 aPos;uniform mat4 model;uniform mat4 view; uniform mat4 projection;
void main(){gl_Position=projection * view * model*vec4(aPos,1.0);}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
uniform vec4 objectColor;
void main() {
    FragColor = objectColor;
}
)glsl";

/* Global Variables */
bool running = true;
bool pause = false;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float lastX = 400.0f, lastY = 300.0f;
float yaw = -90.0f;
float pitch = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

const double G = 6.643e-11; // m^3 kg^-1 s^-2
const float cl = 299792458.0f;
float initMass = 5.0f * pow(10, 20) / 5;

GLFWwindow* StartGLUI();
GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource);
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount);
void UpdateCam(GLuint shaderProgram, glm::vec3 cameraPos);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

void mouse_callback(GLFWwindow* window, double xPos, double yPos);
glm::vec sphericalToCartesian(float r, float theta, float phi);  // 0 theta p phi r=how far from center
void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount);

