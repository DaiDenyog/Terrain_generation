#pragma once
#include <glm/glm.hpp>

class Camera {
public:
    Camera();
    void ProcessMouse(float xpos, float ypos);
    void ProcessKeyboard(float deltaTime);
    glm::mat4 GetViewMatrix() const;
    glm::vec3 Position, Front, Up;

private:
    float yaw, pitch;
    float lastX, lastY;
    bool firstMouse;
    bool mouseCaptured;
};
