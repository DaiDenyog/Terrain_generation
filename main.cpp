#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "Shader.h"
#include "Terrain.h"
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// размеры окна
const unsigned SCR_W = 1920, SCR_H = 1080;

void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}

bool mouseCaptured = true;
bool firstMouse = true;
float lastX = SCR_W * 0.5f;
float lastY = SCR_H * 0.5f;

Camera camera;

void mouse_callback(GLFWwindow* /*wnd*/, double xpos, double ypos) {
    ImGuiIO& io = ImGui::GetIO();
    if (!mouseCaptured || io.WantCaptureMouse)
        return;   // либо в UI-режиме, либо ImGui перехватил мышь

    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float xoff = (float)xpos - lastX;
    float yoff = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    camera.ProcessMouseMovement(xoff, yoff);
}

// колбэк на нажатие кнопок мыши
void mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/) {
    ImGuiIO& io = ImGui::GetIO();

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Ћ ћ Ч переходим в FPS-режим
        mouseCaptured = true;
        // скрыть курсор и Ђзахватитьї его
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        lastX = (float)xpos;
        lastY = (float)ypos;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // ѕ ћ Ч переходим в UI-режим
        mouseCaptured = false;
        // показать курсор
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

int main() {
    // GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "Terrain", NULL, NULL);
    if (!window) { std::cerr << "Failed GLFW\n"; return -1; }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed GLAD\n"; return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // камера
    

    // шейдеры
    Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");

    // террейн
    Terrain terrain(128, 64.0f);
    terrain.generate(50.0f, 0.04f, 4, 0.0f);

    // текстуры
    auto loadTex = [&](const char* path) {
        GLuint tex; glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        int w, h, c;
        unsigned char* data = stbi_load(path, &w, &h, &c, 0);
        if (!data) {
            std::cerr << "Failed to load texture at: " << path << std::endl;
        }
        if (data) {
            GLenum fmt = (c == 4 ? GL_RGBA : GL_RGB);
            glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else std::cerr << "Failed tex " << path << "\n";
        stbi_image_free(data);
        return tex;
        };
    GLuint grassTex = loadTex("textures/Grass004.jpg");
    GLuint rockTex = loadTex("textures/Rock011.jpg");
    GLuint snowTex = loadTex("textures/Snow004.jpg");

    // shadow map setup omitted for brevity...
    // lightSpaceMatrix, FBO и depthTexture надо создать здесь

    float sunElevationDeg = 15.0f;               // угол возвышени€ над горизонтом
    float sunAzimuthDeg = 0.0f;               // направление по горизонтали (по желанию)
    float ambientIntensity = 0.8f;
    float diffuseIntensity = 2.0f;
    float specularIntensity = 1.5f;

    float el = glm::radians(sunElevationDeg);
    float az = glm::radians(sunAzimuthDeg);
    glm::vec3 L = glm::normalize(glm::vec3(
        cos(el) * cos(az),
        sin(el),
        cos(el) * sin(az)
    ));
    glm::vec3 sunDir = -L;

    // цикл
    while (!glfwWindowShouldClose(window)) {
        float current = (float)glfwGetTime();
        static float lastTime = current;
        float deltaTime = current - lastTime;
        lastTime = current;

        // ввод
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            static float amp = 50, freq = 0.04f, ofs = 0; 
            static int oct = 4;
            static float sunAzimuth = 0.0f;   // в градусах 0Ц360
            static float sunElevation = 15.0f;  // угол возвышени€ 0Ц90
            static float ambientInt = ambientIntensity;
            static float diffuseInt = diffuseIntensity;
            static float specularInt = specularIntensity;

            ImGui::Begin("Terrain");

            if (ImGui::SliderFloat("Amplitude", &amp, 0, 100)) terrain.generate(amp, freq, oct, ofs);
            if (ImGui::SliderFloat("Frequency", &freq, 0, 0.1f)) terrain.generate(amp, freq, oct, ofs);
            if (ImGui::SliderInt("Octaves", &oct, 1, 8))     terrain.generate(amp, freq, oct, ofs);
            if (ImGui::SliderFloat("Offset", &ofs, -1000, 1000)) terrain.generate(amp, freq, oct, ofs);
            if (ImGui::SliderFloat("Sun Azimuth", &sunAzimuth, 0.0f, 360.0f)); 
            if (ImGui::SliderFloat("Sun Elevation", &sunElevation, 0.0f, 90.0f));
            if (ImGui::SliderFloat("Ambient", &ambientInt, 0.0f, 1.0f));
            if (ImGui::SliderFloat("Diffuse", &diffuseInt, 0.0f, 5.0f));
            if (ImGui::SliderFloat("Specular", &specularInt, 0.0f, 5.0f));

            ImGui::End();
            {
                // сохран€ем новые значени€
                ambientIntensity = ambientInt;
                diffuseIntensity = diffuseInt;
                specularIntensity = specularInt;

                // пересчЄт направлени€ солнца
                float el = glm::radians(sunElevation);
                float az = glm::radians(sunAzimuth);
                glm::vec3 L = glm::normalize(glm::vec3(
                    cos(el) * cos(az),
                    sin(el),
                    cos(el) * sin(az)
                ));
                sunDir = -L;
            }
        }


        // рендер
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrainShader.use();
        // задаЄм uniform'ы: model, view, proj, lightSpaceMatrix, sun, viewPos и текстурные блоки
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom),
            float(SCR_W) / SCR_H,
            0.1f, 500.0f);
        terrainShader.setMat4("model", model);
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("projection", proj);
        // shadow map и параметры солнца нужно тоже сюда

        // позици€ камеры в шейдер
        terrainShader.setVec3("viewPos", camera.Position);

        // параметры направленного света (—олнце)
        terrainShader.setVec3("sun.direction", sunDir);
        terrainShader.setVec3("sun.ambient", glm::vec3(ambientIntensity));
        terrainShader.setVec3("sun.diffuse", glm::vec3(diffuseIntensity));
        terrainShader.setVec3("sun.specular", glm::vec3(specularIntensity));

        // текстуры
        glActiveTexture(GL_TEXTURE0);  glBindTexture(GL_TEXTURE_2D, grassTex);
        glActiveTexture(GL_TEXTURE1);  glBindTexture(GL_TEXTURE_2D, rockTex);
        glActiveTexture(GL_TEXTURE2);  glBindTexture(GL_TEXTURE_2D, snowTex);
        terrainShader.setInt("grassTex", 0);
        terrainShader.setInt("rockTex", 1);
        terrainShader.setInt("snowTex", 2);
        // shadow map в слот 3
        // glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, depthTex);
        // terrainShader.setInt("shadowMap",3);

        terrain.draw(terrainShader);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // очистка
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
