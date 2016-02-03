// Terrain Generator.

#include "noise.h";
#include <math.h>;
#include <windows.h>;
#include <vector>;

namespace octet {
  /// Scene containing a box with octet.
  class terrain_generator : public app {
    // Scene for drawing box.
    ref<visual_scene> app_scene;

  public:
    terrain_generator(int argc, char **argv) : app(argc, argv) {
    }

    /// This is called once OpenGL is initialized.
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->set_far_plane(450);
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(grid_width * 0.5f, grid_height * 0.5f * 0.5f, 400));

      // Overlay text.
      aabb bb0(vec3(-210, 200, 0), vec3(160, 150, 0));
      aabb bb1(vec3(-220, -270, 0), vec3(150, 80, 0));
      overlay = new text_overlay();
      noise_information = new mesh_text(overlay->get_default_font(), "", &bb0);
      control_information = new mesh_text(overlay->get_default_font(), controls, &bb1);
      overlay->add_mesh_text(noise_information);
      overlay->add_mesh_text(control_information);

      // Materials.
      ground_mat = new material(vec4(1, 1, 1, 1));

      // Assign the vector arrays space.
      meshes = std::vector<std::vector<mesh_instance>>();
      height_line = std::vector<int>(grid_width, 0);
      luminance = std::vector<std::vector<int>>(grid_width, std::vector<int>(grid_width, 0));

      // Noise setup.
      noise.initialise_perms();
      
      // WINDOWS-ONLY code used to display noise maps in the console window - extremely inefficient.
      console = GetConsoleWindow();
      dc = GetDC(console);

      generate_terrain_grid();
      generate_height_line();
      generate_cave_noise();
     
      //draw_height_line(noise, height_line);
      //pixel_luminance(noise, height_line, luminance);

