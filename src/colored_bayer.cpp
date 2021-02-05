#include "colored_bayer.h"

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

std::vector<float> colored_bayer::debay(const std::vector<float> &in, int width,
                                        int height,
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

      int altername[2];
      altername[0] = ix % 2;
      altername[1] = iy % 2;

      float rgb[3];
      if (altername[1] == 0) {
        if (altername[0] == 0) {
          rgb[0] = C;
          rgb[1] = 0;
          rgb[2] = 0;
        } else {
          rgb[0] = 0;
          rgb[1] = C;
          rgb[2] = 0;
        }
      } else {
        if (altername[0] == 0) {
          rgb[0] = 0;
          rgb[1] = C;
          rgb[2] = 0;
        } else {
          rgb[0] = 0;
          rgb[1] = 0;
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
