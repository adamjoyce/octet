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
      app_scene->get_camera_instance(0)->set_far_plane(1000);
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(grid_width * 0.5f, grid_height * 0.5f, 400));
      //app_scene->get_camera_instance(0)->get_node()->rotate(90, vec3(0, 1, 0));

      /*material *red = new material(vec4(1, 0, 0, 1));
      mesh_box *box = new mesh_box(vec3(4, 4, 0));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, red));*/

      noise noise;
      noise.initialise_perms();

      std::vector<std::vector<int>> luminance(grid_height, std::vector<int>(grid_width, 0));

      ground = new material(vec4(1, 0, 0, 1));
      under_ground = new material(vec4(0, 0, 1, 1));

      /*std::vector<int> height(grid_height, 0);


      for (int i = 0; i < grid_height; i++) {
        height[i] = noise.simplex_noise(i);
        printf("%i ", height[i]);
      }*/
      
      // Windows-only code used to inefficiently display noise maps in the console window.
      console = GetConsoleWindow();
      dc = GetDC(console);

      std::vector<int> height_line(grid_width, 0);
      draw_height_line(noise, height_line);

      // Populate the luminance values for the grid.
      pixel_luminance(noise, luminance);

      // Set up the height landscape array.
      /*random rand;
      int index = rand.get(0, grid_height);
      std::vector<int> height_line(grid_width, 0);
      for (int i = 0; i < luminance[index].size(); i++) {
        height_line[i] = luminance[index][i];
      }*/

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
    const float scale_factor = 0.09f;
    const int iterations = 16;
    const float persistense = 0.5f;

    // 
    int max_height = 0;
    int min_height = grid_height;

    //
    material *ground;
    material *under_ground;

    // Windows-only console variables.
    HWND console;
    HDC dc;
 
    // Determine the luminance for each pixel in the 2D grid (and draw them in Windows console).
    void pixel_luminance(noise &noise, std::vector<std::vector<int>> &luminance) {
      for (int i = 0; i < grid_height; i++) {
        for (int j = 0; j < grid_width; j++) {
          luminance[j][i] = noise.fBM(iterations, j, i, persistense, scale_factor, 0, 255);
          if (i < min_height && luminance[j][i] >= 115) {
            create_ground_tile(vec3(j, i, 0), ground, false);
          }
          // WINDOWS ONLY.
          SetPixel(dc, j, i, RGB(luminance[j][i], luminance[j][i], luminance[j][i]));
          //printf("%i ", luminance);
        }
        //printf("\n");
      }
    } 

    // WINDOWS ONLY.
    //Draws the height line for the terrain in the windows console.
    void draw_height_line(noise &noise, std::vector<int> &height_line) {
      for (int i = 0; i < grid_width; i++) {
        height_line[i] = noise.fBM(iterations, 0, i, 0.4f, 0.003f, 0, 255);
        if (height_line[i] > max_height) {
          max_height = height_line[i];
        } else if (height_line[i] < min_height) {
          min_height = height_line[i];
        }
      }

      //min_height = grid_height - min_height;
      //max_height = grid_height - max_height;
      //printf("%i , %i ", min_height, max_height);

      bool line_reached = false;
      for (int i = 0; i < grid_width; i++) {
        int red = 0;
        for (int j = min_height - 1; j < max_height; j++) {
          if (line_reached || j == height_line[i] - 1) {
            SetPixel(dc, i + 255, j, RGB(255, 255, 255));
            line_reached = true;
          }
          else {
            SetPixel(dc, i + 255, j, RGB(0, 0, 0));
            create_ground_tile(vec3(i, j, 0), ground, false);
            red = 1;
          }
          //printf("%i, %i, %i\n", i, j, red);
          red = 0;
        }
        line_reached = false;
      }
    }

    // Creates a 2D box representing a ground tile.
    void create_ground_tile(vec3 location, material *color, bool is_dynamic) {
      mat4t mat;
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(vec3(0.5, 0.5, 0)), color, is_dynamic);
    }
  };
}
