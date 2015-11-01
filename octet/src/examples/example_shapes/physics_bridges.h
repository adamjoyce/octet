// Physics bridges using Octet with Bullet physics.
// Author: Adam Joyce
// Version: 2.3

#include "sound_system.h"

namespace octet {
  class physics_bridges : public app {
    // scene for drawing objects
    ref<visual_scene> app_scene;
    btDynamicsWorld *dynamics_world;

    //sound_system sound_sys;
    //FMOD::Sound *ball_sound;

    dynarray<mesh_instance*> mesh_instances;
    dynarray<btRigidBody*> rigid_bodies;
    random rand;

    enum {
      bridge_height = 5,
      plank_num = 7,
      platform_num = 2,
      hinge_plank_gap = 3,
      spring_plank_gap = 3,

      // position of the first hinge platform
      hinge_platform_x = -12,
      hinge_platform_y = bridge_height + 1,
      hinge_platform_z = -5,
      // x position of the second hinge platform
      hinge_platform2_x = hinge_platform_x + ((plank_num + 1) * hinge_plank_gap),

      // position of the first spring platform
      spring_platform_x = -12,
      spring_platform_y = bridge_height + 1,
      spring_platform_z = 10,
      // x position of the second spring platform
      spring_platform2_x = spring_platform_x + ((plank_num + 1) * spring_plank_gap),

      //ground = 0,

      first_hinge_platform = 0,
      last_hinge_platform = first_hinge_platform + 1,

      first_hinge_plank,
      last_hinge_plank = first_hinge_plank + plank_num - 1,

      first_spring_platform,
      last_spring_platform = first_spring_platform + 1,

      first_spring_plank,
      last_spring_plank = first_spring_plank + (plank_num * 2) - 1,
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
      dynamics_world = app_scene->get_dynamics_world();

      // ground
      /*material *ground_color = new material(vec4(0, 1, 0, 1));
      mat4t mat;
      mat.loadIdentity();
      app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), ground_color, false);
      mesh_instances.push_back(app_scene->get_mesh_instance(ground));
      rigid_bodies.push_back(mesh_instances[ground]->get_node()->get_rigid_body());*/


      // build the bridges
      create_hinge_bridge();
      create_spring_bridge();
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
      create_platform(mat, platform_color, vec3(hinge_platform2_x, hinge_platform_y, hinge_platform_z), bridge_height);
      mesh_instances.push_back(app_scene->get_mesh_instance(last_hinge_platform));
      rigid_bodies.push_back(mesh_instances[last_hinge_platform]->get_node()->get_rigid_body());

      // place and hinge the first plank to the platform
      vec3 plank_location = vec3(hinge_platform_x + hinge_plank_gap, hinge_platform_y + (bridge_height - 0.5f), hinge_platform_z);
      create_plank(mat, plank_color, true, plank_location);
      mesh_instances.push_back(app_scene->get_mesh_instance(first_hinge_plank));
      rigid_bodies.push_back(mesh_instances[first_hinge_plank]->get_node()->get_rigid_body());

      btHingeConstraint *hinge = new btHingeConstraint(*rigid_bodies[first_hinge_platform], *rigid_bodies[first_hinge_plank],
                                                        btVector3(1.5f, bridge_height - 0.5f, 0), btVector3(-1.5f, 0, 0),
                                                        btVector3(0, 0, 1), btVector3(0, 0, 1), false);
      dynamics_world->addConstraint(hinge);

      // place and hinge the remaining planks
      int i = first_hinge_plank + 1;
      for (i; i <= last_hinge_plank; i++) {
        plank_location = vec3(plank_location[0] + hinge_plank_gap, plank_location[1], plank_location[2]); 
        create_plank(mat, plank_color, true, plank_location);
        mesh_instances.push_back(app_scene->get_mesh_instance(i));
        rigid_bodies.push_back(mesh_instances[i]->get_node()->get_rigid_body());

        hinge = new btHingeConstraint(*rigid_bodies[i-1], *rigid_bodies[i], btVector3(1.5f, 0, 0), btVector3(-1.5f, 0, 0),
                                       btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        dynamics_world->addConstraint(hinge);

        // deal with the last plank to platform hinge
        if (i == last_hinge_plank) {
          hinge = new btHingeConstraint(*rigid_bodies[i], *rigid_bodies[last_hinge_platform], btVector3(1.5f, 0, 0), 
                                         btVector3(-1.5f, bridge_height - 0.5f, 0), btVector3(0, 0, 1), btVector3(0, 0, 1), false);
          dynamics_world->addConstraint(hinge);
        }
      }
    }

