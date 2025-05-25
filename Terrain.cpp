#include "Terrain.h"
#include <glm/gtc/noise.hpp>
#include <iostream>

Terrain::Terrain(int gridSize, float worldSize)
    : GRID_SIZE(gridSize), WORLD_SIZE(worldSize) {
    // только создаём VAO/VBO/EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
}

Terrain::~Terrain() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Terrain::generate(float amplitude, float frequency, int octaves, float offset) {
    // 1) Создаём позиции и UV, применяем Perlin noise по y
    int N = GRID_SIZE;
    vertices.clear();
    vertices.reserve(N * N * 8);
    for (int z = 0; z < N; ++z) {
        for (int x = 0; x < N; ++x) {
            float xn = float(x) / (N - 1);
            float zn = float(z) / (N - 1);
            float xPos = (xn - 0.5f) * WORLD_SIZE;
            float zPos = (zn - 0.5f) * WORLD_SIZE;
            // шум
            float n = 0, f = frequency, a = 1, maxA = 0;
            for (int o = 0; o < octaves; ++o) {
                n += glm::perlin(glm::vec2(xPos * f + offset, zPos * f + offset)) * a;
                maxA += a; f *= 2; a *= 0.5f;
            }
            n = (n / maxA) * 0.5f + 0.5f;
            float yPos = n * amplitude;
            // запись
            vertices.insert(vertices.end(), {
                xPos, yPos, zPos,
                0,0,0,            // нормаль заполним позже
                xn * 10.0f, zn * 10.0f // UV с тайлингом =10
                });
        }
    }
    // 2) Индексы
    indices.clear();
    for (int z = 0; z < N - 1; ++z) {
        for (int x = 0; x < N - 1; ++x) {
            unsigned int i = z * N + x;
            indices.insert(indices.end(), {
                i, i + 1, i + N,
                i + 1, i + N + 1, i + N
                });
        }
    }
    indexCount = indices.size();
    // 3) Нормали
    computeNormals();
    // 4) Загрузка в OpenGL
    setupMesh();
}

void Terrain::computeNormals() {
    int N = GRID_SIZE;
    // нули для нормалей
    std::vector<glm::vec3> norms(N * N, glm::vec3(0.0f));
    // вспомогательный доступ
    auto pos = [&](int i) {
        return glm::vec3(
            vertices[i * 8 + 0],
            vertices[i * 8 + 1],
            vertices[i * 8 + 2]
        );
        };
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned a = indices[i + 0], b = indices[i + 1], c = indices[i + 2];
        glm::vec3 v0 = pos(a), v1 = pos(b), v2 = pos(c);
        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        norms[a] += normal;
        norms[b] += normal;
        norms[c] += normal;
    }
    for (int i = 0; i < N * N; ++i) {
        glm::vec3 n = glm::normalize(norms[i]);
        vertices[i * 8 + 3] = n.x;
        vertices[i * 8 + 4] = n.y;
        vertices[i * 8 + 5] = n.z;
    }
}

void Terrain::setupMesh() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
        vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        indices.data(), GL_STATIC_DRAW);
    GLsizei stride = 8 * sizeof(float);
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    // uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void Terrain::draw(const Shader& shader) const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
