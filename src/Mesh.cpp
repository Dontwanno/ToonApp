#include "Mesh.h"
#include <string>
//include random number generator
#include <random>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    setupMesh();
}

void Mesh::Draw(unsigned int shaderProgram) {
    // 1. Bind Textures
    unsigned int diffuseNr  = 1;
    unsigned int specularNr = 1;

    if (textures.empty()) {
        // If no textures, bind a 1x1 White Texture to slot 0
        // We use a static variable so we only create this texture ONCE for the whole program
        static unsigned int randomTextureID = 0;
        if (randomTextureID == 0) {
            glGenTextures(1, &randomTextureID);
            glBindTexture(GL_TEXTURE_2D, randomTextureID);
            unsigned char random[] = {static_cast<unsigned char>(rand() % 255), static_cast<unsigned char>(rand() % 255), static_cast<unsigned char>(rand() % 255)};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, random);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, randomTextureID);
        
        // Tell the shader to use Texture Unit 0 for "texture_diffuse1"
        glUniform1i(glGetUniformLocation(shaderProgram, "texture_diffuse1"), 0);
    }

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