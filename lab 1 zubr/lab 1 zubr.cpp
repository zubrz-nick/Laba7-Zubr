#define GLFW_DLL
#define GLEW_DLL
#include <iostream>
#include "glew-2.1.0/include/GL/glew.h"
#include "glfw-3.4.bin.WIN64/include/GLFW/glfw3.h"
#include "Shader.h"
#include "Model.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 256.0f;
float lastY = 256.0f;
bool firstMouse = true;

float cameraSpeed = 0.05f;
float mouseSensitivity = 0.1f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float angleRF = 0.0f;
float angleTG = 0.0f;
float angleYH = 0.0f;

const float ANGLE_TG_MIN = -90.0f;
const float ANGLE_TG_MAX = 60.0f;

const float ROTATION_SPEED = 50.0f;

glm::vec3 lightPos = glm::vec3(2.0f, 2.0f, 2.0f);

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        angleRF += deltaTime * ROTATION_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        angleRF -= deltaTime * ROTATION_SPEED;
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        float newAngle = angleTG + deltaTime * ROTATION_SPEED;
        if (newAngle <= ANGLE_TG_MAX) angleTG = newAngle;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        float newAngle = angleTG - deltaTime * ROTATION_SPEED;
        if (newAngle >= ANGLE_TG_MIN) angleTG = newAngle;
    }

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        angleYH += deltaTime * ROTATION_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        angleYH -= deltaTime * ROTATION_SPEED;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
glm::vec3 getMeshCenter(Model& model, int meshIndex) {
    if (meshIndex >= model.meshes.size() || model.meshes[meshIndex].vertices.empty()) {
        return glm::vec3(0.0f);
    }

    glm::vec3 minBounds(FLT_MAX), maxBounds(-FLT_MAX);
    for (const auto& vertex : model.meshes[meshIndex].vertices) {
        minBounds.x = std::min(minBounds.x, vertex.Position.x);
        minBounds.y = std::min(minBounds.y, vertex.Position.y);
        minBounds.z = std::min(minBounds.z, vertex.Position.z);
        maxBounds.x = std::max(maxBounds.x, vertex.Position.x);
        maxBounds.y = std::max(maxBounds.y, vertex.Position.y);
        maxBounds.z = std::max(maxBounds.z, vertex.Position.z);
    }
    return (minBounds + maxBounds) / 2.0f;
}
int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* Okno;
    Okno = glfwCreateWindow(800, 600, "Lab 7", NULL, NULL);

    if (!Okno) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Okno);

    GLenum ret = glewInit();
    if (GLEW_OK != ret) {
        fprintf(stderr, "ERROR GLEW Init: %s\n", glewGetErrorString(ret));
        return -2;
    }

    glfwSetInputMode(Okno, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(Okno, mouse_callback);

    glEnable(GL_DEPTH_TEST);

    Shader shader("shader.vert", "shader.frag");
    Model ourModel("Lab_3_VAR_11_Zubreichuk.obj");
    glm::vec3 lightPos(2.0f, 5.0f, 4.0f);
    glm::vec3 LightColor(0.8f, 0.8f, 0.8f);

    const int MESH_COUNT = ourModel.meshes.size();
    std::vector<glm::vec3> meshCenters(MESH_COUNT);
    for (int i = 0; i < MESH_COUNT; i++) {
        meshCenters[i] = getMeshCenter(ourModel, i);
    }

    glm::vec3 pivotPoint = glm::vec3(-1.37f, -0.518f, 0.0f);
    glm::vec3 pivotTG = glm::vec3(-1.078f, -0.137f, -0.138f);
    glm::vec3 pivotYH = glm::vec3(0.0f, 1.056f, 0.0f);

    std::vector<glm::mat4> transforms(MESH_COUNT);


    while (!glfwWindowShouldClose(Okno)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(Okno);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1024.0f / 1024.0f, 0.1f, 100.0f);

        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        shader.setVec3("material.ambient", 0.15f, 0.3f, 0.8f);
        shader.setVec3("material.diffuse", 0.2f, 0.4f, 1.0f);
        shader.setVec3("material.specular", 0.7f, 0.8f, 1.0f);
        shader.setFloat("material.shininess", 64.0f);

        shader.setVec3("light.position", lightPos.x, lightPos.y, lightPos.z);
        shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        shader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
        shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        shader.setVec3("viewPos", cameraPos);


        transforms[0] = glm::mat4(1.0f);

        glm::mat4 baseRot = glm::mat4(1.0f);
        baseRot = glm::translate(baseRot, pivotPoint);
        baseRot = glm::rotate(baseRot, glm::radians(angleRF), glm::vec3(0.0f, 1.0f, 0.0f));
        baseRot = glm::translate(baseRot, -pivotPoint);

        transforms[3] = baseRot;

        glm::mat4 tgRot = baseRot;
        tgRot = glm::translate(tgRot, pivotTG);
        tgRot = glm::rotate(tgRot, glm::radians(angleTG), glm::vec3(0.0f, 0.0f, 1.0f));
        tgRot = glm::translate(tgRot, -pivotTG);
        transforms[2] = tgRot;

        glm::mat4 yhRot = tgRot;
        yhRot = glm::translate(yhRot, pivotYH);
        yhRot = glm::rotate(yhRot, glm::radians(angleYH), glm::vec3(1.0f, 0.0f, 0.0f));
        yhRot = glm::translate(yhRot, -pivotYH);
        transforms[1] = yhRot;

        ourModel.Draw(shader, transforms.data(), MESH_COUNT);

        glfwSwapBuffers(Okno);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}