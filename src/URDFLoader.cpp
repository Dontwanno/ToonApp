#include "URDFLoader.h"
#include "Robot.h"
#include <tinyxml2.h>
#include <iostream>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// #define GLM_ENABLE_EXPERIMENTAL // Uncomment if needed for your specific GLM version
// #include <glm/gtx/euler_angles.hpp> 

#include "FileSystem.h"

using namespace tinyxml2;

// --- HELPERS ---

static glm::vec3 ParseVec3(const char* str) {
    glm::vec3 vec(0.0f);
    if (str) sscanf(str, "%f %f %f", &vec.x, &vec.y, &vec.z);
    return vec;
}

static std::string ResolvePath(std::string path) {
    std::string prefix = "package://";
    size_t pos = path.find(prefix);
    if (pos != std::string::npos) {
        path.erase(pos, prefix.length());
    }

    std::string wrongPrefix = "drake_models/iiwa_description/";
    size_t wrongPos = path.find(wrongPrefix);
    
    if (wrongPos != std::string::npos) {
        path.replace(wrongPos, wrongPrefix.length(), "assets/kuka/");
    } else {
        path = "assets/" + path;
    }

    return FileSystem::getPath(path);
}

bool URDFLoader::Load(const std::string& path, Robot& robot) {
    // Resolve the path using FileSystem to get the absolute path
    std::string fullPath = FileSystem::getPath(path);
    
    XMLDocument doc;
    if (doc.LoadFile(fullPath.c_str()) != XML_SUCCESS) {
        std::cout << "URDF LOAD ERROR: Could not open " << fullPath << std::endl;
        return false;
    }

    XMLElement* root = doc.FirstChildElement("robot");
    // SDF compatibility: sometimes root is <sdf><model>...
    if (!root) root = doc.FirstChildElement("sdf"); 
    if (root && root->FirstChildElement("model")) root = root->FirstChildElement("model");
    
    if (!root) return false;

    robot.allLinks.clear();
    robot.rootLink = nullptr;

    std::map<std::string, RobotLink*> linkMap;

    // -----------------------------------------
    // PASS 1: PARSE ALL LINKS (Visuals & Names)
    // -----------------------------------------
    XMLElement* xmlLink = root->FirstChildElement("link");
    while (xmlLink) {
        auto newLink = std::make_shared<RobotLink>();
        newLink->name = xmlLink->Attribute("name");
        
        // -- Loop through ALL Visual tags (Fixed) --
        XMLElement* visual = xmlLink->FirstChildElement("visual");
        while (visual) {
            RobotVisual newVisual;
            
            // Parse Visual Origin
            XMLElement* visOrigin = visual->FirstChildElement("origin");
            // SDF often uses <pose> instead of <origin>
            XMLElement* visPose = visual->FirstChildElement("pose");
            
            if (visOrigin) {
                if(visOrigin->Attribute("xyz")) 
                    newVisual.offset = ParseVec3(visOrigin->Attribute("xyz"));
                if(visOrigin->Attribute("rpy")) 
                    newVisual.rpy = ParseVec3(visOrigin->Attribute("rpy"));
            } 
            else if (visPose) {
                // Quick hack for SDF pose "x y z r p y"
                // Ideally you'd parse 6 floats
            }

            // Geometry
            XMLElement* geo = visual->FirstChildElement("geometry");
            if (geo) {
                XMLElement* mesh = geo->FirstChildElement("mesh");
                if (mesh) {
                    const char* filename = nullptr;
                    // URDF uses attribute "filename", SDF uses <uri> child
                    if (mesh->Attribute("filename")) filename = mesh->Attribute("filename");
                    else if (mesh->FirstChildElement("uri")) filename = mesh->FirstChildElement("uri")->GetText();

                    if (filename) {
                        std::string meshPath = ResolvePath(filename);
                        newVisual.model = std::make_shared<Model>(meshPath);
                    }
                }
            }

            // Material
            XMLElement* mat = visual->FirstChildElement("material");
            if (mat) {
                XMLElement* color = mat->FirstChildElement("color");
                if (color && color->Attribute("rgba")) {
                    float r, g, b, a;
                    sscanf(color->Attribute("rgba"), "%f %f %f %f", &r, &g, &b, &a);
                    newVisual.color = glm::vec3(r, g, b);
                }
            }
            
            // Only add if we successfully loaded a model
            if (newVisual.model) {
                newLink->visuals.push_back(newVisual);
            }

            // Move to next sibling <visual> tag
            visual = visual->NextSiblingElement("visual");
        }

        robot.allLinks.push_back(newLink);
        linkMap[newLink->name] = newLink.get();

        xmlLink = xmlLink->NextSiblingElement("link");
    }

    // -----------------------------------------
    // PASS 2: PARSE JOINTS (Hierarchy)
    // -----------------------------------------
    XMLElement* xmlJoint = root->FirstChildElement("joint");
    while (xmlJoint) {
        XMLElement* parentEl = xmlJoint->FirstChildElement("parent");
        XMLElement* childEl = xmlJoint->FirstChildElement("child");

        if (parentEl && childEl) {
            const char* parentName = parentEl->Attribute("link");
            const char* childName = childEl->Attribute("link");
            
            // SDF fallback: <parent>link_name</parent> (text content)
            if (!parentName && parentEl->GetText()) parentName = parentEl->GetText();
            if (!childName && childEl->GetText()) childName = childEl->GetText();

            if (parentName && childName && linkMap.count(parentName) && linkMap.count(childName)) {
                RobotLink* parent = linkMap[parentName];
                RobotLink* child = linkMap[childName];

                child->parent = parent;
                parent->children.push_back(child);

                XMLElement* origin = xmlJoint->FirstChildElement("origin");
                XMLElement* pose = xmlJoint->FirstChildElement("pose"); // SDF

                if (origin) {
                    if(origin->Attribute("xyz")) 
                        child->jointOrigin = ParseVec3(origin->Attribute("xyz"));
                    if(origin->Attribute("rpy"))
                        child->jointRPY = ParseVec3(origin->Attribute("rpy"));
                }
                else if (pose && pose->GetText()) {
                    // SDF Pose parsing "0 0 0.1575 0 0 0" (x y z r p y)
                    float x, y, z, r, p, yaw;
                    sscanf(pose->GetText(), "%f %f %f %f %f %f", &x, &y, &z, &r, &p, &yaw);
                    child->jointOrigin = glm::vec3(x, y, z);
                    child->jointRPY = glm::vec3(r, p, yaw);
                }

                XMLElement* axis = xmlJoint->FirstChildElement("axis");
                if (axis) {
                    if (axis->Attribute("xyz"))
                        child->jointAxis = ParseVec3(axis->Attribute("xyz"));
                    else if (axis->FirstChildElement("xyz")) // SDF style
                        child->jointAxis = ParseVec3(axis->FirstChildElement("xyz")->GetText());
                }
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