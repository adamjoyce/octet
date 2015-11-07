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

    // for bounce sound
    ALuint bounce;
    static const int num_sound_sources = 8;
    unsigned cur_source;
    ALuint sources[num_sound_sources];
    ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }

    //sound_system sound_sys;
    //FMOD::Sound *ball_sound;

    // second platform x coordinates
    int hinge_plat2_pos;
    int spring_plat2_pos;

    enum {
      // data array indices
      bridge_height = 0,

      plank_num,
      ball_num,
      slab_num,

      hinge_plank_gap,
      spring_plank_gap,

      hinge_plat_pos,
      spring_plat_pos,

      plank_size,
      platform_size,
      slab_size,

      // mesh and rigid body array indices that do not require csv data to calculate
      ground = 0,
      first_hinge_platform,
      last_hinge_platform,
    };

    // remaining indices for mesh and rigid body arrays based on csv data
    int first_hinge_plank, last_hinge_plank;
    int first_spring_platform, last_spring_platform;
    int first_spring_plank, last_spring_plank;
    int first_ball, last_ball;
    int first_slab, last_slab;

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

      // sounds
      bounce = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bang.wav");
      cur_source = 0;
      alGenSources(8, sources);
    }

    /// Add a shape's mesh_instance and rigid_body to the corresponding arrays.
    void add_to_arrays(int index) {
      mesh_instances.push_back(app_scene->get_mesh_instance(index));
      rigid_bodies.push_back(mesh_instances[index]->get_node()->get_rigid_body());
    }

    /// Creates a box in a certain size, color, and location.
    void create_box(vec3 box_size, mat4t mat, vec3 location, material *color, bool is_dynamic, int array_index) {
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(box_size), color, is_dynamic);
      add_to_arrays(array_index);
    }

    /// Assemble the hinge bridge.
    void create_hinge_bridge() {
      // place the first and last platforms
      mat4t mat;
      create_box(data[platform_size], mat, vec3(data[hinge_plat_pos][0], data[hinge_plat_pos][1], data[hinge_plat_pos][2]),
                 platform_color, false, first_hinge_platform);

      // calculate the second platform's x coordinate
      hinge_plat2_pos = data[hinge_plat_pos][0] + ((data[plank_num][0] + 1) * data[hinge_plank_gap][0]);
      create_box(data[platform_size], mat, vec3(hinge_plat2_pos, data[hinge_plat_pos][1], data[hinge_plat_pos][2]),
                 platform_color, false, last_hinge_platform);

      // place and hinge the first plank to the first platform
      vec3 plank_location = vec3(data[hinge_plat_pos][0] + data[hinge_plank_gap][0], 
                                 data[hinge_plat_pos][1] + (data[bridge_height][0] - 0.5f), data[hinge_plat_pos][2]);
      create_box(data[plank_size], mat, plank_location, hinge_plank_color, true, first_hinge_plank);

      btHingeConstraint *hinge = new btHingeConstraint(*rigid_bodies[first_hinge_platform], *rigid_bodies[first_hinge_plank],
                                                        btVector3(data[hinge_plank_gap][0] * 0.5f, 
                                                                  data[bridge_height][0] - (data[plank_size][1] * 0.5f), 0),
                                                        btVector3(data[hinge_plank_gap][0] * - 0.5f, 0, 0),
                                                        btVector3(0, 0, 1), btVector3(0, 0, 1), false);
      dynamics_world->addConstraint(hinge);

      // place and hinge the remaining planks
      int i = first_hinge_plank + 1;
      for (i; i <= last_hinge_plank; i++) {
        plank_location = vec3(plank_location[0] + data[hinge_plank_gap][0], plank_location[1], plank_location[2]); 
        create_box(data[plank_size], mat, plank_location, hinge_plank_color, true, i);

        hinge = new btHingeConstraint(*rigid_bodies[i-1], *rigid_bodies[i], btVector3(data[hinge_plank_gap][0] * 0.5f, 0, 0),
                                       btVector3(data[hinge_plank_gap][0] * -0.5f, 0, 0), btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        dynamics_world->addConstraint(hinge);

        // deal with the last plank to platform hinge
        if (i == last_hinge_plank) {
          hinge = new btHingeConstraint(*rigid_bodies[i], *rigid_bodies[last_hinge_platform], 
                                         btVector3(data[hinge_plank_gap][0] * 0.5f, 0, 0),
                                         btVector3(data[hinge_plank_gap][0] * -0.5f, data[bridge_height][0] - 0.5f, 0),
                                         btVector3(0, 0, 1), btVector3(0, 0, 1), false);
          dynamics_world->addConstraint(hinge);
        }
      }
    }

    /// Assemble spring bridge.
    /// Reference: http://bullet.googlecode.com/svn/trunk/Demos/ConstraintDemo/ConstraintDemo.cpp
    void create_spring_bridge() {
      // place the first and last platforms
      mat4t mat;
      create_box(data[platform_size], mat, vec3(data[spring_plat_pos][0], data[spring_plat_pos][1], data[spring_plat_pos][2]),
                 platform_color, false, first_spring_platform);
      // calculate the second platform's x coordinate
      spring_plat2_pos = data[spring_plat_pos][0] + ((data[plank_num][0] + 1) * data[spring_plank_gap][0]);
      create_box(data[platform_size], mat, vec3(spring_plat2_pos, data[spring_plat_pos][1], data[spring_plat_pos][2]),
                 platform_color, false, last_spring_platform);

      btTransform frame_in_a, frame_in_b;
      btGeneric6DofSpringConstraint *spring;
      vec3 plank_location = vec3(data[spring_plat_pos][0], data[spring_plat_pos][1], data[spring_plat_pos][2]);
      for (int i = 0; i <= last_spring_plank - first_spring_plank; i += 2) {
        // create the spring planks
        float new_x_location = plank_location[0] + data[spring_plank_gap][0];
        plank_location = vec3(new_x_location, plank_location[1], plank_location[2]);
        create_box(data[plank_size], mat, plank_location, spring_plank_color, true, first_spring_plank + i);

        // create the spring anchors
        create_box(data[plank_size], mat, vec3(plank_location[0], plank_location[1] + 30, plank_location[2]), spring_plank_color, false,
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
        spring->enableSpring(0, false);
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
          //ALuint source = get_sound_source();
          //play_sound(source);
          //sound_sys.play_sound(ball_sound, false);
        }
      }
    }

    /// Set up the bounce sound.
    void play_sound() {
      ALuint source = get_sound_source();
      // check if something is playing
      if (isPlaying(source)) {
        return;
      }

      alSourcei(source, AL_BUFFER, bounce);
      alSourcePlay(source);
    }

    bool isPlaying(ALuint source)
    {
      ALenum state;
      alGetSourcei(source, AL_SOURCE_STATE, &state);
      return (state == AL_PLAYING);
    }

    /// Issue with fmod in sound_system class that needs resolving.
    /*void sound_setup() {
      sound_sys = sound_system();
      sound_sys.create_sound(ball_sound, "ball_sound.mp3");
    }*/

    /// Create a ball.
    void create_ball(mat4t mat, vec3 location, material *color, int array_index) {
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_sphere(), ball_color, true);
      add_to_arrays(array_index);
    }

    /// Spawn ball and slab objects above the bridges.
    void spawn_objects() {
      mat4t mat;

      float random_x, random_y;
      int spawn_height = data[bridge_height][0] + (data[bridge_height][0] * 3);

      int hinge_min = data[hinge_plat_pos][0] + data[hinge_plank_gap][0];
      int hinge_max = hinge_plat2_pos - data[hinge_plank_gap][0];
      int spring_min = data[spring_plat_pos][0] + data[spring_plank_gap][0];
      int spring_max = spring_plat2_pos - data[spring_plank_gap][0];

      if (frame == 0) {
        // balls
        for (int i = 0; i < data[ball_num][0]; i++) {
          // generate randomish positions above the bridge
          random_x = rand.get(hinge_min, hinge_max);
          random_y = rand.get(spawn_height, spawn_height + 10);
          create_ball(mat, vec3(random_x, random_y, data[hinge_plat_pos][2]), ball_color, first_ball + i);
        }

        //slabs
        for (int i = 0; i < data[slab_num][0]; i++) {
          // generate randomish positions above the bridge
          random_x = rand.get(spring_min, spring_max);
          random_y = rand.get(spawn_height, spawn_height + 10);
          create_box(data[slab_size], mat, vec3(random_x, random_y, data[spring_plat_pos][2]), ball_color, true, first_slab + i);
        }
      } else if (frame % 125 == 0) {
        // balls
        for (int i = 0; i < data[ball_num][0]; i++) {
          // generate randomish positions above each bridge
          random_x = rand.get(hinge_min, hinge_max);
          random_y = rand.get(spawn_height, spawn_height + 10);

          mesh_instance *ball = mesh_instances[first_ball+i];
          ball->get_node()->set_linear_velocity(vec3(0, 0, 0));
          ball->get_node()->set_position(vec3(random_x, random_y, data[hinge_plat_pos][2]));
        }

        // slabs
        for (int i = 0; i < data[slab_num][0]; i++) {
          // generate randomish positions above each bridge
          random_x = rand.get(spring_min, spring_max);
          random_y = rand.get(spawn_height, spawn_height + 10);

          mesh_instance *slab = mesh_instances[first_slab+i];
          slab->get_node()->set_linear_velocity(vec3(0, 0, 0));
          slab->get_node()->set_position(vec3(random_x, random_y, data[spring_plat_pos][2]));
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

      first_ball = last_spring_plank + 1;
      last_ball = first_ball + data[ball_num][0] - 1;

      first_slab = last_ball + 1;
      last_slab = first_slab + data[slab_num][0] - 1;
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

      mat4t mat;
      if (is_key_down(key_space)) {
        create_box(data[slab_size], mat, vec3(data[spring_plat_pos][0] + 6, data[spring_plat_pos][1] + 20, data[spring_plat_pos][2]),
                   ball_color, true, mesh_instances.size());
      }

      spawn_objects();

      play_sound();

      handle_collisions();
    }
  };
}