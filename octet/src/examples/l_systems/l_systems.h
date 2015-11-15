////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include "l_system_parser.h"

namespace octet {
  /// Scene containing a box with octet.
  class l_systems : public app {

    // scene for drawing box
    ref<visual_scene> app_scene;

    l_system_parser tree;

  public:
    /// this is called when we construct the class before everything is initialised.
    l_systems(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      tree.read_initial_data("data1.csv");
      printf("\n");
      printf("\n");
      printf("\n");
      printf("%c", tree.get_axiom()[0]);
      printf("\n");
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      handle_input();

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }

    void draw_axiom() {
      dynarray<char> axiom = tree.get_axiom();
      float tree_angle = tree.get_angle();

      vec3 pos = vec3(0, 0, 0);
      float angle = 0.0f;

      // Interrupt each command in the axiom.
      for (unsigned int i = 0; i < axiom.size(); ++i) {
        switch (axiom[i]) {
          case '+': {
            angle += tree_angle;
            break;
          }
          case '-': {
            angle -= tree_angle;
            break;
          }
          case '[': {
            // Push the position on the stack.
            break;
          }
          case ']': {
            // Pop the position from the stack.
            break;
          }
          case 'F': {
            // Draw a line.
            break;
          }
          default: {
            // Skip unevaluated progression values.
            continue;
          }
        }

      }
    }

    void handle_input() {
      if (is_key_down(key_up)) {
        tree.next_iteration();
        //draw_axiom();
      }
    }
  };
}
