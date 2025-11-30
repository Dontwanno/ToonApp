#pragma once

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <vector>

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void Step(float deltaTime);

    // Factory methods to create rigid bodies
    btRigidBody* CreateBox(const glm::vec3& size, float mass, const glm::vec3& position);
    btRigidBody* CreateSphere(float radius, float mass, const glm::vec3& position);
    btRigidBody* CreateStaticPlane(); // The ground

    // Helper to cleanup bodies
    void RemoveBody(btRigidBody* body);

private:
    // Core Bullet components
    btDefaultCollisionConfiguration* collisionConfig;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* overlappingPairCache;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;

    std::vector<btCollisionShape*> collisionShapes;
};