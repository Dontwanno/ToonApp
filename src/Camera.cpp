#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    
    // 1. Calculate a robust "Flat Forward" vector
    // This removes the Y-component so you don't fly into the ground/sky
    glm::vec3 frontFlat = glm::vec3(Front.x, 0.0f, Front.z);
    
    // Safety check: if looking straight up/down, x/z are near 0.
    if (glm::length(frontFlat) > 0.001f)
        frontFlat = glm::normalize(frontFlat);
    else
        // Fallback: if looking straight up, assume forward is -Z
        frontFlat = glm::vec3(0.0f, 0.0f, -1.0f); 

    // 2. Calculate "Flat Right" from "Flat Forward"
    // This ensures strafing (A/D) is perfectly perpendicular to walking (W/S)
    glm::vec3 rightFlat = glm::normalize(glm::cross(frontFlat, WorldUp));

    if (direction == FORWARD)
        Position += frontFlat * velocity;
    if (direction == BACKWARD)
        Position -= frontFlat * velocity;
    if (direction == LEFT)
        Position -= rightFlat * velocity;
    if (direction == RIGHT)
        Position += rightFlat * velocity;
        
    if (direction == UP)
        Position += WorldUp * velocity;
    if (direction == DOWN)
        Position -= WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    
    Right = glm::normalize(glm::cross(Front, WorldUp));  
    Up    = glm::normalize(glm::cross(Right, Front));
}