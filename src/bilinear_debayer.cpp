#include "bilinear_debayer.h"

std::vector<float> bilinear_debayer::debay(const std::vector<float> &in,
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
