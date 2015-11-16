////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include "l_system_parser.h"

namespace octet {
  class tree_node {
    vec3 position;
    float angle;

  public:
    tree_node() {
      position = vec3(0, 0, 0);
      angle = 0.0f;
    }

    tree_node(vec3 node_position, float node_angle) {
      position = node_position;
      angle = node_angle;
    }

    vec3 &get_position() {
      return position;
    }

    float &get_angle() {
      return angle;
    }
  };

  class l_systems : public app {

    // scene for drawing box
    ref<visual_scene> app_scene;

    l_system_parser tree;

    dynarray<tree_node> stack;

    float line_length;

  public:
    /// this is called when we construct the class before everything is initialised.
    l_systems(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(2000);
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 40, 100));


      line_length = 1.0f;

      tree.read_initial_data("data1.csv");
      printf("\n");
      printf("\n");
      printf("\n");
      printf("%c", tree.get_axiom()[0]);
      printf("\n");

      draw_tree();
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      app_scene->begin_render(w, h);

      handle_input();

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)w / h);

      // Control camera.
      if (is_key_down(key_down))
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 0, 1));
      if (is_key_down(key_up))
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 0, -1));
      if (is_key_down(key_f1))
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(-1, 0, 0));
      if (is_key_down(key_f4))
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(1, 0, 0));
      if (is_key_down(key_f2))
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 1, 0));
      if (is_key_down(key_f3))
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, -1, 0));
    }

    /// Draw a line - position and angle is based on the tree node stack.
    vec3 &draw_line(const vec3 &position, const float angle) {
      material *color = new material(vec4(0, 1, 0, 1));

      vec3 mid_point = position;
      mid_point.x() = mid_point.x() + line_length * cos((angle + 90) * CL_M_PI / 180);
      mid_point.y() = mid_point.y() + line_length * sin((angle + 90) * CL_M_PI / 180);

      vec3 end_point = position;
      end_point.x() = end_point.x() + line_length * 2.0f * cos((angle + 90) * CL_M_PI / 180);
      end_point.y() = end_point.y() + line_length * 2.0f * sin((angle + 90) * CL_M_PI / 180);

      mat4t mat;
      mat.loadIdentity();
      mat.translate(mid_point);
      mat.rotate(angle, 0, 0, 1);

      mesh_box *line = new mesh_box(vec3(0.3f, 1.0f, 0.0f), mat);
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, line, color));

      return end_point;
    }

    void draw_tree() {
      dynarray<char> axiom = tree.get_axiom();
      float tree_angle = tree.get_angle();

      vec3 position = vec3(0, 0, 0);
      float angle = 0.0f;

      // Interpret each command in the axiom.
      for (unsigned int i = 0; i < axiom.size(); ++i) {
        switch (axiom[i]) {
          case 'F': {
            position = draw_line(position, angle);
            break;
          }
          case '+': {
            angle += tree_angle;
            break;
          }
          case '-': {
            angle -= tree_angle;
            break;
          }
          case '[': {
            tree_node node = tree_node(position, angle);
            stack.push_back(node);
            break;
          }
          case ']': {
            tree_node node = stack[stack.size() - 1];
            stack.pop_back();

            position = node.get_position();
            angle = node.get_angle();

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
      if (is_key_down(key_space)) {
        tree.next_iteration();
        draw_tree();
        // NEED TO REFRESH THE SCENE TO STOP MULTIPLE INSTANCES OF LINES.
      }
    }
  };
}
