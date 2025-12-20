#pragma once

#include <mujoco/mujoco.h>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief A class to manage MuJoCo physics independently of any visualizer.
 * Perfect for custom OpenGL rendering pipelines.
 */
class MujocoSim {
public:
    MujocoSim();
    ~MujocoSim();

    // Prevent copying
    MujocoSim(const MujocoSim&) = delete;
    MujocoSim& operator=(const MujocoSim&) = delete;

    /**
     * @brief Load a model file (e.g., Menagerie's spot/scene.xml)
     * @param modelPath Path to the MJCF or URDF file
     */
    void loadModel(const std::string& modelPath);

    /**
     * @brief Steps the simulation by one timestep (m->opt.timestep)
     */
    void step();

    /**
     * @brief Steps the simulation multiple times to match a target real-time duration
     * @param duration Real-time seconds to advance
     */
    void advance(double duration);

    /**
     * @brief Returns the number of geoms in the model
     */
    int getGeomCount() const;

    /**
     * @brief Gets the name of a geom by index
     */
    std::string getGeomName(int index) const;

    /**
     * @brief Gets the world-space transform for a specific geom
     * @param index The geom ID
     * @param pos Pointer to float[3] to receive position
     * @param mat Pointer to float[9] to receive 3x3 rotation matrix (row-major)
     */
    void getGeomTransform(int index, float* pos, float* mat) const;

    /**
     * @brief Access internal MuJoCo pointers (for advanced control)
     */
    mjModel* getModel() { return m_; }
    mjData* getData() { return d_; }

private:
    mjModel* m_ = nullptr;
    mjData* d_ = nullptr;
    char error_[1000];

    void cleanup();
};