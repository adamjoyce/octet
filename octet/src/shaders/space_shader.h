// Simple space background shader
// Author: Adam Joyce
// Version 1.02

namespace octet {
  namespace shaders {
    class space_shader : public shader {
      // indices to use with glUniform*()
      // index for model space to projection space matrix
      GLuint modelToProjectionIndex_;

    public:
      void init() {
        // this is the vertex shader. main() will be called for every vertex
        // of every triangle. The output is gl_Position which is used to generate
        // 2D triangles in the rasterizer.
        const char vertex_shader[] = SHADER_STR(
          attribute vec4 pos;
          uniform mat4 modelToProjection;
          void main() { 
            gl_Position = modelToProjection * pos; 
          }
        );

        // this is the fragment shader. It is called once for every pixel
        // in the rasterized triangles.
        const char fragment_shader[] = SHADER_STR(
          // maths reference: https://www.shadertoy.com/view/4lSSRw
          void main() { 
            vec2 coord = gl_FragCoord.xy;
            vec3 maths = vec3(max((fract(dot(sin(coord), coord)) - 0.99f) * 90, 0));
            gl_FragColor = vec4(maths, 1);
          }
        );

        // compile and link the shaders
        shader::init(vertex_shader, fragment_shader);

        // set up handles to access the uniforms.
        modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
      }

      // start drawing with this shader
      void render(const mat4t &modelToProjection) {
        // start using the program
        shader::render();

        // set the uniforms.
        glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());

        // now we are ready to define the attributes and draw the triangles.
      }
    };
  }
}