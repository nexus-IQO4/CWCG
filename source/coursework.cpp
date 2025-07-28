#include <iostream>
#include <vector>
#include <cmath>
#include <string>

#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>

// Forward declarations for callback functions
void processInput(GLFWwindow* window);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Timing variables
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Scene lighting setup
glm::vec3 globalLightPosition(0.0f, 10.0f, 10.0f); // Main light from above and front
glm::vec3 globalLightColor(0.8f, 0.8f, 0.7f);    // A bright, slightly warm white light

// Shader program and uniform IDs
GLuint shaderProgramID;
GLuint modelMatrixID, viewMatrixID, projectionMatrixID;
GLuint lightPosID, lightColorID, viewPosID;
GLuint pointLightPositionsID, pointLightColorsID;

// Point light definitions for atmosphere
glm::vec3 pointLight1_Pos(0.0f, 0.0f, 10.0f); // Blueish fill light from the front
glm::vec3 pointLight2_Pos(0.0f, 0.0f, -10.0f); // Reddish fill light from the back
glm::vec3 lightColors[2];

// Structure for a renderable object in the scene
struct Renderable {
    Model* model = nullptr;
    glm::mat4 transform = glm::mat4(1.0f);
};

std::vector<Model*> loadedModels;
std::vector<Renderable> sceneObjects;

// Screen dimensions
const unsigned int SCREEN_WIDTH = 1024;
const unsigned int SCREEN_HEIGHT = 768;

// Camera setup
Camera camera(glm::vec3(0.0f, 1.0f, 15.0f));
float lastMouseX = SCREEN_WIDTH / 2.0f;
float lastMouseY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

int main(void)
{
    char currentPath[FILENAME_MAX];
    GetCurrentDir(currentPath, sizeof(currentPath));
    std::cout << "Current working directory: " << currentPath << std::endl;

    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Coursework - Two Planets", NULL, NULL); // MODIFIED: Window Title
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = true;
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaderProgramID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    modelMatrixID = glGetUniformLocation(shaderProgramID, "model");
    viewMatrixID = glGetUniformLocation(shaderProgramID, "view");
    projectionMatrixID = glGetUniformLocation(shaderProgramID, "projection");
    lightPosID = glGetUniformLocation(shaderProgramID, "globalLightPos");
    lightColorID = glGetUniformLocation(shaderProgramID, "globalLightColor");
    viewPosID = glGetUniformLocation(shaderProgramID, "viewPos");
    pointLightPositionsID = glGetUniformLocation(shaderProgramID, "pointLightPositions");
    pointLightColorsID = glGetUniformLocation(shaderProgramID, "pointLightColors");

    // Load Models and Textures for the Two Planets
    const std::string assetPath = "../assets/";

    // Create Planet 1 (Rocky)
    Model* planet1Model = new Model((assetPath + "moon.obj").c_str());
    if (planet1Model && !planet1Model->vertices.empty()) {
        loadedModels.push_back(planet1Model);
        planet1Model->addTexture((assetPath + "moon_diffuse.png").c_str(), "diffuse");
        planet1Model->addTexture((assetPath + "moon_specular.png").c_str(), "specular");

        Renderable planet1Object;
        planet1Object.model = planet1Model;
        planet1Object.transform = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f));
        planet1Object.transform = glm::scale(planet1Object.transform, glm::vec3(1.5f)); // MODIFIED: Set size
        sceneObjects.push_back(planet1Object);
    }

    // Planet 2 (Fiery/Volcanic)
    Model* planet2Model = new Model((assetPath + "moon.obj").c_str());
    if (planet2Model && !planet2Model->vertices.empty()) {
        loadedModels.push_back(planet2Model);
        planet2Model->addTexture((assetPath + "mars_diffuse.png").c_str(), "diffuse");
        //specular map
        planet2Model->addTexture((assetPath + "mars_specular.png").c_str(), "specular");

        Renderable planet2Object;
        planet2Object.model = planet2Model;
        planet2Object.transform = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
        planet2Object.transform = glm::scale(planet2Object.transform, glm::vec3(1.5f)); // MODIFIED: Set to same size as Planet 1

        planet2Object.model->ka = 0.1f;
        planet2Object.model->kd = 0.9f;
        planet2Object.model->ks = 0.5f;

        sceneObjects.push_back(planet2Object);
    }

    // Main Render Loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.01f, 0.01f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgramID);

        glm::mat4 viewMatrix = camera.GetViewMatrix();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (height == 0) ? 1.0f : (float)width / (float)height, 0.1f, 200.0f);

        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniform3fv(viewPosID, 1, &camera.Position[0]);
        glUniform3fv(lightPosID, 1, &globalLightPosition[0]);
        glUniform3fv(lightColorID, 1, &globalLightColor[0]);

        // Set point light colors
        lightColors[0] = glm::vec3(0.1f, 0.2f, 0.7f); // Blue
        lightColors[1] = glm::vec3(0.7f, 0.2f, 0.1f); // Red

        glm::vec3 pointLightPositions[2] = { pointLight1_Pos, pointLight2_Pos };
        glUniform3fv(pointLightPositionsID, 2, glm::value_ptr(pointLightPositions[0]));
        glUniform3fv(pointLightColorsID, 2, glm::value_ptr(lightColors[0]));

        // Animate and Render Planets
        float time = glfwGetTime();

        // Planet 1 rotation
        if (sceneObjects.size() > 0) {
            glm::mat4 baseTransform = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 0.0f));
            baseTransform = glm::scale(baseTransform, glm::vec3(1.5f));
            sceneObjects[0].transform = glm::rotate(baseTransform, time * 0.2f, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        // Planet 2 rotation
        if (sceneObjects.size() > 1) {
            glm::mat4 baseTransform = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
            baseTransform = glm::scale(baseTransform, glm::vec3(1.5f));
            sceneObjects[1].transform = glm::rotate(baseTransform, time * -0.3f, glm::vec3(0.0f, 1.0f, 0.1f));
        }

        // Render all objects
        for (const auto& object : sceneObjects) {
            if (object.model) {
                glUniform1f(glGetUniformLocation(shaderProgramID, "ka"), object.model->ka);
                glUniform1f(glGetUniformLocation(shaderProgramID, "kd"), object.model->kd);
                glUniform1f(glGetUniformLocation(shaderProgramID, "ks"), object.model->ks);
                glUniform1f(glGetUniformLocation(shaderProgramID, "Ns"), object.model->Ns);

                glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(object.transform));
                object.model->draw(shaderProgramID);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    for (Model* model : loadedModels) {
        if (model) {
            model->deleteBuffers();
            delete model;
        }
    }
    glDeleteProgram(shaderProgramID);
    glfwTerminate();
    return 0;
}

// Input processing function
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, deltaTime);
}

// Mouse movement callback
void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Mouse scroll callback
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Window resize callback
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}