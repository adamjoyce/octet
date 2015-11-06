////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason and Adam Joyce 2012-2015
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// invaderer example: simple game with sprites and sounds
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Simple game mechanics
//   Texture loaded from GIF file
//   Audio
//

// for std::ifstream
#include <fstream>
// for std::getline()
#include <string>
// for std::stringstream
#include <sstream>

namespace octet {
class sprite {
  // where is our sprite (overkill for a 2D game!)
  mat4t modelToWorld;
  // half the width of the sprite
  float halfWidth;
  // half the height of the sprite
  float halfHeight;
  // what texture is on our sprite
  int texture;
  // true if this sprite is enabled.
  bool enabled;

 public:
  sprite() {
    texture = 0;
    enabled = true;
  }

  void init(int _texture, float x, float y, float w, float h) {
    modelToWorld.loadIdentity();
    modelToWorld.translate(x, y, 0);
    halfWidth = w * 0.5f;
    halfHeight = h * 0.5f;
    texture = _texture;
    enabled = true;
  }

  void render(texture_shader &shader, mat4t &cameraToWorld, vec4 color = vec4(1, 1, 1, 1)) {
    // invisible sprite... used for gameplay.
    if (!texture) return;

    // build a projection matrix: model -> world -> camera -> projection
    // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
    mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

    // set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // use "old skool" rendering
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    shader.render(modelToProjection, 0, color);

    // this is an array of the positions of the corners of the sprite in 3D
    // a straight "float" here means this array is being generated here at runtime.
    float vertices[] = {
      -halfWidth, -halfHeight, 0,
       halfWidth, -halfHeight, 0,
       halfWidth,  halfHeight, 0,
      -halfWidth,  halfHeight, 0,
    };

    // attribute_pos (=0) is position of each corner
    // each corner has 3 floats (x, y, z)
    // there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
    glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)vertices );
    glEnableVertexAttribArray(attribute_pos);
    
    // this is an array of the positions of the corners of the texture in 2D
    static const float uvs[] = {
       0,  0,
       1,  0,
       1,  1,
       0,  1,
    };

    // attribute_uv is position in the texture of each corner
    // each corner (vertex) has 2 floats (x, y)
    // there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
    glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)uvs );
    glEnableVertexAttribArray(attribute_uv);
    
    // finally, draw the sprite (4 vertices)
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  void render(space_shader &shader, mat4t &cameraToWorld) {
    // build a projection matrix: model -> world -> camera -> projection
    // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
    mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

    shader.render(modelToProjection);

    // this is an array of the positions of the corners of the sprite in 3D
    // a straight "float" here means this array is being generated here at runtime.
    float vertices[] = {
      -halfWidth, -halfHeight, 0,
      halfWidth, -halfHeight, 0,
      halfWidth,  halfHeight, 0,
      -halfWidth,  halfHeight, 0,
    };

    glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
    glEnableVertexAttribArray(attribute_pos);

    // this is an array of the positions of the corners of the texture in 2D
    static const float uvs[] = {
      0,  0,
      1,  0,
      1,  1,
      0,  1,
    };

    // attribute_uv is position in the texture of each corner
    // each corner (vertex) has 2 floats (x, y)
    // there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
    glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
    glEnableVertexAttribArray(attribute_uv);

    // finally, draw the sprite (4 vertices)
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  // move the object
  void translate(float x, float y) {
    modelToWorld.translate(x, y, 0);
  }

  // position the object relative to another.
  void set_relative(sprite &rhs, float x, float y) {
    modelToWorld = rhs.modelToWorld;
    modelToWorld.translate(x, y, 0);
  }

  // return true if this sprite collides with another.
  // note the "const"s which say we do not modify either sprite
  bool collides_with(const sprite &rhs) const {
    float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
    float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

    // both distances have to be under the sum of the halfwidths
    // for a collision
    return (fabsf(dx) < halfWidth + rhs.halfWidth) &&
           (fabsf(dy) < halfHeight + rhs.halfHeight);
  }

  bool &is_enabled() {
    return enabled;
  }
};

class invaderers_app : public octet::app {
  // Matrix to transform points in our camera space to the world.
  // This lets us move our camera
  mat4t cameraToWorld;

  // shader to draw a textured triangle
  texture_shader texture_shader_;
  space_shader space_shader_;

  enum {
    num_sound_sources = 8,
    num_borders = 4,
    num_invaderers = 5,
    num_bombs = num_invaderers * 2,

    // sprite definitions
    background_sprite = 0,
    ship_sprite,
    game_over_sprite,

    first_invaderer_sprite,
    last_invaderer_sprite = first_invaderer_sprite + num_invaderers - 1,

