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
    /// This is called when we construct the class before everything is initialised.
    terrain_generator(int argc, char **argv) : app(argc, argv) {
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
      random_noise(200);
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
    float random_noise(int seed) {
      seed = (seed << 13) ^ seed;
      return (1.0 - ((seed * (seed * seed * 15731 + 789221) + 1376312589) & 0x7fffff) / 1073741824.0);
    }

    /// Smooth noise.
    float smooth_noise(float x) {
      return random_noise(x) * 0.5 + random_noise(x - 1) * 0.25 + random_noise(x + 1) * 0.25; 
    }

    /// Cosine interpolation.
    float cosine_interpolation(float v1, float v2, float x) {
      float a = x * 3.1415927;
      float b = (1 - cos(a)) * 0.5f;
      return  v1 * (1 - b) + v2 * b;
    }

    /// Interpolated noise.
    float interpolate_noise(float x) {
      int int_x = int(x);
      float fract_x = x - int_x;

      float v1 = smooth_noise(int_x);
      float v2 = smooth_noise(int_x + 1);

      return cosine_interpolation(v1, v2, fract_x);
    }

    /// Perlin noise.
    float perlin_noise(float x) {
      float total = 0.0f;
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
    }
  };
}
