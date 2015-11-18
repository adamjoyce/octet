////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include "l_system_generator.h"

namespace octet {
  class tree_node {
    vec3 position;
    float angle;

  public:
    tree_node() {
      position = vec3(0, 0, 0);
      angle = 0.0f;
    }

    tree_node(vec3 &node_position, float node_angle) {
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

    // Scene for drawing.
    ref<visual_scene> app_scene;

    // Reads the csv files.
    l_system_generator tree;

    // Records node position and angle.
    dynarray<tree_node> stack;

    // For drawing text.
    ref<text_overlay> overlay;
    ref<mesh_text> l_system_information;
    ref<mesh_text> control_information;

    // Constants.
    const float LINE_INCREMENT = 0.2f;
    const float ANGLE_INCREMENT = 10.0f;
    const int CAMERA_INCREMENTS = 10;

    const float DEFAULT_LINE_LENGTH = 1.0f;
    const float DEFAULT_LINE_WIDTH = 0.4f;

    const float MIN_LINE_LENGTH = LINE_INCREMENT;
    const float MAX_LINE_LENGTH = 2.0f;

    const float MIN_LINE_WIDTH = LINE_INCREMENT;
    const float MAX_LINE_WIDTH = 1.0f;

    const int MIN_ANGLE = 0;
    const int MAX_ANGLE = 100;

    const int FAR_PLANE = 20000;

    // L-system node lines.
    float line_length, line_width;

    // Camera coordinates.
    int camera_x, camera_y, camera_z;

    // The current system's details.
    unsigned int current_file;
    unsigned int current_iteration;
    float current_angle;

    // Material colors for lines.
    material *stem, *leaf, *current_color;

  public:
    /// Called when we construct the class before everything is initialised.
    l_systems(int argc, char **argv) : app(argc, argv) {
    }

    /// Called once OpenGL is initialized
    void app_init() {
      camera_x = 0;
      camera_y = 250;
      camera_z = 620;

      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(FAR_PLANE);
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(camera_x, camera_y, camera_z));

      tree.read_data("data1.csv");

      // For overlay display.
      string controls = "Controls:\n"
        "Load Files      = F1 - F8\n"
        "+/- Iterate     = SPACE/BCKSPACE\n"
        "+/- Angle       = TAB/CTRL\n"
        "+/- Line Width  = INS/DEL\n"
        "+/- Line Length = ESC/ALT\n"
        "+/- Zoom        = RIGHT/LEFT ARROW\n"
        "Move Up/Down    = UP/DOWN ARROW\n";

      // Overlay text.
      aabb bb0(vec3(-250, 290, 0), vec3(120, 50, 0));
      aabb bb1(vec3(-220, -270, 0), vec3(150, 80, 0));
      overlay = new text_overlay();
      l_system_information = new mesh_text(overlay->get_default_font(), "", &bb0);
      control_information = new mesh_text(overlay->get_default_font(), controls, &bb1);
      overlay->add_mesh_text(l_system_information);
      overlay->add_mesh_text(control_information);

      line_length = DEFAULT_LINE_LENGTH;
      line_width = DEFAULT_LINE_WIDTH;

      current_file = 1;
      current_iteration = 0;
      current_angle = tree.get_angle();

      stem = new material(vec4(0.55f, 0.27f, 0.07f, 1));
      leaf = new material(vec4(0.23f, 0.37f, 0.04f, 1));
      current_color = stem;

      // Evaluate the initial axiom.
      parse_axiom();
    }

    /// Called to draw the world.
    void draw_world(int x, int y, int w, int h) {

      int vx = 0, vy = 0;

      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // Update matrices - assuming 30 fps.
      app_scene->update(1.0f/30);

      // Draw the scene.
      app_scene->render((float)vx / vy);

      handle_input();

      update_text(vx, vy);
    }

  private:
    /// Interpret the current axiom of the l-system model and update the scene.
    void parse_axiom() {
      dynarray<char> axiom = tree.get_axiom();

      vec3 position = vec3(0, 0, 0);
      float angle = 0.0f;

      // Interpret each command in the axiom.
      for (unsigned int i = 0; i < axiom.size(); ++i) {
        switch (axiom[i]) {
          // Draw a line.
          case 'F': {
            current_color = stem;

            // Apply the correct material color.
            for (unsigned int j = i + 1; j < axiom.size(); ++j) {
              if (axiom[j] == ']') {
                if (j + 1 != axiom.size()) {
                  current_color = leaf;
                }
              }
              else if (axiom[j] == 'F') {
                break;
              }
            }

            position = draw_line(position, angle);
            break;
          }
          // Turn left by angle.
          case '+': {
            angle += current_angle;
            break;
          }
          // Turn right by angle.
          case '-': {
            angle -= current_angle;
            break;
          }
          // Push angle and position on the node stack.
          case '[': {
            tree_node node = tree_node(position, angle);
            stack.push_back(node);
            break;
          }
          // Pop angle and position from the node stack.
          case ']': {
            tree_node node = stack[stack.size() - 1];
            stack.pop_back();

            position = node.get_position();
            angle = node.get_angle();

            break;
          }
          // Skip unevaluated progression values.
          default: {
            continue;
          }
        }
      }
    }

    /// Draw a line given the node position and angle.
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
      app_scene->add_mesh_instance(new mesh_instance(node, line, current_color));

      return end_point;
    }

    /// Detect inputs for a number of actions on the l-system.
    void handle_input() {
      iterate();

      camera_controls();

      load_csv_file();

      vary_angle();

      vary_line_width();

      vary_line_length();
    }

