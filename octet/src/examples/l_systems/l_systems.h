// L Systems
// Author: Adam Joyce
// Version: 1.04

namespace octet {
  class l_systems : public app {
    // Scene for drawing.
    ref<visual_scene> app_scene;

  public:
    /// Constructor.
    l_systems(int argc, char **argv) : app(argc, argv) {
    }

    /// Destructor.
    ~l_systems() {

    }

    /// Initiliases our app - called once OpenGL is initliased.
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 20 ,0));
    }

    /// Draws the world.
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // Update matrices - assumes 30 fps.
      app_scene->update(1.0f / 30);

      // Draw the scene.
      app_scene->render((float)vx / vy);
    }
  };
}