#pragma once

#include <glad/glad.h>
#include "Shader.h"

class FrameBuffer {
public:
    // Dimensions
    int width;
    int height;

    // Constructor
    FrameBuffer(int scrWidth, int scrHeight);
    ~FrameBuffer();

    // Re-generates textures when window resizes
    void Resize(int newWidth, int newHeight);

    // Call this before drawing your 3D scene
    void Bind();

    // Call this after drawing 3D scene to render the final image
    void DrawToScreen(Shader& postProcessShader);

private:
    unsigned int fbo;       // Framebuffer Object
    unsigned int texID;     // Color Texture
    unsigned int depthTexID; // <--- CHANGED: Depth Texture (was RBO)
    
    unsigned int rectVAO, rectVBO; // Screen Quad Geometry

    void setupFramebuffer();
    void setupScreenQuad();
};