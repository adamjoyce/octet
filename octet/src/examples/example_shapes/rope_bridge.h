// Rope bridge using Octet with Bullet physics.
// Might also be a falling sphere...

namespace octet {
  class rope_bridge : public app {
    // scene for drawing objects
    ref<visual_scene> app_scene;
    dynarray<btRigidBody*> rb;

  public:
    rope_bridge(int argc, char **argv) : app(argc, argv) {
    }

    ~rope_bridge() {
    }

    /// Called once OpenGL is initialised.
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 3, 0));
      btDynamicsWorld *dynamics_world = app_scene->get_dynamics_world();

      // materials
      material *red = new material(vec4(1, 0, 0, 1));
      material *green = new material(vec4(0, 1, 0, 1));
      material *blue = new material(vec4(0, 0, 1, 1));

      // place planks
      mat4t mat;
      create_plank(mat, blue, true, vec3(6, 5, 0));
      create_plank(mat, green, false, vec3(0, 10, 0));
      create_plank(mat, red, true, vec3(0, 6, 0));

      mesh_instance *plank0 = app_scene->get_mesh_instance(0);
      mesh_instance *plank1 = app_scene->get_mesh_instance(1);
      mesh_instance *plank2 = app_scene->get_mesh_instance(2);
      rb.push_back(plank0->get_node()->get_rigid_body());
      rb.push_back(plank1->get_node()->get_rigid_body());
      rb.push_back(plank2->get_node()->get_rigid_body());

      // ground
      mat.loadIdentity();
      app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), green, false);

      // hinges
      btHingeConstraint *hinge = new btHingeConstraint(*rb[0], btVector3(-1, 0.5f, 0), btVector3(0, 0, 1), false);
      dynamics_world->addConstraint(hinge);

      // generic 6DOF spring
      // Acknowledgement: http://bullet.googlecode.com/svn/trunk/Demos/ConstraintDemo/ConstraintDemo.cpp
      btTransform frame_in_a, frame_in_b;
      frame_in_a = btTransform::getIdentity();
      frame_in_a.setOrigin(btVector3(btScalar(0), btScalar(0), btScalar(0)));
      frame_in_b = btTransform::getIdentity();
      frame_in_b.setOrigin(btVector3(btScalar(0), btScalar(4), btScalar(0)));
      btGeneric6DofSpringConstraint *spring = new btGeneric6DofSpringConstraint(*rb[1], *rb[2], frame_in_a, frame_in_b, true);

      spring->setLinearUpperLimit(btVector3(0, 2, 0));
      spring->setLinearLowerLimit(btVector3(0, -5, 0));

      spring->setAngularLowerLimit(btVector3(0, 0, 0));
      spring->setAngularUpperLimit(btVector3(0, 0, 0));

      dynamics_world->addConstraint(spring, false);

      spring->enableSpring(1, true);
      spring->setStiffness(1, 19);
      spring->setDamping(1, 0.5f);
      spring->setEquilibriumPoint();

      //rb_plank0->applyForce(btVector3(0, 400, 0), btVector3(0, 0, 0));
    }

    /// Creates a plank for our rope bridge.
    void create_plank(mat4t mat, material *color, bool is_dynamic, vec3 location) {
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(vec3(1, 0.5f, 2)), color, is_dynamic);
    }

    /// Called to draw the world.
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      if (is_key_down(key_space))
        rb[2]->applyForce(btVector3(0, 10, 0), btVector3(0, 0, 0));

      // update matrices, assume 30fps
      app_scene->update(1.0f / 30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}