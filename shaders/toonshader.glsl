#shader vertex
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; 
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

#shader fragment
#version 410 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D texture_diffuse1; 

void main() {
    // 1. Texture & Alpha Test
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // --- FIX: DISCARD TRANSPARENT PIXELS ---
    // This removes the black boxes around the leaves
    if(texColor.a < 0.1)
        discard;

    // 2. Ambient
    float ambientStrength = 0.5; // Bumped up slightly for better visibility
    vec3 ambient = ambientStrength * vec3(1.0);

    // 3. Diffuse (Toon Shading)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = dot(norm, lightDir);
    float intensity;
    
    // Toon "Snap"
    if (diff > 0.95)      intensity = 1.0;
    else if (diff > 0.5)  intensity = 0.7;
    else if (diff > 0.25) intensity = 0.4;
    else                  intensity = 0.2;

    vec3 diffuse = intensity * lightColor;

    // Combine
    vec3 result = (ambient + diffuse) * texColor.rgb;
    
    FragColor = vec4(result, 1.0);
}