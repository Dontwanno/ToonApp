#pragma once

#include <string>
#include <vector>
#include <map>
#include <tinyxml2.h>
#include <glm/glm.hpp>

class Robot; // Forward declaration

class URDFLoader {
public:
    // Helper function to load the URDF file into a Robot object
    // Returns true on success
    static bool Load(const std::string& path, Robot& robot);
};