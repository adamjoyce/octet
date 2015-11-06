// Physics bridges using Octet with Bullet physics.
// Author: Adam Joyce
// Version: 2.3

#include "../../resources/csv_parser.h"
//#include "sound_system.h"

namespace octet {
  class physics_bridges : public app {
    // scene for drawing objects
    ref<visual_scene> app_scene;
    // dynamics world
    btDynamicsWorld *dynamics_world;

    // store shape's mesh_instances and rigid_bodies
    dynarray<mesh_instance*> mesh_instances;
    dynarray<btRigidBody*> rigid_bodies;

    // store the bridge data from csv
    dynarray<vec4> data;

    // materials
    material *ground_color;
    material *platform_color;
    material *hinge_plank_color;
    material *spring_plank_color;
    material *ball_color;

    random rand;

    // rough frame counter for dropping balls
    int frame;

    // second platform x coordinates
    int hinge_plat2_pos;
    int spring_plat2_pos;

    //sound_system sound_sys;
    //FMOD::Sound *ball_sound;

    enum {
      // data array indices
      bridge_height = 0,
      plank_num,
      hinge_plank_gap,
      spring_plank_gap,

      hinge_plat_pos,
      spring_plat_pos,

      // mesh and rigid body array indices that do not require csv data to calculate
      ground = 0,
      first_hinge_platform,
      last_hinge_platform,

      /*first_hinge_plank,
      last_hinge_plank = first_hinge_plank + plank_num - 1,

      first_spring_platform,
      last_spring_platform = first_spring_platform + 1,

      first_spring_plank,
      last_spring_plank = first_spring_plank + (plank_num * 2) - 1,*/
    };

    // remaining indices for mesh and rigid body arrays based on csv data
    int first_hinge_plank;
    int last_hinge_plank;
    int first_spring_platform;
    int last_spring_platform;
    int first_spring_plank;
    int last_spring_plank;

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

      ground_color = new material(vec4(0, 1, 0, 1));
      platform_color = new material(vec4(1, 0, 0, 1));
      hinge_plank_color = new material(vec4(0, 0, 1, 1));
      spring_plank_color = new material(vec4(1, 1, 0, 1));
      ball_color = new material(vec4(0, 0, 0, 1));

      frame = 0;

      // load platform locations from csv and calculate remaining array indices
      load_csv_data();
      calculate_indices();

