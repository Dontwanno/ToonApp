#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include "Mesh.h"

class Model {
public:
    std::vector<Texture> textures_loaded; // Cache to avoid duplicate loading
    std::vector<Mesh> meshes;
    std::string directory;

    Model(const std::string& path);
    void Draw(unsigned int shaderProgram);

private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    
    // Loads textures from material
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, const aiScene* scene);
};