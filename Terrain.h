#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

class Shader; // ����� ����������

class Terrain {
public:
    Terrain(int gridSize, float worldSize);
    ~Terrain();

    // ���������: ��������� ����, �������, ������, ��������
    void generate(float amplitude, float frequency, int octaves, float offset);
    void draw(const Shader& shader) const;

private:
    int   GRID_SIZE;
    float WORLD_SIZE;
    GLuint VAO, VBO, EBO;
    size_t indexCount;

    // x,y,z | nx,ny,nz | tx,ty | tan.x,y,z | bitan.x,y,z  => 14 float
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    void computeNormals();
    void computeTangents();
    void setupMesh();
};