    first_bomb_sprite,
    last_bomb_sprite = first_bomb_sprite + num_bombs - 1,

    first_border_sprite,
    last_border_sprite = first_border_sprite + num_borders - 1,

    num_sprites,
  };

  // timer for bombs
  int bombs_disabled;
  float bomb_speed;

  // accounting for bad guys
  int live_invaderers;
  int num_lives;

  // game state
  bool game_over;
  int score;

  // sounds
  ALuint whoosh;
  ALuint bang;
  unsigned cur_source;
  ALuint sources[num_sound_sources];

  // big array of sprites
  sprite sprites[num_sprites];

  // random number generator
  class random randomizer;

  // a texture for our text
  GLuint font_texture;

  // information for our text
  bitmap_font font;

  ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }
  
  // called when we hit an enemy
  void on_hit_invaderer() {
    ALuint source = get_sound_source();
    alSourcei(source, AL_BUFFER, bang);
    alSourcePlay(source);

    live_invaderers--;
    score++;

    if (live_invaderers == 0) {
      game_over = true;
      sprites[game_over_sprite].translate(-20, 0);
    }
  }

  // called when we are hit
  void on_hit_ship() {
    ALuint source = get_sound_source();
    alSourcei(source, AL_BUFFER, bang);
    alSourcePlay(source);
    
    if (--num_lives == 0) {
      game_over = true;
      sprites[game_over_sprite].translate(-20, 0);
    }
  }

  float ship_speed = 0.1f;
  // use the keyboard to move the ship
  void move_ship() {
    // down and up arrows
    if (is_key_down(key_down)) {
      sprites[ship_sprite].translate(0, -ship_speed);
      if (sprites[ship_sprite].collides_with(sprites[first_border_sprite+0])) {
        sprites[ship_sprite].translate(0, +ship_speed);
      }
    } else if (is_key_down(key_up)) {
      sprites[ship_sprite].translate(0, +ship_speed);
      if (sprites[ship_sprite].collides_with(sprites[first_border_sprite+1])) {
        sprites[ship_sprite].translate(0, -ship_speed);
      }
    }
    // left and right arrows
    if (is_key_down(key_left)) {
      sprites[ship_sprite].translate(-ship_speed, 0);
      if (sprites[ship_sprite].collides_with(sprites[first_border_sprite+2])) {
        sprites[ship_sprite].translate(+ship_speed, 0);
      }
    } else if (is_key_down(key_right)) {
      sprites[ship_sprite].translate(+ship_speed, 0);
      if (sprites[ship_sprite].collides_with(sprites[first_border_sprite+3])) {
        sprites[ship_sprite].translate(-ship_speed, 0);
      }
    }   

    // set bomb speed based on player movement
    if ((!is_key_down(key_left) && !is_key_down(key_right) && !is_key_down(key_up) && !is_key_down(key_down)) ||
        (sprites[ship_sprite].collides_with(sprites[first_border_sprite]) ||
        sprites[ship_sprite].collides_with(sprites[first_border_sprite+1]) || 
        sprites[ship_sprite].collides_with(sprites[first_border_sprite+2]) || 
        sprites[ship_sprite].collides_with(sprites[first_border_sprite+3]))) {
      bomb_speed = 0.05f;
    } else {
      bomb_speed = 0.2f;
    }
  }

  // pick an invaderer and fire a bomb
  void fire_bombs() {
    if (bombs_disabled) {
      --bombs_disabled;
    }
    else {
      // find an invaderer
      sprite &ship = sprites[ship_sprite];
      for (int j = randomizer.get(0, num_invaderers); j < num_invaderers; ++j) {
        sprite &invaderer = sprites[first_invaderer_sprite + j];
        if (invaderer.is_enabled()) {
          // find a bomb
          for (int i = 0; i != num_bombs; ++i) {
            if (!sprites[first_bomb_sprite + i].is_enabled()) {
              sprites[first_bomb_sprite + i].set_relative(invaderer, 0, 0);
              sprites[first_bomb_sprite + i].is_enabled() = true;
              bombs_disabled = 0;
              ALuint source = get_sound_source();
              alSourcei(source, AL_BUFFER, whoosh);
              alSourcePlay(source);
              return;
            }
          }
          return;
        }
      }
    }
  }

  // animate the bombs
  void move_bombs() {
    for (int i = 0; i != num_bombs; ++i) {
      float x = random_float(-bomb_speed, bomb_speed);
      float y = random_float(-bomb_speed, bomb_speed);
      sprite &bomb = sprites[first_bomb_sprite + i];
      if (bomb.is_enabled()) {
        bomb.translate(x, y);
        if (bomb.collides_with(sprites[ship_sprite])) {
          bomb.is_enabled() = false;
          bomb.translate(20, 0);
          bombs_disabled = 0;
          on_hit_ship();
          goto next_bomb;
        }
        if ((bomb.collides_with(sprites[first_border_sprite+0])) ||
            (bomb.collides_with(sprites[first_border_sprite+1])) ||
            (bomb.collides_with(sprites[first_border_sprite+2])) ||
            (bomb.collides_with(sprites[first_border_sprite+3]))) {
          bomb.is_enabled() = false;
          bomb.translate(20, 0);
        }
      }
    next_bomb:;
    }
  }

  // deal with collided invaderer and scale player
  void collide_invaderer() {
    for (int i = 0; i != num_invaderers; ++i) {
      sprite &invaderer = sprites[first_invaderer_sprite+i];
      if (invaderer.is_enabled() && sprites[ship_sprite].collides_with(invaderer)) {
        invaderer.is_enabled() = false;
        invaderer.translate(20, 0);
        on_hit_invaderer();
        //sprites[ship_sprite].scale(2.0f, 2.0f);
      }
    }
  }

  // random number generator
  float random_float(float min, float max) {
    float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float difference = max - min;
    float r = random * difference;
    return r + min;
  }

  void draw_text(texture_shader &shader, float x, float y, float scale, const char *text, vec4 color = vec4(1, 1, 1, 1)) {
    mat4t modelToWorld;
    modelToWorld.loadIdentity();
    modelToWorld.translate(x, y, 0);
    modelToWorld.scale(scale, scale, 1);
    mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

    /*mat4t tmp;
    glLoadIdentity();
    glTranslatef(x, y, 0);
    glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
    glScalef(scale, scale, 1);
    glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

    enum { max_quads = 32 };
    bitmap_font::vertex vertices[max_quads*4];
    uint32_t indices[max_quads*6];
    aabb bb(vec3(0, 0, 0), vec3(256, 256, 0));

    unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_texture);

    shader.render(modelToProjection, 0, color);

    glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x );
    glEnableVertexAttribArray(attribute_pos);
    glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u );
    glEnableVertexAttribArray(attribute_uv);

    glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
  }

 public:
   
  // this is called when we construct the class
  invaderers_app(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
  }
 
  /// Load position data from csv file.
  dynarray<vec4> load_csv_data() {
    std::ifstream file_stream("sprite_locations.csv");
    
    if (!file_stream) {
      std::string err = "Error loading csv file";
      printf(err.c_str());
    }

    dynarray<vec4> sprite_locations;
    std::string line = "";

    while (file_stream.good()) {
      std::getline(file_stream, line, '\n');

      // delimit each row with a comma
      line = line + ',';
      std::stringstream line_stream(line);

      float x, y, w, h;
      std::string value = "";

      while (line_stream.good()) {
        std::getline(line_stream, value, ',');

        if (isdigit(value[0]) || value[0] == '-') {
          // x coordinate
          x = (float)atof(value.c_str());
          printf(value.c_str());

          // y coordinate
          std::getline(line_stream, value, ',');
          y = (float)atof(value.c_str());
          printf(value.c_str());

          // width
          std::getline(line_stream, value, ',');
          w = (float)atof(value.c_str());
          printf(value.c_str());

          // height
          std::getline(line_stream, value, ',');
          h = (float)atof(value.c_str());
          printf(value.c_str());

          printf("\n");

          sprite_locations.push_back(vec4(x, y, w, h));
        }
      }
    }

    return sprite_locations;
  }

  /// Setup sprites.
  void sprite_setup() {
    dynarray<vec4> sprite_data = load_csv_data();

    // background
    // set the background with a blank texture
    sprites[background_sprite].init(NULL, sprite_data[background_sprite][0], sprite_data[background_sprite][1], 6, 6);

    // ship
    GLuint ship = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/ship.gif");
    sprites[ship_sprite].init(ship, sprite_data[ship_sprite][0], sprite_data[ship_sprite][1],
                              sprite_data[ship_sprite][2], sprite_data[ship_sprite][3]);

    // gameover
    GLuint GameOver = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameOver.gif");
    sprites[game_over_sprite].init(GameOver, sprite_data[game_over_sprite][0], sprite_data[game_over_sprite][1],
                                   sprite_data[game_over_sprite][2], sprite_data[game_over_sprite][3]);

    // invaderers
    GLuint invaderer = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/invaderer.gif");
    for (int i = 0; i != num_invaderers; ++i) {
      assert(first_invaderer_sprite + i <= last_invaderer_sprite);
      sprites[first_invaderer_sprite+i].init(invaderer, sprite_data[first_invaderer_sprite+i][0], sprite_data[first_invaderer_sprite+i][1],
                                             sprite_data[first_invaderer_sprite+i][2], sprite_data[first_invaderer_sprite+i][3]);
    }

    // bombs
    GLuint bomb = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/bomb.gif");
    for (int i = 0; i != num_bombs; ++i) {
      // create bombs off-screen
      sprites[first_bomb_sprite+i].init(bomb, sprite_data[first_bomb_sprite][0], sprite_data[first_bomb_sprite][1],
                                        sprite_data[first_bomb_sprite][2], sprite_data[first_bomb_sprite][3]);
      sprites[first_bomb_sprite+i].is_enabled() = false;
    }

    // borders
    int first_border_location = first_bomb_sprite + 1;
    // set the border to white for clarity
    GLuint white = resource_dict::get_texture_handle(GL_RGB, "#ffffff");
    sprites[first_border_sprite+0].init(white, sprite_data[first_border_location+0][0], sprite_data[first_border_location+0][1],
                                        sprite_data[first_border_location+0][2], sprite_data[first_border_location+0][3]);
    sprites[first_border_sprite+1].init(white, sprite_data[first_border_location+1][0], sprite_data[first_border_location+1][1],
                                        sprite_data[first_border_location+1][2], sprite_data[first_border_location+1][3]);
    sprites[first_border_sprite+2].init(white, sprite_data[first_border_location+2][0], sprite_data[first_border_location+2][1],
                                        sprite_data[first_border_location+2][2], sprite_data[first_border_location+2][3]);
    sprites[first_border_sprite+3].init(white, sprite_data[first_border_location+3][0], sprite_data[first_border_location+3][1],
                                        sprite_data[first_border_location+3][2], sprite_data[first_border_location+3][3]);
  }

  // this is called once OpenGL is initialized
  void app_init() {
    // set up the shader
    texture_shader_.init();
    space_shader_.init();

    // set up the matrices with a camera 5 units from the origin
    cameraToWorld.loadIdentity();
    cameraToWorld.translate(0, 0, 3);

    font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

    sprite_setup();

    // sounds
    whoosh = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/whoosh.wav");
    bang = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bang.wav");
    cur_source = 0;
    alGenSources(num_sound_sources, sources);

    // sundry counters and game state.
    bombs_disabled = 10;
    bomb_speed = 0.01f;
    live_invaderers = num_invaderers;
    num_lives = 3;
    game_over = false;
    score = 0;
  }

  // called every frame to move things
  void simulate() {
    if (game_over) {
      if (is_key_down(key_space)) {
        app_init();
      }
      return;
    }

    move_ship();

    fire_bombs();

    move_bombs();
    
    collide_invaderer();
  }

  /// Draw spites.
  void draw_sprites() {
    sprites[background_sprite].render(space_shader_, cameraToWorld);

    sprites[ship_sprite].render(texture_shader_, cameraToWorld, vec4(1, 1, 0, 1));

    sprites[game_over_sprite].render(texture_shader_, cameraToWorld);

    // invaderers
    for (int i = first_invaderer_sprite; i <= last_invaderer_sprite; i++) {
      sprites[i].render(texture_shader_, cameraToWorld, vec4(0.05f, 0.45f, 0.03f, 1));
    }

    // bombs
    vec4 bomb_color;
    if (bomb_speed == 0.2f) {
      bomb_color = vec4(1, 0, 0, 1);
    }
    else {
      bomb_color = vec4(1, 0.68f, 0.68f, 1);
    }

    for (int i = first_bomb_sprite; i <= last_bomb_sprite; i++) {
      sprites[i].render(texture_shader_, cameraToWorld, bomb_color);
    }

    // borders
    for (int i = first_border_sprite; i <= last_border_sprite; i++) {
      sprites[i].render(texture_shader_, cameraToWorld);
    }

  }

  // this is called to draw the world
  void draw_world(int x, int y, int w, int h) {
    simulate();

    // set a viewport - includes whole window area
    glViewport(x, y, w, h);

    // clear the background to black
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
    glDisable(GL_DEPTH_TEST);

    // allow alpha blend (transparency when alpha channel is 0)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    draw_sprites();

    char score_text[32];
    sprintf(score_text, "score: %d   lives: %d\n", score, num_lives);
    draw_text(texture_shader_, -1.75f, 2, 1.0f/256, score_text);

    // move the listener with the camera
    vec4 &cpos = cameraToWorld.w();
    alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
  }
};
}
