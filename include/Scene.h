#pragma once

#include <vector>
#include <memory> 
#include "Entity.h"
#include "Shader.h"
#include "PhysicsWorld.h" // <--- NEW

class Scene {
public:
    Scene();
    ~Scene();

    // Physics System
    std::unique_ptr<PhysicsWorld> physics; // <--- NEW

    void AddEntity(std::unique_ptr<Entity> entity);
    void Update(float deltaTime); // <--- NEW: Step physics
    void Draw(Shader* shader);
    void Clear();

private:
    std::vector<std::unique_ptr<Entity>> entities;
};