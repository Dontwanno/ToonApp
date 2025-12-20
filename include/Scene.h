#pragma once

#include <vector>
#include <memory> 
#include "Shader.h"

class Scene {
public:
    Scene();
    ~Scene();

    void Update(float deltaTime); // <--- NEW: Step physics
    void Draw(Shader* shader);
    void Clear();

private:
};