#pragma once
#include "Model.h"
#include "Shader.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct RobotLink {
    std::shared_ptr<Model> model;
    glm::vec3 offsetFromParent; // From Xacro <origin xyz="...">
    glm::vec3 rotationAxis;     // From Xacro <axis xyz="...">
    float currentAngle = 0.0f;  // The angle we want to rotate
};

class KukaRobot {
public:
    std::vector<RobotLink> links;
    glm::vec3 basePosition;

    KukaRobot(glm::vec3 pos) : basePosition(pos) {
        loadModels();
        setupKinematics();
    }

    void Draw(Shader* shader) {
        // 1. Define the correction to flip Z-Up models to Y-Up
        // Rotate -90 degrees around X axis
        glm::mat4 modelCorrection = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat4 globalTransform = glm::mat4(1.0f);
        globalTransform = glm::translate(globalTransform, basePosition);

        // Bind the White Texture (from previous step) so it isn't orange/backpack textured
        // (Assuming you have access to whiteTextureID, otherwise skip this line)
        // glBindTexture(GL_TEXTURE_2D, whiteTextureID); 

        for (auto& link : links) {
            // --- KINEMATICS (The Math) ---
            // 1. Move to the joint location (Translate)
            globalTransform = glm::translate(globalTransform, link.offsetFromParent);
            
            // 2. Rotate the joint (Rotate)
            globalTransform = glm::rotate(globalTransform, glm::radians(link.currentAngle), link.rotationAxis);

            // --- RENDERING (The Visuals) ---
            // We multiply the math (globalTransform) by the correction (flip upright)
            // BEFORE passing it to the shader.
            glm::mat4 finalModelMatrix = globalTransform * modelCorrection;

            shader->setMat4("model", finalModelMatrix);
            link.model->Draw(shader->ID);
        }
    }

private:
    void loadModels() {
        // Ensure you converted your STLs to OBJs in Blender first!
        // We push back empty links first, then fill data in setupKinematics
        for(int i=0; i<=7; i++) links.push_back(RobotLink());

        links[0].model = std::make_shared<Model>("assets/kuka/meshes/lbr_iiwa14_r820/visual/base_link.stl");
        links[1].model = std::make_shared<Model>("assets/kuka/meshes/lbr_iiwa14_r820/visual/link_1.stl");
        links[2].model = std::make_shared<Model>("assets/kuka/meshes/lbr_iiwa14_r820/visual/link_2.stl");
        links[3].model = std::make_shared<Model>("assets/kuka/meshes/lbr_iiwa14_r820/visual/link_3.stl");
        links[4].model = std::make_shared<Model>("assets/kuka/meshes/lbr_iiwa14_r820/visual/link_4.stl");
        links[5].model = std::make_shared<Model>("assets/kuka/meshes/lbr_iiwa14_r820/visual/link_5.stl");
        links[6].model = std::make_shared<Model>("assets/kuka/meshes/lbr_iiwa14_r820/visual/link_6.stl");
        links[7].model = std::make_shared<Model>("assets/kuka/meshes/lbr_iiwa14_r820/visual/link_7.stl");
    }

    void setupKinematics() {
        // -- BASE (Link 0) --
        links[0].offsetFromParent = glm::vec3(0.0f); 
        links[0].rotationAxis = glm::vec3(0, 1, 0); 
        links[0].currentAngle = 0.0f; // Base doesn't rotate

        // -- JOINT 1 --
        // From Xacro: <origin xyz="0 0 0.1575"/> <axis xyz="0 0 1"/>
        links[1].offsetFromParent = glm::vec3(0.0f, 0.1575f, 0.0f);
        links[1].rotationAxis = glm::vec3(0, 1, 0); // Y-Up in OpenGL matches Z-Up in ROS usually, check axes!
        // *NOTE*: ROS uses Z-up. OpenGL usually uses Y-up. 
        // If your engine is Y-up, you map ROS(Z) -> GL(Y), ROS(Y) -> GL(Z), ROS(X) -> GL(X).
        // The mapping below assumes you converted the models in Blender to Y-UP.

        // Assuming Blender Export was Y-Up:
        links[1].rotationAxis = glm::vec3(0, 1, 0); // Rotate around Y (vertical)

        // -- JOINT 2 --
        // Xacro: 0.2025, Axis Y
        links[2].offsetFromParent = glm::vec3(0.0f, 0.2025f, 0.0f);
        links[2].rotationAxis = glm::vec3(0, 0, 1); // Axis Z (Horizontal)

        // -- JOINT 3 --
        // Xacro: 0.2045, Axis Z
        links[3].offsetFromParent = glm::vec3(0.0f, 0.2045f, 0.0f);
        links[3].rotationAxis = glm::vec3(0, 1, 0); // Vertical

        // -- JOINT 4 --
        // Xacro: 0.2155, Axis -Y
        links[4].offsetFromParent = glm::vec3(0.0f, 0.2155f, 0.0f);
        links[4].rotationAxis = glm::vec3(0, 0, -1); // Horizontal

        // -- JOINT 5 --
        // Xacro: 0.1845, Axis Z
        links[5].offsetFromParent = glm::vec3(0.0f, 0.1845f, 0.0f);
        links[5].rotationAxis = glm::vec3(0, 1, 0); 

        // -- JOINT 6 --
        // Xacro: 0.2155, Axis Y
        links[6].offsetFromParent = glm::vec3(0.0f, 0.2155f, 0.0f);
        links[6].rotationAxis = glm::vec3(0, 0, 1);

        // -- JOINT 7 (Flange) --
        // Xacro: 0.081, Axis Z
        links[7].offsetFromParent = glm::vec3(0.0f, 0.081f, 0.0f);
        links[7].rotationAxis = glm::vec3(0, 1, 0);
    }
};