      // WINDOWS-ONLY code.
      //ReleaseDC(console, dc);
    }

    /// This is called to draw the world.
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // Update matrices. Assumes 30 fps.
      app_scene->update(1.0f/30);

      // Draw the scene.
      app_scene->render((float)vx / vy);

      handle_input();

      update_text(vx, vy);

      // Tumble the box (there is only one mesh instance).
      //scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      //node->rotate(1, vec3(1, 0, 0));
      //node->rotate(1, vec3(0, 1, 0));
    }

  private:
    // For drawing text.
    ref<text_overlay> overlay;
    ref<mesh_text> noise_information;
    ref<mesh_text> control_information;

    // For overlay display.
    string controls = "Controls:\n"
      "+/- Octaves     = F1/F2 or 1/2\n"
      "+/- Threshold   = F3/F4\n"
      "+/- Scale/Freq  = F5/F6 or 5/6\n"
      "+/- Persistence = F7/F8 or 7/8\n";

    // Materials.
    material *ground_mat;

    // Noise variables.
    noise noise;
    int iterations = 16;
    int luminance_threshold = 128;
    float scale = 0.08f;
    float persistence = 0.5f;
  
    // Increment variables.
    int iterations_inc = 1;
    int threshold_inc = 10;
    float scale_inc = 0.01f;
    float persistence_inc = 0.1f;

    // Terrain / noise grid dimensions.
    const int grid_height = 256;
    const int grid_width = 256;

    // Arrays for terrain data.
    std::vector<std::vector<mesh_instance>> meshes;
    std::vector<int> height_line;
    std::vector<std::vector<int>> luminance;

    // To store maximum height values for terrain.
    int max_height = 0;
    int min_height = grid_height;

    // WINDOWS-ONLY console variables.
    HWND console;
    HDC dc;

    /// Generates a grid of ground meshes with dimenions grid_width by grid_height.
    void generate_terrain_grid() {
      for (int i = 0; i < grid_width; i++) {
        for (int j = 0; j < grid_height; j++) {
          create_ground_tile(vec3(i, j, 0), ground_mat, false);
          //meshes.push_back.push_back(app_scene->get_mesh_instance(i));
        }
      }
    }

    /// Creates a 2D box mesh representing a ground tile.
    void create_ground_tile(vec3 location, material *color, bool is_dynamic) {
      mat4t mat;
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(vec3(0.5, 0.5, 0)), color, is_dynamic);
    }

    ///Draws the height line for the terrain in the windows console.
    void generate_height_line() {
      for (int i = 0; i < grid_width; i++) {
        height_line[i] = noise.fBM(iterations, 0, i, 0.5f, 0.007f, 0, 255);
        if (height_line[i] > max_height) {
          max_height = height_line[i];
        }
        else if (height_line[i] < min_height) {
          min_height = height_line[i];
        }
        // WINDOWS-ONLY.
        //draw_line_to_console();
      }
    }

    /// WINDOWS-ONLY function that draws the generated height line in the console window.
    void draw_line_to_console() {
      bool line_reached = false;
      for (int i = 0; i < grid_width; i++) {
        int red = 0;
        for (int j = 255 - min_height - 1; j > 255 - max_height; j--) {
          if (j == 255 - height_line[i] - 1) {
            SetPixel(dc, i + 255, j, RGB(255, 255, 255));
            break;
          }
        }
      }
    }

    /// Generate noise for each grid cell and determine if ground should appear there.
    void generate_cave_noise() {
      // Generate noise for each grid coordinate.
      bool height_reached = false;
      for (int i = 0; i < grid_height; i++) {
        for (int j = 0; j < grid_width; j++) {
          luminance[i][j] = noise.fBM(iterations, i, j, persistence, scale, 0, 255);
         
          scene_node *node = app_scene->get_mesh_instance(i * grid_width + j)->get_node();
          if (j <= height_line[i] && luminance[i][j] >= luminance_threshold) {
            // Ground is present!
            if (!node->get_enabled()) {
              node->set_enabled(true);
            }
          } else {
            // Remove ground block from render frustrum.
            if (node->get_enabled()) {
              node->set_enabled(false);
            }
          }
          // WINDOWS-ONLY.
          SetPixel(dc, j, i, RGB(luminance[i][j], luminance[i][j], luminance[i][j]));
        }
      }
    }

    /// Detect inputs for a number of actions on the terrain.
    void handle_input() {
      adjust_noise_variables();
    }

    void adjust_noise_variables() {
      bool adjustments = false;

      // Iterations.
      if (is_key_going_down(key_f1)) {
        iterations += iterations_inc;
        adjustments = true;
      } if (is_key_going_down(key_f2)) {
        iterations -= iterations_inc;
        adjustments = true;
      }

      // Luminance Threshold.
      if (is_key_going_down(key_f3)) {
        luminance_threshold += threshold_inc;
        adjustments = true;
      } if (is_key_going_down(key_f4)) {
        luminance_threshold -= threshold_inc;
        adjustments = true;
      }

      // Scale / Frequency.
      if (is_key_going_down(key_f5)) {
        scale += scale_inc;
        adjustments = true;
      } if (is_key_going_down(key_f6)) {
        scale -= scale_inc;
        adjustments = true;
      }

      // Persistence.
      if (is_key_going_down(key_f7)) {
        persistence += persistence_inc;
        adjustments = true;
      } if (is_key_going_down(key_f8)) {
        persistence -= persistence_inc;
        adjustments = true;
      }

      // Redraw for any changes.
      if (adjustments) {
        generate_cave_noise();
      }
    }

    /// Update the terrain information on the text overlay.
    void update_text(int vx, int vy)
    {
      noise_information->clear();

      // Write text.
      char buffer[5][256];
      sprintf(buffer[0], "%i", iterations);
      sprintf(buffer[1], "%i", luminance_threshold);
      sprintf(buffer[2], "%.2f", scale);
      sprintf(buffer[3], "%.2f", persistence);

      // Format the mesh.
      noise_information->format("Noise Information:\n"
                                "Octaves:               %s\n"
                                "Luminance Threshold:   %s\n"
                                "Scale / Frequency:     %s\n"
                                "Persistence:           %s\n",
        buffer[0],
        buffer[1],    
        buffer[2],
        buffer[3],
        buffer[4]);

      // Update the OpenGL geometry.
      noise_information->update();

      // Render the overlay.
      overlay->render(vx, vy);
    }
  };
}
