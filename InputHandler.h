#pragma once
#include <GLFW/glfw3.h>

class InputHandler {
public:
    static void SetupCallbacks(GLFWwindow* window);
private:
    static void MouseButtonCallback(GLFWwindow* w, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* w, double xpos, double ypos);
};
