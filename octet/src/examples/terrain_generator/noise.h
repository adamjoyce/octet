// Noise class.

#include <math.h>;

namespace octet {
  class noise {
  public:
    noise() {
    }

    ~noise() {
    }

    // Fractional Brownian Motion using 1D Simplex Noise.
    /*float fBM(int iterations, float x_in, float persistence, float scale, int low, int high) {
      float amplitude = 1.0f;
      float current_amplitude = 0.0f;
      float frequency = scale;
      float noise = 0.0f;

      // Calculate and add the octaves together.
      for (int i = 0; i < iterations; i++) {
        noise += simplex_noise(x_in * frequency) * amplitude;
        current_amplitude += amplitude;
        amplitude *= persistence;
        frequency *= 2;
      }

      // Take the average of the summed iterations.
      noise /= current_amplitude;

      // Normalise the result.
      noise = noise * (high - low) / 2 + (high + low) / 2;

      return noise;
    }*/

    // Fractional Brownian Motion using 2D Simplex Noise.
    float fBM(int iterations, float x_in, float y_in, float persistence, float scale, int low, int high) {
      float amplitude = 1.0f;
      float current_amplitude = 0.0f;
      float frequency = scale;
      float noise = 0.0f;

      // Calculate and add the octaves together.
      for (int i = 0; i < iterations; i++) {
        noise += simplex_noise(x_in * frequency, y_in * frequency) * amplitude;
        current_amplitude += amplitude;
        amplitude *= persistence;
        frequency *= 2;
      }

      // Take the average of the summed iterations.
      noise /= current_amplitude;

      // Normalise the result.
      noise = noise * (high - low) / 2 + (high + low) / 2;

      return noise;
    }

    // Initiliase perms.
    void initialise_perms() {
      for (int i = 0; i < 512; i++)
      {
        perm[i] = p[i & 255];
        perm_mod12[i] = (short)(perm[i] % 12);
        //perm_mod16[i] = (short)(perm[i] % 16);
      }
    }

  private:
    /*const float grad1[16] = {-8.f, -7.f, -6.f, -5.f, -4.f, -3.f, -2.f,
                              -1.f, 1.f,  2.f,  3.f,  4.f,  5.f,  6.f,
                              7.f,  8.f};*/

    vec3 grad3[12] = { vec3(1,1,0), vec3(-1,1,0), vec3(1,-1,0), vec3(-1,-1,0),
      vec3(1,0,1), vec3(-1,0,1), vec3(1,0,-1), vec3(-1,0,-1),
      vec3(0,1,1), vec3(0,-1,1), vec3(0,1,-1), vec3(0,-1,-1) };

    // Permutation table.
    int p[256] = { 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233,
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
      195, 78, 66, 215, 61, 156, 180 };

    // Double the permutation table length to remove the need for index wrapping.
    short perm[512] = {};
    short perm_mod12[512] = {};
    //short perm_mod16[512] = {};

    // Skewing and unsweing factors for two dimensions.
    const float F2 = 0.5f * (sqrt(3.0f) - 1.0f);
    const float G2 = (3.0f - sqrt(3.0f)) / 6.0f;

    // Fast floor function.
    int fast_floor(float x) {
      int xi = (int)x;
      return x > xi ? xi : xi - 1;
    }

    // Dot product for one dimension.
    /*float dot(vec3 gradient, float x) {
      return gradient[0] * x;
    }*/

    // Dot product for two dimensions.
    float dot(vec3 gradient, float x, float y) {
      return gradient[0] * x + gradient[1] * y;
    }

    // One dimensional Simplex Noise.
    /*float simplex_noise(float x_in) {
      // Noise contributions from the two corners.
      float n0, n1;

      // Skewing is unecessary in one dimensional space.

      // Corner coordinates.
      int i0 = fast_floor(x_in);
      int i1 = i0 + 1;
      // Distance from corners.
      float x0 = x_in - i0;
      float x1 = x0 + 1.0f;

      // Calculate hashed gradient indices for corners.
      int ii = i0 & 255;
      int gi0 = perm_mod16[ii];
      int gi1 = perm_mod16[ii + 1];

      // Calculate contributions from the corners.
      float t0 = 1.0f - x0 * x0;
      t0 *= t0;
      n0 = t0 * t0 * dot(grad1[gi0], x0);

      float t1 = 1.0f - x1 * x1;
      t1 *= t1;
      n1 = t1 * t1 * dot(grad1[gi1], x1);
      printf("%f ", n0 + n1 * 0.395f);
      printf("\n");
      return 0.395f * (n0 + n1);
    }*/

    // Two dimensional Simplex Noise.
    float simplex_noise(float x_in, float y_in) {
      // Noise contributions from the three corners.
      float n0, n1, n2;

      // Skew the input grid to determine which cell the coordinates fall into.
      float s = (x_in + y_in) * F2;
      int i = fast_floor(x_in + s);
      int j = fast_floor(y_in + s);
      float t = (i + j) * G2;

      // Unskew the cell origin back to (x, y) space.
      float x_zero = i - t;
      float y_zero = j - t;
      // The x and y distances from the cell origin.
      float x0 = x_in - x_zero;
      float y0 = y_in - y_zero;

      // Determine which simplex the coordinates fall in.
      // Simplexes are equilateral triangles for the two dimension case.
      // Offsets for middle corner in (i, j).
      int i1, j1;
      if (x0 > y0) {
        // We are in the lower triangle.
        // Triangle reads (0,0) -> (1,0) -> (1,1).
        i1 = 1;
        j1 = 0;
      }
      else {
        // We are in the upper triangle.
        // Triangle reads (0,0) -> (0,1) -> (1,1).
        i1 = 0;
        j1 = 1;
      }

      // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
      // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
      // c = (3 - sqrt(3)) / 6.

      // Offsets for middle corner in (x, y) (unskewed) coordinates.
      float x1 = x0 - i1 + G2;
      float y1 = y0 - j1 + G2;
      // Offsets for last corner in (x, y) (unskewed) coordinates.
      float x2 = x0 - 1.0f + 2.0f * G2;
      float y2 = y0 - 1.0f + 2.0f * G2;

      // Calculate hashed gradient indices for the three gradient corners.
      int ii = i & 255;
      int jj = j & 255;
      int gi0 = perm_mod12[ii + perm[jj]];
      int gi1 = perm_mod12[ii + i1 + perm[jj + j1]];
      int gi2 = perm_mod12[ii + 1 + perm[jj + 1]];

      // Calculate the contribution from each of the three corners.
      float t0 = 0.5f - x0 * x0 - y0 * y0;
      if (t0 < 0) {
        n0 = 0.0f;
      }
      else {
        t0 *= t0;
        // x and y of grad3 used for two dimension gradient.
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
      }

      float t1 = 0.5 - x1 * x1 - y1 * y1;
      if (t1 < 0) {
        n1 = 0.0f;
      }
      else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
      }

      float t2 = 0.5 - x2 * x2 - y2 * y2;
      if (t2 < 0) n2 = 0.0f;
      else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
      }

      // Add contributions from the three corners to get the final noise value.
      // Scaled to fall in the range -1.0 - 1.0.
      return 70.0f * (n0 + n1 + n2);
    }
  };
}
