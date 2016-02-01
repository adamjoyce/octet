// Terrain Generator.

#include "noise.h";
#include <math.h>;
#include <windows.h>;
#include <vector>;

namespace octet {
  /// Scene containing a box with octet.
  class terrain_generator : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

  public:
    terrain_generator(int argc, char **argv) : app(argc, argv) {
    /// This is called when we construct the class before everything is initialised.
    }

    /// This is called once OpenGL is initialized.
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      //app_scene->get_camera_instance(0)->get_node()->translate(vec3(grid_width * 0.5, grid_height * 0.5, 30));
      //app_scene->get_camera_instance(0)->get_node()->rotate(90, vec3(0, 1, 0));

      /*material *red = new material(vec4(1, 0, 0, 1));
      mesh_box *box = new mesh_box(vec3(4, 4, 0));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, red));*/

      noise noise;
      noise.initialise_perms();

      std::vector<std::vector<int>> luminance(grid_height, std::vector<int>(grid_width, 0));
      
      // Windows-only code used to inefficiently display noise maps in the console window.
      console = GetConsoleWindow();
      dc = GetDC(console);

      // Populate the luminance values for the grid.
      pixel_luminance(noise, luminance);

      // Set up the height landscape array.
      random rand;
      int index = rand.get(0, grid_height);
      std::vector<int> height_line(grid_width, 0);
      for (int i = 0; i < luminance[index].size(); i++) {
        height_line[i] = luminance[index][i];
      }

      draw_height_line(height_line);

      ReleaseDC(console, dc);
    }

    /// This is called to draw the world.
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // Update matrices. Assumes 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

      // Tumble the box (there is only one mesh instance).
      //scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      //node->rotate(1, vec3(1, 0, 0));
      //node->rotate(1, vec3(0, 1, 0));
    }

  private:
    // Private variables.
    const int grid_height = 256;
    const int grid_width = 256;

    // Noise variables.
    const float scale_factor = 0.01f;
    const int iterations = 16;
    const float persistense = 0.4f;

    // Windows-only console variables.
    HWND console;
    HDC dc;
 
    // Determine the luminance for each pixel in the grid (and draw them in Windows consoles).
    void pixel_luminance(noise &noise, std::vector<std::vector<int>> &luminance) {
      for (int i = grid_height - 1; i >= 0; i--) {
        for (int j = 0; j < grid_width; j++) {
          luminance[j][i] = noise.fBM(iterations, j, i, persistense, scale_factor, 0, 255);
          // WINDOWS ONLY.
          SetPixel(dc, j, i, RGB(luminance[j][i], luminance[j][i], luminance[j][i]));
          //printf("%i ", luminance);
        }
        //printf("\n");
      }
    }

    // WINDOWS ONLY.
    //Draws the height line for the terrain in the windows console.
    void draw_height_line(const std::vector<int> &height_line) {
      for (int i = 0; i < grid_width; i++) {
        for (int j = grid_height - 1; j >= 0; j--) {
          if (j == height_line[i]) {
            SetPixel(dc, i + 255, j, RGB(255, 255, 255));
          }
          else {
            SetPixel(dc, i + 255, j, RGB(0, 0, 0));
          }
        }
      }
    }
  };
}
