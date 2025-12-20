#include "Mesh.h"
#include <string>
//include random number generator
#include <random>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->hasTexture = !textures.empty();
    this->baseColor = glm::vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX); // random color
    setupMesh();
}

void Mesh::Draw(unsigned int shaderProgram) {
    // 1. Bind Textures
    unsigned int diffuseNr  = 1;
    unsigned int specularNr = 1;

    for(unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit
        
        // retrieve texture number (the N in texture_diffuseN)
        std::string number;
        std::string name = textures[i].type;
        
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++); 

        // Set the sampler to the correct texture unit
        glUniform1i(glGetUniformLocation(shaderProgram, (name + number).c_str()), i);
        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    

    // Set the shader uniforms
    glUniform1i(glGetUniformLocation(shaderProgram, "hasTexture"), hasTexture);
    glUniform3fv(glGetUniformLocation(shaderProgram, "baseColor"), 1, &baseColor[0]);

    // 2. Draw Mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Vertex Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}