// Rope bridge using Octet with Bullet physics.
// Might also be a falling sphere...

namespace octet {
  class rope_bridge : public app {
    // scene for drawing objects
    ref<visual_scene> app_scene;
    dynarray<mesh_instance*> mesh_instances;
    dynarray<btRigidBody*> rigid_bodies;

  public:
    rope_bridge(int argc, char **argv) : app(argc, argv) {
    }

    ~rope_bridge() {
    }

    /// Called once OpenGL is initialised.
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 5, 0));
      btDynamicsWorld *dynamics_world = app_scene->get_dynamics_world();

      const int bridge_height = 3;
      const int bridge_length_in_planks = 7;
      const vec3 starting_platform_location(-12, 4, 0);

      create_bridge(app_scene, bridge_height, bridge_length_in_planks, starting_platform_location);

      // ground
      material *ground_color = new material(vec4(0, 1, 0, 1));
      mat4t mat;
      mat.loadIdentity();
      app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), ground_color, false);

      /*// materials
      material *red = new material(vec4(1, 0, 0, 1));
      material *green = new material(vec4(0, 1, 0, 1));
      material *blue = new material(vec4(0, 0, 1, 1));

      // place planks
      mat4t mat;
      create_plank(mat, blue, true, vec3(6, 5, 0));
      create_plank(mat, green, false, vec3(0, 10, 0));
      create_plank(mat, red, true, vec3(0, 6, 0));
      create_plank(mat, green, false, vec3(-9, 8, 0));
      create_plank(mat, green, true, vec3(-6, 8, 0));

      mesh_instances.push_back(app_scene->get_mesh_instance(0));
      mesh_instances.push_back(app_scene->get_mesh_instance(1));
      mesh_instances.push_back(app_scene->get_mesh_instance(2));
      mesh_instances.push_back(app_scene->get_mesh_instance(3));
      mesh_instances.push_back(app_scene->get_mesh_instance(4));
      rigid_bodies.push_back(mesh_instances[0]->get_node()->get_rigid_body());
      rigid_bodies.push_back(mesh_instances[1]->get_node()->get_rigid_body());
      rigid_bodies.push_back(mesh_instances[2]->get_node()->get_rigid_body());
      rigid_bodies.push_back(mesh_instances[3]->get_node()->get_rigid_body());
      rigid_bodies.push_back(mesh_instances[4]->get_node()->get_rigid_body());

      // hinges
      btHingeConstraint *hinge0 = new btHingeConstraint(*rigid_bodies[0], btVector3(-1, 0.5f, 0), btVector3(0, 0, 1), false);
      btHingeConstraint *hinge1 = new btHingeConstraint(*rigid_bodies[3], *rigid_bodies[4], btVector3(1.5f, 0, 0), btVector3(-1.5f, 0, 0),
                                                         btVector3(0, 0, 1), btVector3(0, 0, 1));

      const float PI = 3.14159f;
      btScalar lower_angle_limit = 0;
      btScalar upper_angle_limit = PI / 6;
      hinge1->setLimit(lower_angle_limit, upper_angle_limit);

      dynamics_world->addConstraint(hinge0);
      dynamics_world->addConstraint(hinge1);

      // generic 6DOF spring
      // Acknowledgement: http://bullet.googlecode.com/svn/trunk/Demos/ConstraintDemo/ConstraintDemo.cpp
      btTransform frame_in_a, frame_in_b;
      frame_in_a = btTransform::getIdentity();
      frame_in_a.setOrigin(btVector3(btScalar(0), btScalar(0), btScalar(0)));
      frame_in_b = btTransform::getIdentity();
      frame_in_b.setOrigin(btVector3(btScalar(0), btScalar(4), btScalar(0)));
      btGeneric6DofSpringConstraint *spring = new btGeneric6DofSpringConstraint(*rigid_bodies[1], *rigid_bodies[2], frame_in_a, frame_in_b, true);

      spring->setLinearUpperLimit(btVector3(0, 2, 0));
      spring->setLinearLowerLimit(btVector3(0, -5, 0));

      spring->setAngularLowerLimit(btVector3(0, 0, 0));
      spring->setAngularUpperLimit(btVector3(0, 0, 0));

      dynamics_world->addConstraint(spring, false);

      spring->enableSpring(1, true);
      spring->setStiffness(1, 19);
      spring->setDamping(1, 0.5f);
      spring->setEquilibriumPoint();*/
    }

    /// Creates a plank for our bridge.
    void create_plank(mat4t mat, material *color, bool is_dynamic, vec3 location) {
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(vec3(1, 0.5f, 2)), color, is_dynamic);
    }

    /// Assemble the bridge.
    void create_bridge(ref<visual_scene> &app_scene, int height, int length_in_planks, vec3 start_location) {
      material *platform_color = new material(vec4(1, 0, 0, 1));
      material *plank_color = new material(vec4(0, 0, 1, 1));
      const float PI = 3.14159f;

      // place inital platform
      mat4t mat;
      mat.loadIdentity();
      mat.translate(start_location);
      app_scene->add_shape(mat, new mesh_box(vec3(1, height, 2)), platform_color, false);
      mesh_instances.push_back(app_scene->get_mesh_instance(0));
      rigid_bodies.push_back(mesh_instances[0]->get_node()->get_rigid_body());

      // the first plank's origin
      vec3 plank_location = vec3(start_location[0], start_location[1] + (height - 0.5f), start_location[2]);

      for (int i = 0; i < length_in_planks; ++i) {
        int index = i + 1;
        float x = plank_location[0] + 3;
        plank_location = vec3(x, plank_location[1], plank_location[2]);
        create_plank(mat, plank_color, true, plank_location);
        // use an enum for platform and plank indices?
        // index is i + 1 as the first platform has index 0
        mesh_instances.push_back(app_scene->get_mesh_instance(index));
        rigid_bodies.push_back(mesh_instances[index]->get_node()->get_rigid_body());

        btHingeConstraint *hinge;

        if (i == 0) {
          hinge = new btHingeConstraint(*rigid_bodies[i], *rigid_bodies[index], btVector3(1.5f, height - 0.5f, 0),
            btVector3(-1.5f, 0, 0), btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        } else {
          hinge = new btHingeConstraint(*rigid_bodies[i], *rigid_bodies[index], btVector3(1.5f, 0, 0),
            btVector3(-1.5f, 0, 0), btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        }

        btScalar lower_angle_limit = 0;
        btScalar upper_angle_limit = PI / 6;
        hinge->setLimit(lower_angle_limit, upper_angle_limit);
        app_scene->get_dynamics_world()->addConstraint(hinge);
      }

      // place final platform
      mat.loadIdentity();
      mat.translate(vec3(plank_location[0] + 3, start_location[1], plank_location[2]));
      app_scene->add_shape(mat, new mesh_box(vec3(1, height, 2)), platform_color, false);
      mesh_instances.push_back(app_scene->get_mesh_instance(length_in_planks + 1));
      rigid_bodies.push_back(mesh_instances[length_in_planks+1]->get_node()->get_rigid_body());

      // add hinge between last plank and platform
      int last_index = rigid_bodies.size() - 1;
      btHingeConstraint *hinge = new btHingeConstraint(*rigid_bodies[last_index-1], *rigid_bodies[last_index], btVector3(1.5f, 0, 0),
        btVector3(-1.5f, height - 0.5f, 0), btVector3(0, 0, 1), btVector3(0, 0, 1), false);
      btScalar lower_angle_limit = 0;
      btScalar upper_angle_limit = PI / 6;
      hinge->setLimit(lower_angle_limit, upper_angle_limit);
      app_scene->get_dynamics_world()->addConstraint(hinge);
    }

    /// Called to draw the world.
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // used to test spring
      if (is_key_down(key_space))
        rigid_bodies[4]->applyForce(btVector3(0, 100, 0), btVector3(0, 0, 0));

      // update matrices, assume 30fps
      app_scene->update(1.0f / 30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}