    /// Detects input to iterate the l-system model.
    void iterate() {
      if (is_key_going_down(key_space)) {
        if (current_iteration < tree.get_max_iterations()) {
          tree.next_iteration();
          update_scene();
          current_iteration++;
        }
      }
      else if (is_key_going_down(key_backspace)) {
        if (current_iteration > 0) {
          tree.previous_iteration();
          update_scene();
          current_iteration--;
        }
      }
    }

    /// Detects input for controlling the camera.
    void camera_controls() {
      if (is_key_down(key_up)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, CAMERA_INCREMENTS, 0));
        camera_y += CAMERA_INCREMENTS;
      }

      if (is_key_down(key_down)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, -CAMERA_INCREMENTS, 0));
        camera_y -= CAMERA_INCREMENTS;
      }

      if (is_key_down(key_left)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 0, CAMERA_INCREMENTS));
        camera_z += CAMERA_INCREMENTS;
      }

      if (is_key_down(key_right)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 0, -CAMERA_INCREMENTS));
        camera_z -= CAMERA_INCREMENTS;
      }
    }

    /// Detects input for loading another csv file.
    void load_csv_file() {
      if (is_key_going_down(key_f1)) {
        switch_tree("data1.csv", 0.4f, 250, 620);
        current_file = 1;
      }
      else if (is_key_going_down(key_f2)) {
        switch_tree("data2.csv", 0.2f, 60, 130);
        current_file = 2;
      }
      else if (is_key_going_down(key_f3)) {
        switch_tree("data3.csv", 0.1f, 60, 120);
        current_file = 3;
      }
      else if (is_key_going_down(key_f4)) {
        switch_tree("data4.csv", 0.4f, 250, 620);
        current_file = 4;
      }
      else if (is_key_going_down(key_f5)) {
        switch_tree("data5.csv", 0.4f, 250, 620);
        current_file = 5;
      }
      else if (is_key_going_down(key_f6)) {
        switch_tree("data6.csv", 0.2f, 80, 180);
        current_file = 6;
      }
      else if (is_key_going_down(key_f7)) {
        switch_tree("data7.csv", 0.1f, 15, 50);
        current_file = 7;
      }
      else if (is_key_going_down(key_f8)) {
        // Camera coordinate parameters order: y, z, x.
        switch_tree("data8.csv", 0.1f, 30, 80, -30);
        current_file = 8;
      }
    }

    /// Detects input for varying the angle of the l-system model.
    void vary_angle() {
      float new_angle;

      if (is_key_going_down(key_tab)) {
        new_angle = current_angle + ANGLE_INCREMENT;

        if (new_angle <= MAX_ANGLE) {
          current_angle = new_angle;
          update_scene();
        }
      } else if (is_key_going_down(key_ctrl)) {
        new_angle = current_angle - ANGLE_INCREMENT;

        if (new_angle >= MIN_ANGLE) {
          current_angle = new_angle;
          update_scene();
        }
      }
    }

    /// Detects input for varying the line width of the l-system model.
    void vary_line_width() {
      float new_width;

      if (is_key_going_down(key_insert)) {
        new_width = line_width + LINE_INCREMENT;

        if (new_width <= MAX_LINE_WIDTH) {
          line_width = new_width;
          update_scene();
        }
      } else if (is_key_going_down(key_delete)) {
        new_width = line_width - LINE_INCREMENT;

        if (new_width >= MIN_LINE_WIDTH) {
          line_width = new_width;
          update_scene();
        }
      }
    }

    /// Detects input for varying the line length of the l-system model.
    void vary_line_length() {
      float new_length;

      if (is_key_going_down(key_f9)) {
        new_length = line_length + LINE_INCREMENT;

        if (new_length <= MAX_LINE_LENGTH) {
          line_length = new_length;
          update_scene();
        }
      } else if (is_key_going_down(key_esc)) {
        new_length = line_length - LINE_INCREMENT;

        if (new_length >= MIN_LINE_LENGTH) {
          line_length = new_length;
          update_scene();
        }
      }
    }

    /// Called to refresh the scene with the altered l-system model.
    void update_scene() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(FAR_PLANE);
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(camera_x, camera_y, camera_z));

      parse_axiom();
    }    

    /// Switch / reset the current tree.
    void switch_tree(const std::string &csv_path, const float &line_width_, const int &camera_y_, const int &camera_z_,
                     const int &camera_x_ = 0) {
      tree.reset();
      tree.read_data(csv_path);

      line_width = line_width_;
      camera_x = camera_x_;
      camera_y = camera_y_;
      camera_z = camera_z_;

      update_scene();

      current_iteration = 0;
      current_angle = tree.get_angle();
    }

    /// Update the l-system information on the text overlay.
    void update_text(int vx, int vy)
    {
      l_system_information->clear();

      // Write text.
      char buffer[5][256];
      sprintf(buffer[0], "%d", current_file);
      sprintf(buffer[1], "%d", current_iteration);
      sprintf(buffer[2], "%.2f", current_angle);
      sprintf(buffer[3], "%.2f", line_width);
      sprintf(buffer[4], "%.2f", line_length);

      // Format the mesh.
      l_system_information->format("Current File:          %s\n"
                                   "Current Iteration:     %s\n"
                                   "Current Angle:         %s\n"
                                   "Current Line Width:    %s\n"
                                   "Current Line Length:   %s\n",
                                   buffer[0],
                                   buffer[1],
                                   buffer[2],
                                   buffer[3],
                                   buffer[4]);

      // Update the OpenGL geometry.
      l_system_information->update();

      // Render the overlay.
      overlay->render(vx, vy);
    }
  };
}
