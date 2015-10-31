// Rope bridge using Octet with Bullet physics.
// Author: Adam Joyce
// Version: 2.3

namespace octet {
  class physics_bridges : public app {
    // scene for drawing objects
    ref<visual_scene> app_scene;
    dynarray<mesh_instance*> mesh_instances;
    dynarray<btRigidBody*> rigid_bodies;

    enum {
      bridge_height = 5,
      plank_num = 7,
      platform_num = 2,
      plank_gap = 3,

      hinge_platform_x = -12,
      hinge_platform_y = bridge_height + 1,
      hinge_platform_z = -2,

      spring_platform_x = -12,
      spring_platform_y = bridge_height + 1,
      spring_platform_z = 2,

      first_hinge_platform = 0,
      last_hinge_platform = first_hinge_platform + 1,

      first_hinge_plank,
      last_hinge_plank = first_hinge_plank + plank_num - 1,
    };

  public:
    physics_bridges(int argc, char **argv) : app(argc, argv) {
    }

    ~physics_bridges() {
    }

  private:
    /// Called once OpenGL is initialised.
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 40, 0));
      app_scene->get_camera_instance(0)->get_node()->rotate(-45, vec3(1, 0, 0));
      btDynamicsWorld *dynamics_world = app_scene->get_dynamics_world();

      // build the bridges
      create_hinge_bridge();
      create_spring_bridge();

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

    /// Creates a plank for a bridge.
    void create_plank(mat4t mat, material *color, bool is_dynamic, vec3 location) {
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(vec3(1, 0.5f, 2)), color, is_dynamic);
    }

    /// Creates a platform for a bridge.
    void create_platform(mat4t mat, material *color, vec3 location, int height) {
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(vec3(1, height, 2)), color, false);
    }

    /// Assemble the hinge bridge.
    void create_hinge_bridge() {
      material *platform_color = new material(vec4(1, 0, 0, 1));
      material *plank_color = new material(vec4(0, 0, 1, 1));

      // place the platforms
      // first platform
      mat4t mat;
      create_platform(mat, platform_color, vec3(hinge_platform_x, hinge_platform_y, hinge_platform_z), bridge_height);
      mesh_instances.push_back(app_scene->get_mesh_instance(first_hinge_platform));
      rigid_bodies.push_back(mesh_instances[first_hinge_platform]->get_node()->get_rigid_body());

      // last platform
      float new_x_position = hinge_platform_x + ((plank_num + 1) * 3);
      create_platform(mat, platform_color, vec3(new_x_position, hinge_platform_y, hinge_platform_z), bridge_height);
      mesh_instances.push_back(app_scene->get_mesh_instance(last_hinge_platform));
      rigid_bodies.push_back(mesh_instances[last_hinge_platform]->get_node()->get_rigid_body());

      // place and hinge the first plank to the platform
      vec3 plank_location = vec3(hinge_platform_x + plank_gap, hinge_platform_y + (bridge_height - 0.5f), hinge_platform_z);
      create_plank(mat, plank_color, true, plank_location);
      mesh_instances.push_back(app_scene->get_mesh_instance(first_hinge_plank));
      rigid_bodies.push_back(mesh_instances[first_hinge_plank]->get_node()->get_rigid_body());

      btHingeConstraint *hinge = new btHingeConstraint(*rigid_bodies[first_hinge_platform], *rigid_bodies[first_hinge_plank],
                                                        btVector3(1.5f, bridge_height - 0.5f, 0), btVector3(-1.5f, 0, 0),
                                                        btVector3(0, 0, 1), btVector3(0, 0, 1), false);
      app_scene->get_dynamics_world()->addConstraint(hinge);

      // place and hinge the remaining planks
      int i = first_hinge_plank + 1;
      for (i; i <= last_hinge_plank; ++i) {
        plank_location = vec3(plank_location[0] + plank_gap, plank_location[1], plank_location[2]); 
        create_plank(mat, plank_color, true, plank_location);
        mesh_instances.push_back(app_scene->get_mesh_instance(i));
        rigid_bodies.push_back(mesh_instances[i]->get_node()->get_rigid_body());

        hinge = new btHingeConstraint(*rigid_bodies[i-1], *rigid_bodies[i], btVector3(1.5f, 0, 0), btVector3(-1.5f, 0, 0),
                                       btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        app_scene->get_dynamics_world()->addConstraint(hinge);

        // deal with the last plank to platform hinge
        if (i == last_hinge_plank) {
          hinge = new btHingeConstraint(*rigid_bodies[i], *rigid_bodies[last_hinge_platform], btVector3(1.5f, 0, 0), 
                                         btVector3(-1.5f, bridge_height - 0.5f, 0), btVector3(0, 0, 1), btVector3(0, 0, 1), false);
          app_scene->get_dynamics_world()->addConstraint(hinge);
        }
      }
    }

    /// Assemble spring bridge.
    void create_spring_bridge() {
      material *platform_color = new material(vec4(1, 0, 0, 1));
      material *plank_color = new material(vec4(0, 1, 0, 1));
    }

    /// Called to draw the world.
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // used to test spring
      if (is_key_down(key_space))
        rigid_bodies[first_hinge_plank + (plank_num * 0.5f)]->applyForce(btVector3(0, 100, 0), btVector3(0, 0, 0));

      // update matrices, assume 30fps
      app_scene->update(1.0f / 30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}