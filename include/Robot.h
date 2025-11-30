#pragma once
#include "Model.h"
#include "Shader.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

struct RobotLink {
    std::string name;
    std::shared_ptr<Model> model;
    
    // Tree Hierarchy
    RobotLink* parent = nullptr;
    std::vector<RobotLink*> children;

    // Kinematics (Bone Info)
    glm::vec3 jointOrigin = glm::vec3(0.0f); // Offset from parent joint
    glm::vec3 jointAxis = glm::vec3(0.0f);   // Axis to rotate around
    float currentAngle = 0.0f;               // User controlled angle

    // Visuals (Mesh Info) -- NEW!
    glm::vec3 visualOffset = glm::vec3(0.0f); 
    glm::vec3 visualRPY = glm::vec3(0.0f);    // Roll-Pitch-Yaw of the mesh
    
    glm::vec3 color = glm::vec3(0.8f); 
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

        // --- COORDINATE SYSTEM FIX ---
        // 1. Start Identity
        glm::mat4 rootTransform = glm::mat4(1.0f);
        
        // 2. Translate to Base Position
        rootTransform = glm::translate(rootTransform, basePosition);

        // 3. GLOBAL ROTATION FIX (The 90 Degree Issue)
        // ROS is Z-Up. OpenGL is Y-Up.
        // Try this line. If it lays on the floor, change -90.0f to 0.0f or 90.0f
        rootTransform = glm::rotate(rootTransform, glm::radians(0.0f), glm::vec3(1, 0, 0));

        DrawRecursive(shader, rootLink, rootTransform);
    }

private:
    void DrawRecursive(Shader* shader, RobotLink* link, glm::mat4 parentTransform) {
        // --- 1. KINEMATICS (Bones) ---
        // Move to the joint's location relative to parent
        glm::mat4 jointTransform = glm::translate(parentTransform, link->jointOrigin);
        
        // Rotate the joint (if it has an axis)
        if (glm::length(link->jointAxis) > 0.01f) {
            jointTransform = glm::rotate(jointTransform, glm::radians(link->currentAngle), link->jointAxis);
        }

        // --- 2. VISUALS (Skin) ---
        // The mesh might not be at the exact bone location.
        // It has its own offset and rotation defined in the <visual> tag.
        
        glm::mat4 visualTransform = glm::translate(jointTransform, link->visualOffset);
        
        // Apply Visual Rotation (RPY from URDF)
        // URDF RPY is usually extrinsic XYZ. In GLM we apply in reverse order: Z, Y, X
        if(link->visualRPY.z != 0) visualTransform = glm::rotate(visualTransform, link->visualRPY.z, glm::vec3(0,0,1)); // Yaw
        if(link->visualRPY.y != 0) visualTransform = glm::rotate(visualTransform, link->visualRPY.y, glm::vec3(0,1,0)); // Pitch
        if(link->visualRPY.x != 0) visualTransform = glm::rotate(visualTransform, link->visualRPY.x, glm::vec3(1,0,0)); // Roll

        // --- 3. DRAW ---
        if (link->model) {
            shader->setMat4("model", visualTransform);
            shader->setVec3("objectColor", link->color); 
            link->model->Draw(shader->ID);
        }

        // --- 4. RECURSE ---
        // Pass the JOINT transform (the bone) to the children, NOT the visual transform
        for (RobotLink* child : link->children) {
            DrawRecursive(shader, child, jointTransform);
        }
    }
};