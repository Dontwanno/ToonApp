#pragma once
#include <string>

class Robot; 

class URDFLoader {
public:
    static bool Load(const std::string& path, Robot& robot);
};