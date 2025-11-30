#include "URDFLoader.h"
#include "Robot.h"
#include <tinyxml2.h>
#include <iostream>
#include <map>
#include <algorithm>

#include "FileSystem.h"

using namespace tinyxml2;

// --- HELPERS ---

// Converts "0 0 1" string to glm::vec3
static glm::vec3 ParseVec3(const char* str) {
    glm::vec3 vec(0.0f);
    if (str) sscanf(str, "%f %f %f", &vec.x, &vec.y, &vec.z);
    return vec;
}

static std::string ResolvePath(std::string path) {
    // 1. Strip "package://"
    std::string prefix = "package://";
    size_t pos = path.find(prefix);
    if (pos != std::string::npos) {
        path.erase(pos, prefix.length());
    }

    // 2. Fix the folder name mismatch
    // The URDF says: "drake_models/iiwa_description/..."
    // You have:      "assets/kuka/..."
    
    std::string wrongPrefix = "drake_models/iiwa_description/";
    size_t wrongPos = path.find(wrongPrefix);
    
    if (wrongPos != std::string::npos) {
        // Replace the wrong prefix with your actual folder path
        path.replace(wrongPos, wrongPrefix.length(), "assets/kuka/");
    } else {
        // Fallback: If the path didn't match the expected wrong prefix, 
        // just prepend "assets/" and hope for the best.
        path = "assets/" + path;
    }

    // 3. Convert to Absolute Path (Fixes the "Unable to open" error)
    std::string fullPath = FileSystem::getPath(path);
    std::cout << "Resolved Path: " << fullPath << std::endl;
    return fullPath;
}

// --- MAIN LOADER ---

bool URDFLoader::Load(const std::string& path, Robot& robot) {
    XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != XML_SUCCESS) {
        std::cout << "URDF LOAD ERROR: Could not open " << path << std::endl;
        return false;
    }

    XMLElement* root = doc.FirstChildElement("robot");
    if (!root) return false;

    robot.allLinks.clear();
    robot.rootLink = nullptr;

    // Temporary map to look up links by name string
    std::map<std::string, RobotLink*> linkMap;

    // -----------------------------------------
    // PASS 1: PARSE ALL LINKS (Visuals & Names)
    // -----------------------------------------
    XMLElement* xmlLink = root->FirstChildElement("link");
    while (xmlLink) {
        auto newLink = std::make_shared<RobotLink>();
        newLink->name = xmlLink->Attribute("name");
        
        // -- Visual Geometry --
        XMLElement* visual = xmlLink->FirstChildElement("visual");
        if (visual) {
              // --- NEW: Parse Visual Origin ---
              XMLElement* visOrigin = visual->FirstChildElement("origin");
              if (visOrigin) {
                  if(visOrigin->Attribute("xyz")) 
                      newLink->visualOffset = ParseVec3(visOrigin->Attribute("xyz"));
                  if(visOrigin->Attribute("rpy")) 
                      newLink->visualRPY = ParseVec3(visOrigin->Attribute("rpy"));
              }
            // 1. Geometry (Mesh)
            XMLElement* geo = visual->FirstChildElement("geometry");
            if (geo) {
                XMLElement* mesh = geo->FirstChildElement("mesh");
                if (mesh) {
                    std::string meshPath = ResolvePath(mesh->Attribute("filename"));
                    // Assimp handles .stl, .dae, .obj automatically
                    newLink->model = std::make_shared<Model>(meshPath);
                }
            }

            // 2. Material (Color)
            XMLElement* mat = visual->FirstChildElement("material");
            if (mat) {
                XMLElement* color = mat->FirstChildElement("color");
                if (color && color->Attribute("rgba")) {
                    float r, g, b, a;
                    sscanf(color->Attribute("rgba"), "%f %f %f %f", &r, &g, &b, &a);
                    newLink->color = glm::vec3(r, g, b);
                }
            }
        }

        // Store in our lists
        robot.allLinks.push_back(newLink);
        linkMap[newLink->name] = newLink.get();

        xmlLink = xmlLink->NextSiblingElement("link");
    }

    // -----------------------------------------
    // PASS 2: PARSE JOINTS (Hierarchy)
    // -----------------------------------------
    XMLElement* xmlJoint = root->FirstChildElement("joint");
    while (xmlJoint) {
        const char* parentName = xmlJoint->FirstChildElement("parent")->Attribute("link");
        const char* childName = xmlJoint->FirstChildElement("child")->Attribute("link");

        if (linkMap.count(parentName) && linkMap.count(childName)) {
            RobotLink* parent = linkMap[parentName];
            RobotLink* child = linkMap[childName];

            // Build Tree
            child->parent = parent;
            parent->children.push_back(child);

            // Parse Transform info
            XMLElement* origin = xmlJoint->FirstChildElement("origin");
            if (origin) {
                // Note: We do NOT swap Y/Z here because we are rotating the whole robot
                // -90 degrees in Robot::Draw. We read the raw URDF (Z-up) data.
                if(origin->Attribute("xyz")) 
                child->jointOrigin = ParseVec3(origin->Attribute("xyz"));
                        }

            XMLElement* axis = xmlJoint->FirstChildElement("axis");
            if (axis) {
                child->jointAxis = ParseVec3(axis->Attribute("xyz"));
            }
        }

        xmlJoint = xmlJoint->NextSiblingElement("joint");
    }

    // -----------------------------------------
    // PASS 3: FIND ROOT
    // -----------------------------------------
    for (auto& link : robot.allLinks) {
        if (link->parent == nullptr) {
            robot.rootLink = link.get();
            break;
        }
    }

    if (!robot.rootLink) {
        std::cout << "URDF ERROR: Robot has circular dependency or no root!" << std::endl;
        return false;
    }

    std::cout << "URDF Loaded Successfully. Root: " << robot.rootLink->name << std::endl;
    return true;
}