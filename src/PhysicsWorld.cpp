#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld() {
    // Standard Bullet Setup
    collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    overlappingPairCache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfig);

    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
}

PhysicsWorld::~PhysicsWorld() {
    // Cleanup in reverse order
    for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState()) {
            delete body->getMotionState();
        }
        dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }

    for (int i = 0; i < collisionShapes.size(); i++) {
        delete collisionShapes[i];
    }

    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfig;
}

void PhysicsWorld::Step(float deltaTime) {
    // Step simulation at 60Hz
    dynamicsWorld->stepSimulation(deltaTime, 10);
}

btRigidBody* PhysicsWorld::CreateBox(const glm::vec3& size, float mass, const glm::vec3& position) {
    btCollisionShape* shape = new btBoxShape(btVector3(size.x, size.y, size.z));
    collisionShapes.push_back(shape);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(position.x, position.y, position.z));

    // Mass = 0 means static (immovable)
    btVector3 localInertia(0, 0, 0);
    if (mass != 0.0f)
        shape->calculateLocalInertia(mass, localInertia);

    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
    
    btRigidBody* body = new btRigidBody(rbInfo);
    dynamicsWorld->addRigidBody(body);
    
    return body;
}

btRigidBody* PhysicsWorld::CreateSphere(float radius, float mass, const glm::vec3& position) {
    btCollisionShape* shape = new btSphereShape(radius);
    collisionShapes.push_back(shape);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(position.x, position.y, position.z));

    btVector3 localInertia(0, 0, 0);
    if (mass != 0.0f)
        shape->calculateLocalInertia(mass, localInertia);

    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
    
    btRigidBody* body = new btRigidBody(rbInfo);
    dynamicsWorld->addRigidBody(body);
    
    return body;
}

btRigidBody* PhysicsWorld::CreateStaticPlane() {
    // Plane defined by Normal (0,1,0) and Constant (0) -> y=0 ground
    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    collisionShapes.push_back(groundShape);

    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    
    btRigidBody* groundBody = new btRigidBody(groundRigidBodyCI);
    dynamicsWorld->addRigidBody(groundBody);
    
    return groundBody;
}

void PhysicsWorld::RemoveBody(btRigidBody* body) {
    if (body) {
        dynamicsWorld->removeRigidBody(body);
        delete body->getMotionState();
        delete body;
    }
}