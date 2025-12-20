#include "Physics.h"
#include <iostream>
#include <cstring>

MujocoSim::MujocoSim() {
    // Optional: Set your MuJoCo license path if using an older version
    // mj_activate("mjkey.txt"); 
}

MujocoSim::~MujocoSim() {
    cleanup();
}

void MujocoSim::loadModel(const std::string& modelPath) {
    cleanup();

    m_ = mj_loadXML(modelPath.c_str(), nullptr, error_, 1000);
    if (!m_) {
        throw std::runtime_error("Failed to load MuJoCo model: " + std::string(error_));
    }

    d_ = mj_makeData(m_);
    if (!d_) {
        cleanup();
        throw std::runtime_error("Failed to create MuJoCo data structure.");
    }
    
    std::cout << "Successfully loaded model: " << modelPath << std::endl;
    std::cout << "Geom count: " << m_->ngeom << std::endl;
}

void MujocoSim::cleanup() {
    if (d_) {
        mj_deleteData(d_);
        d_ = nullptr;
    }
    if (m_) {
        mj_deleteModel(m_);
        m_ = nullptr;
    }
}

void MujocoSim::step() {
    if (m_ && d_) {
        mj_step(m_, d_);
    }
}

void MujocoSim::advance(double duration) {
    if (!m_ || !d_) return;

    mjtNum start_time = d_->time;
    while (d_->time - start_time < duration) {
        mj_step(m_, d_);
    }
}

int MujocoSim::getGeomCount() const {
    return m_ ? m_->ngeom : 0;
}

std::string MujocoSim::getGeomName(int index) const {
    if (!m_ || index < 0 || index >= m_->ngeom) return "unknown";
    const char* name = mj_id2name(m_, mjOBJ_GEOM, index);
    return name ? std::string(name) : "geom_" + std::to_string(index);
}

void MujocoSim::getGeomTransform(int index, float* pos, float* mat) const {
    if (!m_ || !d_ || index < 0 || index >= m_->ngeom) return;

    // MuJoCo uses mjtNum (usually double) for internal calculations
    // We cast them to float for typical OpenGL pipeline compatibility
    for (int i = 0; i < 3; ++i) {
        pos[i] = static_cast<float>(d_->geom_xpos[index * 3 + i]);
    }

    for (int i = 0; i < 9; ++i) {
        mat[i] = static_cast<float>(d_->geom_xmat[index * 9 + i]);
    }
}