#include "colored_bayer.h"

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

      float C = in[y * width + x];

      int altername[2] = {ix % 2, iy % 2};

      float rgb[3] = {0, 0, 0};

      if (altername[1] == 0) {
        if (altername[0] == 0) {
          rgb[0] = C;
        } else {
          rgb[1] = C;
        }
      } else {
        if (altername[0] == 0) {
          rgb[1] = C;
        } else {
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