    /// Assemble spring bridge.
    /// Reference: http://bullet.googlecode.com/svn/trunk/Demos/ConstraintDemo/ConstraintDemo.cpp
    void create_spring_bridge() {
      material *platform_color = new material(vec4(1, 0, 0, 1));
      material *plank_color = new material(vec4(1, 1, 0, 1));

      // place the platforms
      // first platform
      mat4t mat;
      create_platform(mat, platform_color, vec3(spring_platform_x, spring_platform_y, spring_platform_z), bridge_height);
      mesh_instances.push_back(app_scene->get_mesh_instance(first_spring_platform));
      rigid_bodies.push_back(app_scene->get_mesh_instance(first_spring_platform)->get_node()->get_rigid_body());

      // last platform
      create_platform(mat, platform_color, vec3(spring_platform2_x, spring_platform_y, spring_platform_z), bridge_height);
      mesh_instances.push_back(app_scene->get_mesh_instance(last_spring_platform));
      rigid_bodies.push_back(app_scene->get_mesh_instance(last_spring_platform)->get_node()->get_rigid_body());

      btTransform frame_in_a, frame_in_b;
      btGeneric6DofSpringConstraint *spring;
      vec3 plank_location = vec3(spring_platform_x, spring_platform_y, spring_platform_z);
      for (int i = 0; i <= last_spring_plank - first_spring_plank; i += 2) {
        // create the spring platforms
        float new_x_location = plank_location[0] + hinge_plank_gap;
        plank_location = vec3(new_x_location, plank_location[1], plank_location[2]);
        create_plank(mat, plank_color, true, plank_location);
        mesh_instances.push_back(app_scene->get_mesh_instance(first_spring_plank + i));
        rigid_bodies.push_back(mesh_instances[first_spring_plank+i]->get_node()->get_rigid_body());

        // create the spring anchors
        create_plank(mat, plank_color, false, vec3(plank_location[0], plank_location[1] + 30, plank_location[2]));
        mesh_instances.push_back(app_scene->get_mesh_instance(first_spring_plank + i + 1));
        rigid_bodies.push_back(mesh_instances[first_spring_plank+i+1]->get_node()->get_rigid_body());

        frame_in_a = btTransform::getIdentity();
        frame_in_a.setOrigin(btVector3(btScalar(0), btScalar(0), btScalar(0)));
        frame_in_b = btTransform::getIdentity();
        frame_in_b.setOrigin(btVector3(btScalar(0), btScalar(25), btScalar(0)));
        spring = new btGeneric6DofSpringConstraint(*rigid_bodies[first_spring_plank+i+1], *rigid_bodies[first_spring_plank+i], 
                                                    frame_in_a, frame_in_b, true);

        spring->setLinearUpperLimit(btVector3(0, 0, 0));
        spring->setLinearLowerLimit(btVector3(0, -10, 0));
        spring->setAngularLowerLimit(btVector3(0, 0, 0));
        spring->setAngularUpperLimit(btVector3(0, 0, 0));

        spring->enableSpring(1, true);
        spring->setStiffness(1, 19);
        spring->setDamping(1, 0.5f);
        //spring->setEquilibriumPoint();
        app_scene->get_dynamics_world()->addConstraint(spring);
      }
    }

    /// Detect collisions.
    /// Reference: http://hamelot.co.uk/programming/using-bullet-only-for-collision-detection/
    void handle_collisions() {
      // perform collision detection
      dynamics_world->performDiscreteCollisionDetection();
      int num_manifolds = dynamics_world->getDispatcher()->getNumManifolds();

      // for each contact manifold
      for (int i = 0; i < num_manifolds; i++) {
        btPersistentManifold *contact_manifold = dynamics_world->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject *object_a = static_cast<const btCollisionObject*>(contact_manifold->getBody0());
        const btCollisionObject *object_b = static_cast<const btCollisionObject*>(contact_manifold->getBody1());
        contact_manifold->refreshContactPoints(object_a->getWorldTransform(), object_b->getWorldTransform());
        int num_contacts = contact_manifold->getNumContacts();

        // for each contact point in that manifold
        for (int j = 0; j < num_contacts; j++) {
          // get the contact information
          btManifoldPoint &point = contact_manifold->getContactPoint(j);
          btVector3 point_a = point.getPositionWorldOnA();
          btVector3 point_b = point.getPositionWorldOnB();
          double point_dist = point.getDistance();
          //sound_sys.play_sound(ball_sound, false);
        }
      }
    }

    /// Set up the ball sound.
    void sound_setup() {
      //sound_sys = sound_system();
      //sound_sys.create_sound(ball_sound, "ball_sound.mp3");
    }

    /// Called to draw the world.
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // used to test spring
      mat4t mat;
      material *color = new material(vec4(0, 0, 0, 1));
      float r = rand.get(first_spring_plank, last_spring_plank);
      if (is_key_down(key_space)) {
        mat.loadIdentity();
        mat.translate(vec3(-6, 30, hinge_platform_z));
        app_scene->add_shape(mat, new mesh_sphere(), color, true);
        //rigid_bodies[r]->applyForce(btVector3(0, -200, 0), btVector3(0, 0, 0));
      }

      // update matrices, assume 30fps
      app_scene->update(1.0f / 30);

      // draw the scene
      app_scene->render((float)vx / vy);

      handle_collisions();
    }
  };
}