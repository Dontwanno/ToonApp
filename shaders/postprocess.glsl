#shader vertex
#version 410 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}

#shader fragment
#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture; // New input

// Depth linearization vars
const float near_plane = 0.1; 
const float far_plane = 100.0; 

// Converts non-linear depth (0.0 - 1.0) to linear distance
float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;

    // --- 1. QUANTIZATION ---
    // Keep this to make the colors look "flat"
    float levels = 8.0;
    color = floor(color * levels) / levels;

    // --- 2. DEPTH EDGE DETECTION ---
    float offset = 1.0 / 400.0; // Thickness of line

    // Sobel Kernel
    float kernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );
    
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), vec2( 0.0f,    offset), vec2( offset,  offset),
        vec2(-offset,  0.0f),   vec2( 0.0f,    0.0f),   vec2( offset,  0.0f),
        vec2(-offset, -offset), vec2( 0.0f,   -offset), vec2( offset, -offset)
    );

    float depthSamples[9];
    for(int i = 0; i < 9; i++) {
        // We linearize the depth so the outline thickness is consistent
        float d = texture(depthTexture, TexCoords + offsets[i]).r;
        depthSamples[i] = LinearizeDepth(d);
    }

    float edge = 0.0;
    for(int i = 0; i < 9; i++)
        edge += depthSamples[i] * kernel[i];
    
    // Threshold: Only draw lines if depth difference is large enough
    // This effectively ignores small bumps but outlines objects
    if (abs(edge) > 0.5) 
        color = vec3(0.0); // Black Outline

    FragColor = vec4(color, 1.0);
}