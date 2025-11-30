#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory> 
#include <vector>
#include <btBulletDynamicsCommon.h> // Include Bullet

#include "Model.h"
#include "Shader.h"

class Entity {
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    std::shared_ptr<Model> model;
    
    // Physics Body (Optional, can be nullptr)
    btRigidBody* rigidBody; 

    Entity* parent;
    std::vector<std::unique_ptr<Entity>> children;

    Entity(std::shared_ptr<Model> model = nullptr, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 rot = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

    void AddChild(std::unique_ptr<Entity> child);
    
    // NEW: Sync physics to graphics
    void UpdatePhysicsTransform();

    void Draw(Shader* shader, glm::mat4 parentMatrix = glm::mat4(1.0f));
};