      // ground
      mat4t mat;
      mat.loadIdentity();
      app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), ground_color, false);
      add_to_arrays(ground);

      // build the bridges
      create_hinge_bridge();
      create_spring_bridge();
    }

    /// Add a shape's mesh_instance and rigid_body to the corresponding arrays.
    void add_to_arrays(int index) {
      mesh_instances.push_back(app_scene->get_mesh_instance(index));
      rigid_bodies.push_back(mesh_instances[index]->get_node()->get_rigid_body());
    }

    /// Creates a plank for a bridge.
    void create_plank(mat4t mat, material *color, bool is_dynamic, vec3 location, int array_index) {
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(vec3(1, 0.5f, 2)), color, is_dynamic);
      add_to_arrays(array_index);
    }

    /// Creates a platform for a bridge.
    void create_platform(mat4t mat, material *color, vec3 location, int height, int array_index) {
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(vec3(1, height, 2)), color, false);
      add_to_arrays(array_index);
    }

    /// Assemble the hinge bridge.
    void create_hinge_bridge() {
      // place the first and last platforms
      mat4t mat;
      create_platform(mat, platform_color, vec3(data[hinge_plat_pos][0], data[hinge_plat_pos][1], data[hinge_plat_pos][2]),
                      data[bridge_height][0], first_hinge_platform);

      // calculate the second platform's x coordinate
      hinge_plat2_pos = data[hinge_plat_pos][0] + ((data[plank_num][0] + 1) * data[hinge_plank_gap][0]);
      create_platform(mat, platform_color, vec3(hinge_plat2_pos, data[hinge_plat_pos][1], data[hinge_plat_pos][2]),
                      data[bridge_height][0], last_hinge_platform);

      // place and hinge the first plank to the first platform
      vec3 plank_location = vec3(data[hinge_plat_pos][0] + data[hinge_plank_gap][0], 
                                 data[hinge_plat_pos][1] + (data[bridge_height][0] - 0.5f), data[hinge_plat_pos][2]);
      create_plank(mat, hinge_plank_color, true, plank_location, first_hinge_plank);

      btHingeConstraint *hinge = new btHingeConstraint(*rigid_bodies[first_hinge_platform], *rigid_bodies[first_hinge_plank],
                                                        btVector3(1.5f, data[bridge_height][0] - 0.5f, 0), btVector3(-1.5f, 0, 0),
                                                        btVector3(0, 0, 1), btVector3(0, 0, 1), false);
      dynamics_world->addConstraint(hinge);

      // place and hinge the remaining planks
      int i = first_hinge_plank + 1;
      for (i; i <= last_hinge_plank; i++) {
        plank_location = vec3(plank_location[0] + data[hinge_plank_gap][0], plank_location[1], plank_location[2]); 
        create_plank(mat, hinge_plank_color, true, plank_location, i);

        hinge = new btHingeConstraint(*rigid_bodies[i-1], *rigid_bodies[i], btVector3(1.5f, 0, 0), btVector3(-1.5f, 0, 0),
                                       btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        dynamics_world->addConstraint(hinge);

        // deal with the last plank to platform hinge
        if (i == last_hinge_plank) {
          hinge = new btHingeConstraint(*rigid_bodies[i], *rigid_bodies[last_hinge_platform], btVector3(1.5f, 0, 0), 
                                         btVector3(-1.5f, data[bridge_height][0] - 0.5f, 0), btVector3(0, 0, 1), 
                                         btVector3(0, 0, 1), false);
          dynamics_world->addConstraint(hinge);
        }
      }
    }

    /// Assemble spring bridge.
    /// Reference: http://bullet.googlecode.com/svn/trunk/Demos/ConstraintDemo/ConstraintDemo.cpp
    void create_spring_bridge() {
      // place the first and last platforms
      mat4t mat;
      create_platform(mat, platform_color, vec3(data[spring_plat_pos][0], data[spring_plat_pos][1], data[spring_plat_pos][2]),
                      data[bridge_height][0], first_spring_platform);
      // calculate the second platform's x coordinate
      spring_plat2_pos = data[spring_plat_pos][0] + ((data[plank_num][0] + 1) * data[spring_plank_gap][0]);
      create_platform(mat, platform_color, vec3(spring_plat2_pos, data[spring_plat_pos][1], data[spring_plat_pos][2]),
                      data[bridge_height][0], last_spring_platform);

      btTransform frame_in_a, frame_in_b;
      btGeneric6DofSpringConstraint *spring;
      vec3 plank_location = vec3(data[spring_plat_pos][0], data[spring_plat_pos][1], data[spring_plat_pos][2]);
      for (int i = 0; i <= last_spring_plank - first_spring_plank; i += 2) {
        // create the spring planks
        float new_x_location = plank_location[0] + data[spring_plank_gap][0];
        plank_location = vec3(new_x_location, plank_location[1], plank_location[2]);
        create_plank(mat, spring_plank_color, true, plank_location, first_spring_plank + i);

        // create the spring anchors
        create_plank(mat, spring_plank_color, false, vec3(plank_location[0], plank_location[1] + 30, plank_location[2]),
                                                   first_spring_plank + i + 1);

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
    /// Issue with fmod in sound_system class that needs resolving.
    /*void sound_setup() {
      sound_sys = sound_system();
      sound_sys.create_sound(ball_sound, "ball_sound.mp3");
    }*/

    /// Create a ball.
    void create_ball(mat4t mat, vec3 location, material *color) {
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_sphere(), ball_color, true);
      add_to_arrays(mesh_instances.size());
    }

    /// Spawn ball objects above the bridges.
    void spawn_balls() {
      mat4t mat;

      float r_hinge;
      int spawn_height = bridge_height + 20;

      // may screw things up if second platform has a negative x coordinate
      int hinge_min = data[hinge_plat_pos][0] + data[hinge_plank_gap][0];
      int hinge_max = hinge_plat2_pos - hinge_plank_gap;

      if (frame == 0) {
        for (int i = 0; i < 3; i++) {
          // generate randomish positions above each bridge
          r_hinge = rand.get(hinge_min, hinge_max);
          // drop the balls
          create_ball(mat, vec3(r_hinge, rand.get(20, 30), data[hinge_plat_pos][2]), ball_color);
        }
      } else if (frame % 125 == 0) {
        int index = mesh_instances.size() - 1;
        for (int i = 0; i < 3; i++) {
          // generate randomish positions above each bridge
          r_hinge = rand.get(hinge_min, hinge_max);

          mesh_instance *ball = mesh_instances[index-i];
          ball->get_node()->set_linear_velocity(vec3(0, 0, 0));
          ball->get_node()->set_position(vec3(r_hinge, rand.get(20, 30), data[hinge_plat_pos][2]));
        }
      }

      frame++;
    }

    /// Loads bridge data from a csv file.
    void load_csv_data() {
      csv_parser parser;
      parser.vec4_locations_file("data.csv", data);

      // for debugging
      for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < 4; j++) {
          printf(" %f ", data[i][j]);
        }
        printf("\n");
      }
    }

    /// Calculate remaining shape mesh and rigid body indicies based on csv data
    void calculate_indices() {
      first_hinge_plank = last_hinge_platform + 1;
      last_hinge_plank = first_hinge_plank + data[plank_num][0] - 1;

      first_spring_platform = last_hinge_plank + 1;
      last_spring_platform = first_spring_platform + 1;

      first_spring_plank = last_spring_platform + 1;
      last_spring_plank = first_spring_plank + (data[plank_num][0] * 2) - 1;
    }

    /// Called to draw the world.
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices, assume 30fps
      app_scene->update(1.0f / 30);

      // draw the scene
      app_scene->render((float)vx / vy);

      spawn_balls();

      handle_collisions();
    }
  };
}