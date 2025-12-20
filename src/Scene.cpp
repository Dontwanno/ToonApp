#include "Scene.h"
#include <vector>

#include <algorithm> // Required for std::min

#include <algorithm> // for std::min

unsigned int createCheckeredTexture() {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // FIX 1: Increase resolution from 64 -> 1024
    // This keeps the texture sharp much further into the distance
    const int width = 1024;
    const int height = 1024;
    std::vector<unsigned char> data(width * height * 3);

    // Adjust grid size since we increased resolution
    int gridSize = 128; // Larger grid squares in pixels

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Simple math to create the checker pattern
            bool isDark = ((x / gridSize) + (y / gridSize)) % 2 == 0;
            unsigned char color = isDark ? 200 : 100;
            
            int index = (y * width + x) * 3;
            data[index] = color;
            data[index + 1] = color;
            data[index + 2] = color;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // FIX 2: Set Anisotropic Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Force Anisotropy (Mac/OpenGL 4.1 compatible)
    #ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
        #define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
        #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
    #endif

    GLfloat maxAnisotropy = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(16.0f, maxAnisotropy));

    return textureID;
}

// Plane geometry
unsigned int planeVAO = 0, planeVBO;
unsigned int groundTexture = 0;

void setupPlane() {
    float planeVertices[] = {
        // positions            // normals         // texcoords
         // CHANGE Y FROM -0.5f TO 0.0f
         25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, 0.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };

    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    groundTexture = createCheckeredTexture();
}

Scene::Scene() {
    setupPlane(); // Initialize the ground plane
}

Scene::~Scene() {
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteTextures(1, &groundTexture);
}

void Scene::Update(float deltaTime) {

}

void Scene::Draw(Shader* shader) {


    // Draw Ground Plane
    if (planeVAO != 0) {
        glm::mat4 model = glm::mat4(1.0f);
        shader->setMat4("model", model);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundTexture);
        shader->setBool("hasTexture", true);
        shader->setVec3("baseColor", glm::vec3(0.4f, 0.4f, 0.4f));
        shader->setInt("texture_diffuse1", 0); // Assuming texture unit 0

        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
}

void Scene::Clear() {
}