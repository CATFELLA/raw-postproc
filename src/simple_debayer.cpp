#include "simple_debayer.h"

static inline int clamp(int x, int minx, int maxx) {
  if (x < minx)
    return minx;
  if (x > maxx)
    return maxx;
  return x;
}

static inline float fetch(const std::vector<float> &in, int x, int y, int w,
                          int h) {
  int xx = clamp(x, 0, w - 1);
  int yy = clamp(y, 0, h - 1);
  return in[yy * w + xx];
}

// Simple debayer.
// debayerOffset = pixel offset to make CFA pattern RGGB.
std::vector<float> simple_debayer::debay(const std::vector<float> &in,
                                         int width, int height,
                                         const int debayerOffset[2]) const {
  std::vector<float> out(width * height * 3);

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      int x = ix + debayerOffset[0];
      int y = iy + debayerOffset[1];

      float C = fetch(in, x, y, width, height);

      float rgb[3];
      if ((iy & 1) == 0) {
        if ((ix & 1) == 0) {
          // Center pixel is red
          rgb[0] = C;
          rgb[1] = (fetch(in, x, y - 1, width, height) +
                    fetch(in, x, y + 1, width, height) +
                    fetch(in, x - 1, y, width, height) +
                    fetch(in, x + 1, y, width, height)) /
                   4;
          rgb[2] = (fetch(in, x - 1, y - 1, width, height) +
                    fetch(in, x - 1, y + 1, width, height) +
                    fetch(in, x + 1, y - 1, width, height) +
                    fetch(in, x + 1, y + 1, width, height)) /
                   4;
        } else {
          // Center pixel is green
          // Left and right neighbors are red
          rgb[0] = (fetch(in, x - 1, y, width, height) +
                    fetch(in, x + 1, y, width, height)) /
                   2;
          rgb[1] = C;
          rgb[2] = (fetch(in, x, y - 1, width, height) +
                    fetch(in, x, y + 1, width, height)) /
                   2;
        }
      } else {
        if ((ix & 1) == 0) {
          // Center pixel is green
          // Left and right neighbors are blue
          rgb[0] = (fetch(in, x, y - 1, width, height) +
                    fetch(in, x, y + 1, width, height)) /
                   2;
          rgb[1] = C;
          rgb[2] = (fetch(in, x - 1, y, width, height) +
                    fetch(in, x + 1, y, width, height)) /
                   2;
        } else {
          // Center pixel is blue
          rgb[0] = (fetch(in, x - 1, y - 1, width, height) +
                    fetch(in, x - 1, y + 1, width, height) +
                    fetch(in, x + 1, y - 1, width, height) +
                    fetch(in, x + 1, y + 1, width, height)) /
                   4;
          rgb[1] = (fetch(in, x, y - 1, width, height) +
                    fetch(in, x, y + 1, width, height) +
                    fetch(in, x - 1, y, width, height) +
                    fetch(in, x + 1, y, width, height)) /
                   4;
          rgb[2] = C;
        }
      }

      out[3 * (iy * width + ix) + 0] = rgb[0];
      out[3 * (iy * width + ix) + 1] = rgb[1];
      out[3 * (iy * width + ix) + 2] = rgb[2];
    }
  }

  return out;
}
