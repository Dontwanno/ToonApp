#include "Scene.h"
#include <vector>

// Helper to create a checkered texture for the ground
unsigned int createCheckeredTexture() {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    const int width = 64;
    const int height = 64;
    std::vector<unsigned char> data(width * height * 3);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char color = ((x / 8 + y / 8) % 2 == 0) ? 200 : 100;
            int index = (y * width + x) * 3;
            data[index] = color;
            data[index + 1] = color;
            data[index + 2] = color;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

// Plane geometry
unsigned int planeVAO = 0, planeVBO;
unsigned int groundTexture = 0;

void setupPlane() {
    float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
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
    physics = std::make_unique<PhysicsWorld>();
    setupPlane(); // Initialize the ground plane
}

Scene::~Scene() {
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteTextures(1, &groundTexture);
}

void Scene::AddEntity(std::unique_ptr<Entity> entity) {
    entities.push_back(std::move(entity));
}

void Scene::Update(float deltaTime) {
    // 1. Step Physics
    physics->Step(deltaTime);

    // 2. Sync Graphics to Physics
    for (auto& entity : entities) {
        entity->UpdatePhysicsTransform();
    }
}

void Scene::Draw(Shader* shader) {
    // Draw Entities
    for (auto& entity : entities) {
        entity->Draw(shader, glm::mat4(1.0f));
    }

    // Draw Ground Plane
    if (planeVAO != 0) {
        glm::mat4 model = glm::mat4(1.0f);
        shader->setMat4("model", model);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundTexture);
        shader->setInt("texture_diffuse1", 0); // Assuming texture unit 0

        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
}

void Scene::Clear() {
    entities.clear();
    // Also need to clear physics bodies ideally, but PhysicsWorld destructor handles it
}