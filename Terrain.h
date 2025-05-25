#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Shader.h"

class Terrain {
public:
    Terrain(int gridSize, float worldSize);
    ~Terrain();
    void generate(float amplitude, float frequency, int octaves, float offset);
    void draw(const Shader& shader) const;

private:
    int   GRID_SIZE;
    float WORLD_SIZE;
    GLuint VAO, VBO, EBO;
    size_t indexCount;
    std::vector<float> vertices;      // x,y,z, nx,ny,nz, u,v
    std::vector<unsigned int> indices;

    void setupMesh();
    void computeNormals();
};
