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
glm::vec3 sphericalToCartesian(float r, float theta, float phi);  // 0 theta p phi r=how far from center
void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount);

class Object {
    public:
        GLuint VAO, VBO;
        glm::vec3 position = glm::vec3(400, 300, 0);
        glm::vec3 velocity = glm::vec3(0, 0, 0);
        size_t vertexCount;
        glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

        bool Initializing = false;
        bool Launched = false;
        bool Target = false;

        float mass;
        float density; // kg / m^3 HYDROGEN
        float radius;

        glm::vec3 LastPos = position;

        Object(glm::vec3 initPosition, glm::vec3 initVelocity, float mass, float density = 3344) {
            this->position = initPosition;
            this->velocity = initVelocity;
            this->mass = mass;
            this->density = density;
            this->radius = pow( ((3 * this->mass/this->density)/(4 * 3.14159265359)), (1.0f/3.0f)) / 100000;

            // gen vertices (centered at origin)
            std::vector<float> vertices = Draw();
            vertexCount = vertices.size();

            CreateVBOVAO(VAO, VBO, vertices.data(), vertexCount);
        }

        std::vector<float> Draw() {
            std::vector<float> vertices;
            int stacks = 10;
            int sectors = 10;

            // generate circumference points using integer steps
            for (float i = 0.0f; i <= stacks; ++i) {
                float theta1 = (i / stacks) * glm::pi<float>();
                float theta2 = (i+1) / stacks * glm::pi<float>();
                for (float j = 0.0f; j < sectors; ++j) {
                    float phi1 = j / sectors * 2 * glm::pi<float>();
                    float phi2 = (j+1) / sectors * 2 * glm::pi<float>();
                    glm::vec3 v1 = sphericalToCartesian(radius, theta1, phi1);
                    glm::vec3 v2 = sphericalToCartesian(radius, theta1, phi2);
                    glm::vec3 v3 = sphericalToCartesian(radius, theta2, phi1);
                    glm::vec3 v4 = sphericalToCartesian(radius, theta2, phi2);

                    // Triangle 1: v1-v2-v3
                    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z}); //    /|
                    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z}); //   / |
                    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z}); //  /__|

                    // Triangle 2: v1-v4-v3
                    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z});
                    vertices.insert(vertices.end(), {v4.x, v4.y, v4.z});
                    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z});
                }
            }
            return vertices;
        }

        void UpdatePos() {
            this->position[0] += this->velocity[0] / 94;
            this->position[1] += this->velocity[1] / 94;
            this->position[2] += this->velocity[2] / 94;
            this->radius = pow(((3 * this->mass / this->density)/(4 * 3.14159265359)), (1.0f/3.0f)) / 100000;
        }

        void UpdateVertices() {
            // Generate new vertices with current given radius
            std::vector<float> vertices = Draw();

            // Update the VBO with new vertex data
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        }
        glm::vec3 GetPos() const {
            return this->position;
        }
        void accelerate(float x, float y, float z) {
            this->velocity[0] += x / 96;
            this->velocity[1] += y / 96;
            this->velocity[2] += z / 96;
        }

        float CheckCollision(const Object& other) {
            float dx = other.position[0] - this->position[0];
            float dy = other.position[1] - this->position[1];
            float dz = other.position[2] - this->position[2];
            float distance = std::pow(dx*dx + dy*dy + dz*dz, (1.0f/2.0f));
            if (other.radius + this->radius > distance) {
                return -0.2f;
            }
            return 1.0f;
        }
};
std::vector<Object> objs = {};