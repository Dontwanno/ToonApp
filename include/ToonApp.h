#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory> 

#include "Camera.h"
#include "FrameBuffer.h" // Ensure casing matches disk
#include "Shader.h"
#include "Model.h"
#include "Scene.h"
#include "Entity.h"
#include "Kuka.h"



class ToonApp {
public:
    ToonApp(int width, int height, const char* title);
    ~ToonApp();

    void Run();

private:
    GLFWwindow* window;
    int scrWidth;
    int scrHeight;
    
    // Safety Flag
    bool isInitialized; // <--- NEW

    // Engine Systems
    std::unique_ptr<Camera> camera;
    std::unique_ptr<FrameBuffer> gameBuffer;
    
    // Assets
    std::shared_ptr<Shader> regularShader;
    std::shared_ptr<Shader> postProcessShader;
    std::shared_ptr<Model> backpackModel; 
    std::unique_ptr<KukaRobot> kukaRobot;

    std::unique_ptr<Scene> activeScene;

    // State
    glm::vec3 lightPos;
    glm::vec3 lightColor;
    glm::vec3 bgColor;
    
    float debugScale;
    float debugRotSpeed;

    // Input
    float lastX, lastY;
    bool firstMouse;
    bool mouseCaptured;
    float deltaTime;
    float lastFrame;

    void InitGLFW();
    void InitImGui();
    void ProcessInput();
    void Update();
    void RenderScene();
    void RenderUI();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
};