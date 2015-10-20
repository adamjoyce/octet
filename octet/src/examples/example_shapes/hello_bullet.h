// Learning bullet
// Acknowledgement: http://bulletphysics.org/mediawiki-1.5.8/index.php/Hello_World

namespace octet {
  class hello_bullet : public app {
    // Build the broadphase
    btBroadphaseInterface *broadphase;
    // Setup the collision configuration and dispatcher
    btDefaultCollisionConfiguration *collision_configuration;
    btCollisionDispatcher *dispatcher;
    // The actual physics solver
    btSequentialImpulseConstraintSolver *solver;
    // The world
    btDiscreteDynamicsWorld *dynamics_world;

  public:
    hello_bullet(int argc, char **argv) : app(argc, argv) {
      broadphase = new btDbvtBroadphase();
      collision_configuration = new btDefaultCollisionConfiguration();
      dispatcher = new btCollisionDispatcher(collision_configuration);
      solver = new btSequentialImpulseConstraintSolver();
      dynamics_world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collision_configuration);
    }

    ~hello_bullet() {
      delete broadphase;
      delete collision_configuration;
      delete dispatcher;
      delete solver;
      delete dynamics_world;
    }

    void app_init() {
      // Collision shapes for our ground plane and sphere
      btCollisionShape *ground_shape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
      btCollisionShape *fall_shape = new btSphereShape(1);

      // Give the ground a rigid body
      btDefaultMotionState *ground_motion_state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
      btRigidBody::btRigidBodyConstructionInfo ground_rigid_body_ci(0, ground_motion_state, ground_shape, btVector3(0, 0, 0));
      btRigidBody *ground_rigid_body = new btRigidBody(ground_rigid_body_ci);

      // Add the ground to the world
      dynamics_world->addRigidBody(ground_rigid_body);

      // Similar for our sphere
      btDefaultMotionState *sphere_motion_state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));

      // Sort mass and inertia
      btScalar mass = 1;
      btVector3 fall_inertia(0, 0, 0);
      fall_shape->calculateLocalInertia(mass, fall_inertia);

      btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, sphere_motion_state, fall_shape, fall_inertia);
      btRigidBody *fall_rigid_body = new btRigidBody(fallRigidBodyCI);
      dynamics_world->addRigidBody(fall_rigid_body);

      // Step the simulation
      for (int i = 0; i < 300; i++) {
        dynamics_world->stepSimulation(1 / 60.f, 10);

        btTransform trans;
        fall_rigid_body->getMotionState()->getWorldTransform(trans);

        std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
      }

      printf("hello");
    }

    void draw_world(int x, int y, int w, int h) {
    }
  };
}