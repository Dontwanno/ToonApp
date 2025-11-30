#include "ToonApp.h"
#include "FileSystem.h"
#include <iostream>

ToonApp::ToonApp(int width, int height, const char* title) 
    : scrWidth(width), scrHeight(height), firstMouse(true), mouseCaptured(true),
      lightPos(2.0f, 8.0f, 5.0f), lightColor(1.0f, 1.0f, 1.0f), bgColor(1.0f, 1.0f, 1.0f)
{
    // 1. Initialize Window & OpenGL
    InitGLFW();

    // 2. Create Systems & Assets
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 2.0f, 10.0f));
    lastX = width / 2.0f;
    lastY = height / 2.0f;
    
    regularShader = std::make_shared<Shader>(FileSystem::getPath("shaders/regularshader.glsl"));
    postProcessShader = std::make_shared<Shader>(FileSystem::getPath("shaders/passthrough.glsl"));
    activeScene = std::make_unique<Scene>();
    kukaRobot = std::make_unique<KukaRobot>(glm::vec3(0, 0, 0));
    
    // Create scene with plane
    activeScene->physics->CreateStaticPlane();

    gameBuffer = std::make_unique<FrameBuffer>(scrWidth, scrHeight);

    // 3. Initialize UI
    InitImGui();

    // 4. REGISTER CALLBACKS LAST
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
}

ToonApp::~ToonApp() {
    // Smart pointers handle their own cleanup. 
    if (window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

void ToonApp::InitGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(scrWidth, scrHeight, "Toon Engine", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    
    // Handle High-DPI (Retina) immediately
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    scrWidth = width;
    scrHeight = height;
    
    // Load GLAD before calling glViewport
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }
    
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    glfwSetWindowUserPointer(window, this);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
}

void ToonApp::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, false); 
    ImGui_ImplOpenGL3_Init("#version 410");
}

void ToonApp::Run() {
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ProcessInput();
        Update();
        
        // Safety check before render loop
        if (gameBuffer) {
            gameBuffer->Bind();
            RenderScene();
            gameBuffer->DrawToScreen(*postProcessShader);
        }
        
        RenderUI();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void ToonApp::ProcessInput() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (mouseCaptured && camera) { // Check if camera exists
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera->ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera->ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera->ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera->ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera->ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera->ProcessKeyboard(DOWN, deltaTime);
    }
}

void ToonApp::Update() {
    activeScene->Update(deltaTime); // <--- Add this!
}

void ToonApp::RenderScene() {
    if (!regularShader || !camera || !kukaRobot) return; // Safety check

    glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    regularShader->use();
    
    glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)scrWidth / (float)scrHeight, 0.1f, 100.0f);
    glm::mat4 view = camera->GetViewMatrix();
    
    regularShader->setMat4("projection", projection);
    regularShader->setMat4("view", view);
    regularShader->setVec3("lightPos", lightPos);
    regularShader->setVec3("viewPos", camera->Position);
    regularShader->setVec3("lightColor", lightColor);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); 
    // modelMatrix = glm::rotate(modelMatrix, (float)glfwGetTime() * glm::radians(modelRotSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f)); 
    regularShader->setMat4("model", modelMatrix);
    kukaRobot->Draw(regularShader.get());
    activeScene->Draw(regularShader.get());

}

void ToonApp::RenderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Engine Controls");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::DragFloat3("Light Pos", &lightPos.x, 0.1f);
    ImGui::ColorEdit3("Light Color", &lightColor.x);
    ImGui::ColorEdit3("Background", &bgColor.x);
    // ImGui::SliderFloat("Scale", &modelScale, 0.001f, 1.0f);
    // ImGui::SliderFloat("Rotation", &modelRotSpeed, 0.0f, 100.0f);
    ImGui::Text(mouseCaptured ? "GAME MODE (ALT to unlock)" : "UI MODE (ALT to capture)");
    ImGui::End();

        ImGui::Begin("Robot Control");
    // We start at 1 because Link 0 is the static base
    ImGui::SliderFloat("Joint 1", &kukaRobot->links[1].currentAngle, -170.0f, 170.0f);
    ImGui::SliderFloat("Joint 2", &kukaRobot->links[2].currentAngle, -120.0f, 120.0f);
    ImGui::SliderFloat("Joint 3", &kukaRobot->links[3].currentAngle, -170.0f, 170.0f);
    ImGui::SliderFloat("Joint 4", &kukaRobot->links[4].currentAngle, -120.0f, 120.0f);
    ImGui::SliderFloat("Joint 5", &kukaRobot->links[5].currentAngle, -170.0f, 170.0f);
    ImGui::SliderFloat("Joint 6", &kukaRobot->links[6].currentAngle, -120.0f, 120.0f);
    ImGui::SliderFloat("Joint 7", &kukaRobot->links[7].currentAngle, -175.0f, 175.0f);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// --- STATIC CALLBACKS ---

void ToonApp::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    ToonApp* app = (ToonApp*)glfwGetWindowUserPointer(window);
    if (!app) return; 

    app->scrWidth = width;
    app->scrHeight = height;
    glViewport(0, 0, width, height);
    
    if(app->gameBuffer) {
        app->gameBuffer->Resize(width, height);
    }
}

void ToonApp::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    ToonApp* app = (ToonApp*)glfwGetWindowUserPointer(window);
    if (!app) return;

    ImGui_ImplGlfw_CursorPosCallback(window, xposIn, yposIn);
    if (!app->mouseCaptured) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (app->firstMouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    float xoffset = xpos - app->lastX;
    float yoffset = app->lastY - ypos; 
    app->lastX = xpos;
    app->lastY = ypos;

    if (app->camera) {
        app->camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

void ToonApp::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ToonApp* app = (ToonApp*)glfwGetWindowUserPointer(window);
    if (!app) return;

    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    
    if (app->mouseCaptured && !ImGui::GetIO().WantCaptureMouse && app->camera)
        app->camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

void ToonApp::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ToonApp* app = (ToonApp*)glfwGetWindowUserPointer(window);
    if (!app) return;

    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
        app->mouseCaptured = !app->mouseCaptured;
        if (app->mouseCaptured) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void ToonApp::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ToonApp* app = (ToonApp*)glfwGetWindowUserPointer(window);
    if (!app) return; 

    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}