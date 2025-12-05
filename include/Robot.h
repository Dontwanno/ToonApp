#pragma once
#include "Model.h"
#include "Shader.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

enum JointType {
    REVOLUTE,
    PRISMATIC,
    FIXED
};

// A single visual element (mesh) attached to a link
struct RobotVisual {
    std::shared_ptr<Model> model;
    glm::vec3 offset = glm::vec3(0.0f);
    glm::vec3 rpy = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(0.8f);
};

struct RobotLink {
    std::string name;
    
    // Support multiple visuals per link (e.g., grey body + orange bands)
    std::vector<RobotVisual> visuals;

    JointType jointType;
    // Tree Hierarchy
    RobotLink* parent = nullptr;
    std::vector<RobotLink*> children;

    // Kinematics (Bone Info)
    glm::vec3 jointOrigin = glm::vec3(0.0f); // Offset from parent joint
    glm::vec3 jointRPY = glm::vec3(0.0f);    // Rotation of the joint frame
    glm::vec3 jointAxis = glm::vec3(0.0f);   // Axis to rotate around
    float currentAngle = 0.0f;               // User controlled angle
};

class Robot {
public:
    std::vector<std::shared_ptr<RobotLink>> allLinks;
    RobotLink* rootLink = nullptr;
    glm::vec3 basePosition;

    Robot(glm::vec3 pos) : basePosition(pos) {}
    ~Robot() { allLinks.clear(); }

    void Draw(Shader* shader) {
        if (!rootLink) return;

        // 1. GLOBAL ROTATION
        glm::mat4 rootTransform = glm::mat4(1.0f);
        rootTransform = glm::translate(rootTransform, basePosition);
        rootTransform = glm::rotate(rootTransform, glm::radians(-90.0f), glm::vec3(1, 0, 0));

        DrawRecursive(shader, rootLink, rootTransform);
    }

private:
    void DrawRecursive(Shader* shader, RobotLink* link, glm::mat4 parentTransform) {
        // --- 1. KINEMATICS (Bones) ---
        glm::mat4 jointTransform = glm::translate(parentTransform, link->jointOrigin);
        
        // Apply Joint Rotation (Z-Y-X extrinsic)
        if(link->jointRPY.z != 0) jointTransform = glm::rotate(jointTransform, link->jointRPY.z, glm::vec3(0,0,1));
        if(link->jointRPY.y != 0) jointTransform = glm::rotate(jointTransform, link->jointRPY.y, glm::vec3(0,1,0));
        if(link->jointRPY.x != 0) jointTransform = glm::rotate(jointTransform, link->jointRPY.x, glm::vec3(1,0,0));

        // Rotate the joint (Animation)
        if (glm::length(link->jointAxis) > 0.01f) {
            jointTransform = glm::rotate(jointTransform, glm::radians(link->currentAngle), link->jointAxis);
        }

        // --- 2. VISUALS (Iterate all attached meshes) ---
        for (const auto& vis : link->visuals) {
            if (!vis.model) continue;

            glm::mat4 visualTransform = glm::translate(jointTransform, vis.offset);
            
            // Apply Visual Rotation
            if(vis.rpy.z != 0) visualTransform = glm::rotate(visualTransform, vis.rpy.z, glm::vec3(0,0,1));
            if(vis.rpy.y != 0) visualTransform = glm::rotate(visualTransform, vis.rpy.y, glm::vec3(0,1,0));
            if(vis.rpy.x != 0) visualTransform = glm::rotate(visualTransform, vis.rpy.x, glm::vec3(1,0,0));

            // --- 3. MESH CORRECTION ---
            glm::mat4 meshCorrection = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0));
            glm::mat4 finalModelMatrix = visualTransform * meshCorrection;

            shader->setMat4("model", finalModelMatrix);
            shader->setVec3("objectColor", vis.color); 
            vis.model->Draw(shader->ID);
        }

        // --- 4. RECURSE ---
        for (RobotLink* child : link->children) {
            DrawRecursive(shader, child, jointTransform);
        }
    }
};