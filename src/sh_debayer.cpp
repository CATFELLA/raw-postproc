#include "sh_debayer.h"

std::vector<float> sh_debayer::debay(const std::vector<float> &in, int width,
                                     int height,
                                     const int debayerOffset[2]) const {
  std::vector<float> out(width * height * 3);

  const int BlueX = 1 - debayerOffset[0];
  const int BlueY = 1 - debayerOffset[1];

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {

      int x = ix + debayerOffset[0];
      int y = iy + debayerOffset[1];

      float rgb[3];
      if ((x & 1) == debayerOffset[0] && (y & 1) == debayerOffset[1]) {
        // Center pixel is red
        rgb[0] = in[iy * width + ix];
        rgb[1] = (fetch(in, x, y - 1, width, height) +
                  fetch(in, x, y + 1, width, height) +
                  fetch(in, x - 1, y, width, height) +
                  fetch(in, x + 1, y, width, height)) /
                 4;
      } else if ((x & 1) == BlueX && (y & 1) == BlueY) {
        // Center pixel is blue
        rgb[1] = (fetch(in, x, y - 1, width, height) +
                  fetch(in, x, y + 1, width, height) +
                  fetch(in, x - 1, y, width, height) +
                  fetch(in, x + 1, y, width, height)) /
                 4;
        rgb[2] = in[iy * width + ix];
      } else {
        // Center pixel is green
        rgb[1] = in[iy * width + ix];
      }

      out[3 * (iy * width + ix) + 0] = rgb[0];
      out[3 * (iy * width + ix) + 1] = rgb[1];
      out[3 * (iy * width + ix) + 2] = rgb[2];
    }
  }

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      int x = ix + debayerOffset[0];
      int y = iy + debayerOffset[1];

      if ((x & 1) == debayerOffset[0] && (y & 1) == debayerOffset[1]) {
        // Center pixel is red
        out[3 * (iy * width + ix) + 2] =
            (out[3 * (iy * width + ix) + 1] / 4) *
            (fetch(out, 3 * (ix - 1) + 2, iy - 1, width * 3, height) /
                 fetch(out, 3 * (ix - 1) + 1, iy - 1, width * 3, height) +
             fetch(out, 3 * (ix + 1) + 2, iy - 1, width * 3, height) /
                 fetch(out, 3 * (ix + 1) + 1, iy + 1, width * 3, height) +
             fetch(out, 3 * (ix + 1) + 2, iy - 1, width * 3, height) /
                 fetch(out, 3 * (ix + 1) + 1, iy + 1, width * 3, height) +
             fetch(out, 3 * (ix - 1) + 2, iy + 1, width * 3, height) /
                 fetch(out, 3 * (ix - 1) + 1, iy + 1, width * 3, height));
      } else if ((x & 1) == BlueX && (y & 1) == BlueY) {
        // Center pixel is blue
        out[3 * (iy * width + ix) + 0] =
            (out[3 * (iy * width + ix) + 1] / 4) *
            (fetch(out, 3 * (ix - 1) + 0, iy - 1, width * 3, height) /
                 fetch(out, 3 * (ix - 1) + 1, iy - 1, width * 3, height) +
             fetch(out, 3 * (ix + 1) + 0, iy - 1, width * 3, height) /
                 fetch(out, 3 * (ix + 1) + 1, iy + 1, width * 3, height) +
             fetch(out, 3 * (ix + 1) + 0, iy - 1, width * 3, height) /
                 fetch(out, 3 * (ix + 1) + 1, iy + 1, width * 3, height) +
             fetch(out, 3 * (ix - 1) + 0, iy + 1, width * 3, height) /
                 fetch(out, 3 * (ix - 1) + 1, iy + 1, width * 3, height));

      } else {
        // Center pixel is green
        if ((y & 1) == debayerOffset[1]) {
          // Left and right neighbors are red
          out[3 * (iy * width + ix) + 0] =
              (out[3 * (iy * width + ix) + 1] / 2) *
              (fetch(out, 3 * (ix - 1) + 0, iy, width * 3, height) /
                   fetch(out, 3 * (ix - 1) + 1, iy, width * 3, height) +
               fetch(out, 3 * (ix + 1) + 0, iy, width * 3, height) /
                   fetch(out, 3 * (ix + 1) + 1, iy, width * 3, height));
          out[3 * (iy * width + ix) + 2] =
              (out[3 * (iy * width + ix) + 1] / 2) *
              (fetch(out, 3 * (ix) + 2, iy - 1, width * 3, height) /
                   fetch(out, 3 * (ix) + 1, iy - 1, width * 3, height) +
               fetch(out, 3 * (ix) + 2, iy + 1, width * 3, height) /
                   fetch(out, 3 * (ix) + 1, iy + 1, width * 3, height));

        } else {
          // Left and right neighbors are blue
          out[3 * (iy * width + ix) + 0] =
              (out[3 * (iy * width + ix) + 1] / 2) *
              (fetch(out, 3 * (ix) + 0, iy - 1, width * 3, height) /
                   fetch(out, 3 * (ix) + 1, iy - 1, width * 3, height) +
               fetch(out, 3 * (ix) + 0, iy + 1, width * 3, height) /
                   fetch(out, 3 * (ix + 1) + 1, iy + 1, width * 3, height));
          out[3 * (iy * width + ix) + 2] =
              (out[3 * (iy * width + ix) + 1] / 2) *
              (fetch(out, 3 * (ix - 1) + 2, iy, width * 3, height) /
                   fetch(out, 3 * (ix - 1) + 1, iy, width * 3, height) +
               fetch(out, 3 * (ix + 1) + 2, iy, width * 3, height) /
                   fetch(out, 3 * (ix + 1) + 1, iy, width * 3, height));
        }
      }
    }
  }

  return out;
}
