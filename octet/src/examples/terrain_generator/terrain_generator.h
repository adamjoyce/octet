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

      material *red = new material(vec4(1, 0, 0, 1));
      mesh_box *box = new mesh_box(vec3(4));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, red));

      plot_divide(0, grid_height / 2, grid_width, grid_height / 2);
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
    const int grid_width = 10;
    const int grid_height = 10;

  private:
    /// Plot divide.
    void plot_divide(int x1, int x2, int y1, int y2) {
      int x_diff = x2 - x1;
      int y_diff = y2 - y1;
      int vlen = x_diff * x_diff + y_diff * y_diff;

      for (int i = 0; i < grid_height; i++) {
        int dy = i - y1;
        for (int j = 0; j < grid_width; j++) {
          int dx = j - x1;
          float dot_prod = dx * x_diff + dy * y_diff;
          dot_prod /= vlen;

          printf("%i", dot_prod);
        }
        printf("\n");
      }
    }
  };
}
