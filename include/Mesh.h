#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type; // e.g. "texture_diffuse"
    std::string path; // To prevent loading the same texture twice
};

class Mesh {
public:
    // Data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures; // New!
    bool hasTexture; // <--- Add this to check if texture exists
    glm::vec3 baseColor; // <--- Add this to store the fallback color

    // Constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    // Render
    void Draw(unsigned int shaderProgram);

private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};