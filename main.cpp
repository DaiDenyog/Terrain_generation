// main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <vector>
#include <iostream>

// ————————————————————————————————————————————————————
//          ÏÅĞÅÌÅÍÍÛÅ ÊÀÌÅĞÛ (FPS-CAMERA)
// ————————————————————————————————————————————————————
glm::vec3 cameraPos = glm::vec3(0.0f, 50.0f, 100.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.5f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;   // íà÷èíàÿ ñìîòğåòü ïî -Z
float pitch = -20.0f;   // íåìíîãî âíèç
float lastX = 1280.0f / 2.0f, lastY = 720.0f / 2.0f;
bool  firstMouse = true;

float deltaTime = 0.0f, lastFrame = 0.0f;

bool mouseCaptured = true;

// êîëáıê äâèæåíèÿ ìûøè
void mouse_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    if (!mouseCaptured) return;   // åñëè ğåæèì UI — èãíîğèğóåì ïîâîğîòû êàìåğû

    if (firstMouse) {
        lastX = float(xpos);
        lastY = float(ypos);
        firstMouse = false;
    }
    float xoffset = float(xpos) - lastX;
    float yoffset = lastY - float(ypos);
    lastX = float(xpos);
    lastY = float(ypos);

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(dir);
}

// îáğàáîòêà êëàâèø
void processInput(GLFWwindow* window) {
    float speed = 50.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// ————————————————————————————————————————————————————
//               ÃËÎÁÀËÛ ÒÅĞĞÀÈÍÀ
// ————————————————————————————————————————————————————
const float WORLD_SIZE = 64.0f;
const int GRID_SIZE = 256;
static std::vector<float>         vertices;
static std::vector<unsigned int>  indices;

static float g_amplitude = 20.0f;
static float g_frequency = 0.08f;
static int   g_octaves = 4;
static float g_offset = 0.0f;

// ãåíåğàöèÿ âûñîò è öâåòîâ
void GenerateTerrain(int gridSize, float amplitude, float frequency, int octaves, float offset) {
    size_t totalVerts = size_t(gridSize) * gridSize;
    vertices.resize(totalVerts * 6);

    int idx = 0;
    for (int z = 0; z < gridSize; ++z) {
        for (int x = 0; x < gridSize; ++x) {
            
            float xN = float(x) / (gridSize - 1);  // [0..1]
            float zN = float(z) / (gridSize - 1);  // [0..1]
            float xPos = ((float)x / (gridSize - 1) - 0.5f) * WORLD_SIZE;
            float zPos = ((float)z / (gridSize - 1) - 0.5f) * WORLD_SIZE;


            float noiseValue = 0.0f, freq = frequency, ampl = 1.0f, maxAmpl = 0.0f;
            for (int o = 0; o < octaves; ++o) {
                float perlin = glm::perlin(glm::vec2(xPos * freq + offset, zPos * freq + offset));
                noiseValue += perlin * ampl;
                maxAmpl += ampl;
                freq *= 2.0f; ampl *= 0.5f;
            }
            noiseValue /= maxAmpl;
            float height = (noiseValue * 0.5f + 0.5f) * amplitude;

            vertices[idx++] = xPos;
            vertices[idx++] = height;
            vertices[idx++] = zPos;

            glm::vec3 color;
            float hnorm = height / amplitude;
            if (hnorm < 0.4f)      color = { 0,0,1 };
            else if (hnorm < 0.7f) color = { 0,1,0 };
            else                   color = { 1,1,1 };

            vertices[idx++] = color.r;
            vertices[idx++] = color.g;
            vertices[idx++] = color.b;
        }
    }
}

int main() {
    // Init GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Perlin Noise Terrain", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            // ËÊÌ — âêëş÷àåì ğåæèì êàìåğû (ñêğûâàåì êóğñîğ)
            mouseCaptured = true;
            glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // ñáğîñ, ÷òîáû íå áûëî ñêà÷êà ïğè ñëåäóşùåì äâèæåíèè
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            // ÏÊÌ — âêëş÷àåì ğåæèì UI (ïîêàçûâàåì êóğñîğ)
            mouseCaptured = false;
            glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        });

    // ĞÅÃÈÑÒĞÀÖÈß CALLBACK ÄËß ÌÛØÈ
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Øåéäåğû (vertex/fragment) — áåç èçìåíåíèé, êàê ó âàñ
    const char* vs = R"glsl(
      #version 330 core
      layout(location=0) in vec3 aPos;
      layout(location=1) in vec3 aColor;
      out vec3 vertexColor;
      uniform mat4 MVP;
      void main(){
        vertexColor = aColor;
        gl_Position = MVP * vec4(aPos,1.0);
      }
    )glsl";
    const char* fs = R"glsl(
      #version 330 core
      in vec3 vertexColor;
      out vec4 FragColor;
      void main(){
        FragColor = vec4(vertexColor,1.0);
      }
    )glsl";
    GLuint vsh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsh, 1, &vs, NULL); glCompileShader(vsh);
    GLuint fsh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsh, 1, &fs, NULL); glCompileShader(fsh);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vsh); glAttachShader(prog, fsh);
    glLinkProgram(prog);
    glDeleteShader(vsh); glDeleteShader(fsh);

    // èíäåêñû
    indices.reserve((GRID_SIZE - 1) * (GRID_SIZE - 1) * 6);
    for (int r = 0; r < GRID_SIZE - 1; ++r)
        for (int c = 0; c < GRID_SIZE - 1; ++c) {
            unsigned tL = r * GRID_SIZE + c;
            unsigned bL = (r + 1) * GRID_SIZE + c;
            unsigned bR = (r + 1) * GRID_SIZE + (c + 1);
            unsigned tR = r * GRID_SIZE + (c + 1);
            indices.insert(indices.end(), { tL,bL,bR, tL,bR,tR });
        }

    // ãåíåğèì òåğğåéí
    GenerateTerrain(GRID_SIZE, g_amplitude, g_frequency, g_octaves, g_offset);

    // VAO/VBO/EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // ğåíäåğ-öèêë
    while (!glfwWindowShouldClose(window)) {
        // âğåìÿ
        float current = (float)glfwGetTime();
        deltaTime = current - lastFrame;
        lastFrame = current;

        processInput(window);

        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui ïàíåëü
        ImGui::Begin("Terrain Controls");
        bool dirty = false;
        if (ImGui::SliderFloat("Amplitude", &g_amplitude, 0, 100)) dirty = true;
        if (ImGui::SliderFloat("Frequency", &g_frequency, 0, 1)) dirty = true;
        if (ImGui::SliderInt("Octaves", &g_octaves, 1, 8)) dirty = true;
        if (ImGui::SliderFloat("Offset", &g_offset, -1000, 1000)) dirty = true;
        ImGui::End();
        if (dirty) {
            GenerateTerrain(GRID_SIZE, g_amplitude, g_frequency, g_octaves, g_offset);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
        }

        // ğåíäåğ ñöåíû
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int w, h; glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        float aspect = float(w) / float(h);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 500.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 MVP = proj * view * model;

        glUseProgram(prog);
        GLint loc = glGetUniformLocation(prog, "MVP");
        glUniformMatrix4fv(loc, 1, GL_FALSE, &MVP[0][0]);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        // ImGui draw
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteProgram(prog);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
