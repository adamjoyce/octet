////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include <math.h>;
#include <windows.h>;
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

      HWND console = GetConsoleWindow();
      HDC dc = GetDC(console);
  
      initliase_perms();

      float scale = 0.01f;
      for (int i = 0; i < grid_height; i++) {
        for (int j = 0; j < grid_width; j++) {
          luminance[j][i] = calculate_luminance(simplex_noise(j * scale, i * scale));
          SetPixel(dc, j, i, RGB(luminance[j][i], luminance[j][i], luminance[j][i]));
          //int luminance = calculate_luminance(simplex_noise(float(j), float(i)));
          //printf("%i ", luminance);
        }
        //printf("\n");
      }

      ReleaseDC(console, dc);
      //std::cin.ignore();
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
    static const int grid_height = 256;
    static const int grid_width = 256;
    int luminance[grid_height][grid_width];
 
    vec3 grad3[12] = {vec3(1,1,0), vec3(-1,1,0), vec3(1,-1,0), vec3(-1,-1,0),
                      vec3(1,0,1), vec3(-1,0,1), vec3(1,0,-1), vec3(-1,0,-1),
                      vec3(0,1,1), vec3(0,-1,1), vec3(0,1,-1), vec3(0,-1,-1)};

    // Permutation table.
    int p[256] = {151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233,
                  7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
                  190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219,
                  203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174,
                  20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27,
                  166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230,
                  220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25,
                  63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
                  200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173,
                  186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118,
                  126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
                  189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163,
                  70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19,
                  98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246,
                  97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162,
                  241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181,
                  199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150,
                  254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128,
                  195, 78, 66, 215, 61, 156, 180};

    // Double the permutation table length to remove the need for index wrapping.
    short perm[512] = {};
    short perm_mod12[512] = {};

    void initliase_perms() {
      for (int i = 0; i < 512; i++)
      {
        perm[i] = p[i & 255];
        perm_mod12[i] = (short)(perm[i] % 12);
      }
    }

    // Skewing and unsweing factors for two dimensions.
    const double F2 = 0.5f * (sqrt(3.0f) - 1.0f);
    const double G2 = (3.0f - sqrt(3.0f)) / 6.0f;

    // Fast floor function.
    int fast_floor(double x) {
      int xi = (int)x;
      return x > xi ? xi : xi - 1;
    }

    // Dot product for two dimensions.
    double dot(vec3 gradient, double x, double y) {
      return gradient[0] * x + gradient[1] * y;
    }

    // Two dimension Simplex Noise.
    double simplex_noise(double xin, double yin) {
      // Noise contributions from the three corners.
      double n0, n1, n2;

      // Skew the input grid to determine which cell the coordinates fall into.
      double s = (xin + yin) * F2;
      int i = fast_floor(xin + s);
      int j = fast_floor(yin + s);
      double t = (i + j) * G2;

      // Unskew the cell origin back to (x, y) space.
      double x_zero = i - t;
      double y_zero = j - t;
      // The x and y distances from the cell origin.
      double x0 = xin - x_zero;
      double y0 = yin - y_zero;

      // Determine which simplex the coordinates fall in.
      // Simplexes are equilateral triangles for the two dimension case.
      // Offsets for middle corner in (i, j).
      int i1, j1;
      if (x0 > y0) {
        // We are in the lower triangle.
        // Triangle reads (0,0) -> (1,0) -> (1,1).
        i1 = 1;
        j1 = 0;
      } else {
        // We are in the upper triangle.
        // Triangle reads (0,0) -> (0,1) -> (1,1).
        i1 = 0;
        j1 = 1;
      }

      // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
      // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
      // c = (3 - sqrt(3)) / 6.

      // Offsets for middle corner in (x, y) (unskewed) coordinates.
      double x1 = x0 - i1 + G2;
      double y1 = y0 - j1 + G2;
      // Offsets for last corner in (x, y) (unskewed) coordinates.
      double x2 = x0 - 1.0f + 2.0f * G2;
      double y2 = y0 - 1.0f + 2.0f * G2;

      // Calculate hashed gradient indices for the three gradient corners.
      int ii = i & 255;
      int jj = j & 255;
      int gi0 = perm_mod12[ii + perm[jj]];
      int gi1 = perm_mod12[ii + i1 + perm[jj + j1]];
      int gi2 = perm_mod12[ii + 1 + perm[jj + 1]];

      // Calculate the contribution from each of the three corners.
      double t0 = 0.5f - x0 * x0 - y0 * y0;
      if (t0 < 0) {
        n0 = 0.0f;
      } else {
        t0 *= t0;
        // x and y of grad3 used for two dimension gradient.
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
      }

      double t1 = 0.5 - x1 * x1 - y1 * y1;
      if (t1 < 0) {
        n1 = 0.0f;
      } else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
      }

      double t2 = 0.5 - x2 * x2 - y2 * y2;
      if (t2 < 0) n2 = 0.0f;
      else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
      }

      // Add contributions from the three corners to get the final noise value.
      // Scaled to fall in the range -1.0 - 1.0.
      return 70.0f * (n0 + n1 + n2);
    }

    // Calculate luminance.
    int calculate_luminance(float noise_value) {
      return (noise_value + 1) / 2.0f * 255.0f;
    }
  };
}
