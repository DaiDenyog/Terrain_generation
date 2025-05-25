#include "Camera.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : Position{ 0,50,100 }, Front{ 0,-0.5f,-1 }, Up{ 0,1,0 },
    yaw{ -90 }, pitch{ -20 }, lastX{ 640 }, lastY{ 360 },
    firstMouse{ true }, mouseCaptured{ true }
{
}

void Camera::ProcessMouse(float xpos, float ypos) {
    if (!mouseCaptured) return;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoff = (xpos - lastX) * 0.1f;
    float yoff = (lastY - ypos) * 0.1f;
    lastX = xpos; lastY = ypos;
    yaw += xoff; pitch += yoff;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    glm::vec3 dir{
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    };
    Front = glm::normalize(dir);
}

void Camera::ProcessKeyboard(float deltaTime) {
    float speed = 50.0f * deltaTime;
    GLFWwindow* w = glfwGetCurrentContext();
    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) Position += speed * Front;
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) Position -= speed * Front;
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS)
        Position -= glm::normalize(glm::cross(Front, Up)) * speed;
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS)
        Position += glm::normalize(glm::cross(Front, Up)) * speed;
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(w, true);
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}
