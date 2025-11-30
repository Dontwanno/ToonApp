#include "Entity.h"
#include <glm/gtc/type_ptr.hpp>

Entity::Entity(std::shared_ptr<Model> m, glm::vec3 pos, glm::vec3 rot, glm::vec3 scl) 
    : model(m), position(pos), rotation(rot), scale(scl), parent(nullptr), rigidBody(nullptr)
{
}

void Entity::AddChild(std::unique_ptr<Entity> child) {
    child->parent = this;
    children.push_back(std::move(child));
}

void Entity::UpdatePhysicsTransform() {
    if (rigidBody && rigidBody->getMotionState()) {
        btTransform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        
        btVector3 origin = trans.getOrigin();
        btQuaternion rot = trans.getRotation(); // Bullet uses Quaternions

        // Sync Position
        position = glm::vec3(origin.getX(), origin.getY(), origin.getZ());
        
        // Sync Rotation (Convert Quaternion to Euler is tricky, usually better to use matrix directly)
        // For simplicity, we just won't update euler angles here but rely on constructing the matrix from quat if needed.
        // But for your current Draw(), let's just stick to position sync for now.
        // Proper rotation sync requires changing your Draw() to use Quaternions or Matrix decomp.
        
        // Simple Euler update (Approximate)
        btScalar yaw, pitch, roll;
        trans.getBasis().getEulerYPR(yaw, pitch, roll);
        rotation = glm::degrees(glm::vec3(pitch, yaw, roll)); 
    }
}

void Entity::Draw(Shader* shader, glm::mat4 parentMatrix) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    
    if (rigidBody) {
        // If we have physics, use the physics transform directly! (More accurate)
        btTransform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        
        // Convert Bullet Matrix to GLM Matrix
        float matrix[16];
        trans.getOpenGLMatrix(matrix);
        modelMatrix = glm::make_mat4(matrix);
        
        // Apply Scale (Bullet doesn't store scale in transform usually)
        modelMatrix = glm::scale(modelMatrix, scale);
    } 
    else {
        // Manual Transform Logic
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, scale);
    }

    glm::mat4 globalMatrix = parentMatrix * modelMatrix;

    if (model) {
        shader->setMat4("model", globalMatrix);
        model->Draw(shader->ID);
    }

    for (auto& child : children) {
        child->Draw(shader, globalMatrix);
    }
}