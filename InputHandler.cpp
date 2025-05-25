#include "InputHandler.h"
#include "Camera.h"

extern Camera camera;    // в main.cpp объ€вим глобальный объект

void InputHandler::SetupCallbacks(GLFWwindow* window) {
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
}

void InputHandler::MouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {  
   if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {  
       camera.mouseCaptured = true;  
       glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
       camera.firstMouse = true;  
   }  
   if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {  
       camera.mouseCaptured = false;  
       glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  
   }  
}

void InputHandler::CursorPosCallback(GLFWwindow* w, double xpos, double ypos) {
    camera.ProcessMouse(float(xpos), float(ypos));
}
