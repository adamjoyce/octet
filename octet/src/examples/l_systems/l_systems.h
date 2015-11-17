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
    float line_width;
    float line_increments;

    unsigned int far_plane_distance;
    int camera_y, camera_z, camera_increments;

    unsigned int current_iteration;

    material *color;

  public:
    /// this is called when we construct the class before everything is initialised.
    l_systems(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      far_plane_distance = 20000;
      camera_y = 240;
      camera_z = 600;
      camera_increments = 10;

      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(far_plane_distance);
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, camera_y, camera_z));

      line_length = 1.0f;
      line_width = 0.4f;
      line_increments = 0.2f;

      color = new material(vec4(0, 1, 0, 1));

      tree.read_data("data1.csv");

      printf("\n");
      printf("\n");
      printf("\n");
      printf("%c", tree.get_axiom()[0]);
      printf("\n");

      parse_axiom();
      current_iteration = 0;
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      app_scene->begin_render(w, h);

      handle_input();

      // update matrices. assume 30 fps.
      //app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)w / h);

      printf(" %i, %i ", camera_y, camera_z);
    }

    void update_scene() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(far_plane_distance);
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, camera_y, camera_z));
      parse_axiom();
    }

    /// Draw a line - position and angle is based on the tree node stack.
    vec3 &draw_line(const vec3 &position, const float angle) {
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

      mesh_box *line = new mesh_box(vec3(line_width, line_length, 0.0f), mat);
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, line, color));

      return end_point;
    }

    void parse_axiom() {
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
      current_iteration++;
    }

    void handle_input() {
      if (is_key_down(key_space)) {
        if (current_iteration < tree.get_max_iterations()) {
          tree.next_iteration();
          update_scene();
        } else {
          // maximum number of iteration reached for that tree
        }
      }

      // Camera control.
      if (is_key_down(key_up)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, camera_increments, 0));
        camera_y += camera_increments;
      }
      if (is_key_down(key_down)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, -camera_increments, 0));
        camera_y -= camera_increments;
      }
      if (is_key_down(key_left)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 0, camera_increments));
        camera_z += camera_increments;
      }
      if (is_key_down(key_right)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 0, -camera_increments));
        camera_z -= camera_increments;
      }

      // Load a different csv file.
      if (is_key_down(key_f1)) {
        switch_tree("data1.csv", 0.4f, 250, 620);
      } else if (is_key_down(key_f2)) {
        switch_tree("data2.csv", 0.2f, 60, 130);
      } else if (is_key_down(key_f3)) {
        switch_tree("data3.csv", 0.1f, 60, 120);
      } else if (is_key_down(key_f4)) {
        switch_tree("data4.csv", 0.4f, 250, 620);
      } else if (is_key_down(key_f5)) {
        switch_tree("data5.csv", 0.4f, 250, 620);
      } else if (is_key_down(key_f6)) {
        switch_tree("data6.csv", 0.2f, 80, 180);
      } else if (is_key_down(key_f7)) {
        switch_tree("data7.csv", 0.1f, 15, 50);
      }

      // Line dimensions.
      /*if (is_key_down(key_tab)) {
        line_length += line_increments;
      }
      else if (is_key_down(key_shift)) {
        line_length -= line_increments;
      }

      if (is_key_down(key_ctrl)) {
        line_width += line_increments;
      }
      else if (is_key_down(key_alt)) {
        line_width -= line_increments;
      }*/
    }

    /// Switch / reset the current tree.
    void switch_tree(const std::string &csv_path, const float &line_width_, const int &camera_y_, const int &camera_z_) {
      tree.reset();
      tree.read_data(csv_path);
      line_width = line_width_;
      camera_y = camera_y_;
      camera_z = camera_z_;
      update_scene();
      current_iteration = 0;
    }
  };
}
