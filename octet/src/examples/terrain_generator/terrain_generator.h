////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
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
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(grid_width * 0.5, grid_height * 0.5, 30));
      //app_scene->get_camera_instance(0)->get_node()->rotate(90, vec3(0, 1, 0));

      /*material *red = new material(vec4(1, 0, 0, 1));
      mesh_box *box = new mesh_box(vec3(4, 4, 0));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, red));*/

      //plot_divide(0, 0, grid_height, 0);
      //random_noise(200);
      //printf("%i", perlin_noise(0.5f));

      random rand;
      // Calculate random gradient vectors.
      for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
          float x = rand.get(-1.0f, 1.0f);
          float y = rand.get(-1.0f, 1.0f);
          gradient[j][i][0] = x;
          gradient[j][i][1] = y;
        }
      }

      for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
          float x = i / 0.4f;
          float y = j / 0.4f;
          float val = perlin_noise(x, y);
          printf("%f", val);
          printf("\t");
        }
      }
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

    // Variables.
    const int grid_width = 100;
    const int grid_height = 20;
    const float ground_threshold = 0.5f;

    float gradient[10][10][2];

  private:
    /// Plot divide.
    void plot_divide(int x1, int x2, int y1, int y2) {
      int x_diff = x2 - x1;
      int y_diff = y2 - y1;
      int vlen = x_diff * x_diff + y_diff * y_diff;
      int result = 0;

      for (int i = grid_height - 1; i >= 0; i--) {
        int dy = i - y1;
        for (int j = 0; j < grid_width; j++) {
          int dx = j - x1;
          float dot_prod = dx * x_diff + dy * y_diff;
          dot_prod /= vlen;

          result = select(dot_prod, ground_threshold);

          if (result == 1) {
            vec3 location = vec3(j, i, 0);
            material *red = new material(vec4(1, 0, 0, 1));
            create_ground(location, red, false);
          } else {
            vec3 location = vec3(j, i, 0);
            material *green = new material(vec4(0, 1, 0, 1));
            create_ground(location, green, false);
          }

          //printf("%i,%i", i, j);
          //printf("%i", result);
          //printf("\t");
        }
        //printf("\n");
      }
    }

    /// Step function to differentiate between ground and sky.
    int select(float value, float threshold) {
      if (value >= threshold) {
        return 1;
      } else {
        return 0;
      }
    }

    /// Create ground.
    void create_ground(vec3 location, material *color, bool is_dynamic) {
      mat4t mat;
      mat.loadIdentity();
      mat.translate(location);
      app_scene->add_shape(mat, new mesh_box(vec3(0.4, 0.4, 0)), color, is_dynamic);
    }

    /// Random noise generator.
    /*double random_noise(int seed) {
      seed = (seed << 13) ^ seed;
      return (double)(1.0 - ((seed * (seed * seed * 15731 + 789221) + 1376312589) & 0x7fffff) / 1073741824.0);
    }

    /// Smooth noise.
    double smooth_noise(double x) {
      return random_noise(x) * 0.5 + random_noise(x - 1) * 0.25 + random_noise(x + 1) * 0.25; 
    }

    /// Cosine interpolation.
    double cosine_interpolation(double v1, double v2, double x) {
      double a = x * 3.1415927;
      double b = (1 - cos(a)) * 0.5f;
      return  v1 * (1 - b) + v2 * b;
    }

    /// Interpolated noise.
    double interpolate_noise(double x) {
      int int_x = int(x);
      double fract_x = x - int_x;

      double v1 = smooth_noise(int_x);
      double v2 = smooth_noise(int_x + 1);

      return cosine_interpolation(v1, v2, fract_x);
    }

    /// Perlin noise.
    double perlin_noise(double x) {
      double total = 0.0f;
      float persistence = 0.5;
      int octaves = 5;
      
      for (int i = 0; i < octaves - 1; i++) {
        int frequency = 0;
        float amplitude = 0.0f;
        for (int j = 0; j < i; j++) {
          frequency *= 2;
          amplitude *= persistence;
        }

        total = total + interpolate_noise(x * frequency) * amplitude;
      }
      return total;
    }*/

    ////////////////////////////////
    // Linear interpolation between two points.
    // Ensure that w is between 0.0 and 1.0.
    float lerp(float a0, float a1, float w) {
      return (1.0 - w) * a0 + w * a1;
    }

    // Computes the dot product of distance and gradient vectors.
    float dot_grid_gradient(int ix, int iy, float x, float y) {      
      // Compute the distance vector.
      float dx = x - (float)ix;
      float dy = y - (float)iy;

      // Compute the dot product.
      return (dx * gradient[iy][ix][0] + dy * gradient[iy][ix][1]);
    }

    // Compute Perlin Noise at point (x, y).
    float perlin_noise(float x, float y) {
      // Determine cell grid coordinates.
      int x0 = (x > 0.0f ? (int)x : (int)x - 1);
      int x1 = x0 + 1;
      int y0 = (y > 0.0f ? (int)y : (int)y - 1);
      int y1 = y0 + 1;

      // Determine interpolation weights.
      float sx = x - (float)x0;
      float sy = y - (float)y0;

      // Interpolate between grid point gradients.
      float n0, n1, ix0, ix1, value;
      n0 = dot_grid_gradient(x0, y0, x, y);
      n1 = dot_grid_gradient(x1, y0, x, y);
      ix0 = lerp(n0, n1, sx);

      n0 = dot_grid_gradient(x0, y1, x, y);
      n1 = dot_grid_gradient(x1, y1, x, y);
      ix1 = lerp(n0, n1, sx);

      value = lerp(ix0, ix1, sy);

      return value;
    }
  };
}
