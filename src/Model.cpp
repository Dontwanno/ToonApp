#include "Model.h"
#include <iostream>
#include <stb_image.h> 
#include "FileSystem.h"

// Forward declaration
unsigned int LoadTexture(const char *path, const std::string &directory, const aiScene* scene);

Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::Draw(unsigned int shaderProgram) {
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shaderProgram);
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    // Standard flags for game dev + GenSmoothNormals from your baseline
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // 1. Process Vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        // Position
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;

        // Normals
        if (mesh->HasNormals()) {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        } else {
            vertex.Normal = glm::vec3(0.0f);
        }

        // Texture Coordinates
        if(mesh->mTextureCoords[0]) {
            vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // 2. Process Indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // 3. Process Materials
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        // 1. Diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        
        // 2. Specular maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // 3. Normal maps (obj uses height, gltf uses normals)
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", scene);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        
        // 4. Base Color (PBR workflow used by GLTF/GLB - often used instead of diffuse)
        std::vector<Texture> baseColorMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_diffuse", scene);
        textures.insert(textures.end(), baseColorMaps.begin(), baseColorMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, const aiScene* scene) {
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++) {
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        
        if(!skip) {
            Texture texture;
            texture.id = LoadTexture(str.C_Str(), this->directory, scene);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

// --- Robust Texture Loader (Handles Files AND Embedded GLB) ---
unsigned int LoadTexture(const char *path, const std::string &directory, const aiScene* scene) {
    std::string filename = std::string(path);
    
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = nullptr;

    // --- CHECK 1: EMBEDDED TEXTURE (GLB) ---
    // If path starts with '*', it's an index into the scene's internal texture array
    if (filename[0] == '*') {
        // Do NOT flip embedded textures. They are usually aligned with the model's internal UVs already.
        stbi_set_flip_vertically_on_load(false); 

        int index = std::stoi(filename.substr(1));
        if (index < scene->mNumTextures) {
            const aiTexture* embeddedTexture = scene->mTextures[index];
            
            // Case A: Compressed (JPG/PNG) embedded data
            // mHeight is 0 when the texture is compressed
            if (embeddedTexture->mHeight == 0) {
                data = stbi_load_from_memory(
                    reinterpret_cast<unsigned char*>(embeddedTexture->pcData),
                    embeddedTexture->mWidth,
                    &width, &height, &nrComponents, 0
                );
            } 
            // Case B: Raw pixel data (ARGB8888)
            else {
                data = stbi_load_from_memory(
                    reinterpret_cast<unsigned char*>(embeddedTexture->pcData),
                    embeddedTexture->mWidth * embeddedTexture->mHeight, 
                    &width, &height, &nrComponents, 0
                );
            }
        }
    } 
    // --- CHECK 2: EXTERNAL FILE (OBJ/DAE) ---
    else {
        // ENABLE flipping for external files because we used aiProcess_FlipUVs
        stbi_set_flip_vertically_on_load(true);
        
        filename = directory + '/' + filename;
        data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    }

    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Wrapping: Repeat ensures we don't get ugly edges if UVs go out of bounds
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Filtering: Linear Mipmap Linear is the highest quality standard filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    // Reset global state to safe default
    stbi_set_flip_vertically_on_load(false);

    return textureID;
}