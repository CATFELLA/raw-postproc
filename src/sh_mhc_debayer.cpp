#include "sh_mhc_debayer.h"

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

std::vector<float> sh_mhc_debayer::debay(const std::vector<float> &in,
                                         int width, int height,
                                         const int debayerOffset[2]) const {
  std::cout << "yay imh shmhc" << std::endl;
  std::vector<float> out(width * height * 3);

  const int BlueX = 1 - debayerOffset[0];
  const int BlueY = 1 - debayerOffset[1];

  const float alpha = 0.5f, gamma = 0.75f;

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int iy = 0; iy < height; iy++) {
    float neigh[5][5];

    for (int ix = 0; ix < width; ix++) {

      int x = ix + debayerOffset[0];
      int y = iy + debayerOffset[1];

      for (int ny = -2; ny <= 2; ny++) {
        for (int nx = -2; nx <= 2; nx++) {
          neigh[2 + nx][2 + ny] = fetch(in, x + nx, y + ny, width, height);
        }
      }

      float rgb[3];
      float lapl;
      if ((x & 1) == debayerOffset[0] && (y & 1) == debayerOffset[1]) {
        // Center pixel is red
        lapl = neigh[2][2] -
               (neigh[0][2] + neigh[4][2] + neigh[2][0] + neigh[2][4]) / 4;

        rgb[0] = in[iy * width + ix];
        rgb[1] = (neigh[2][1] + neigh[1][2] + neigh[2][3] + neigh[3][2]) / 4 +
                 alpha * lapl;
      } else if ((x & 1) == BlueX && (y & 1) == BlueY) {
        // Center pixel is blue
        lapl = neigh[2][2] -
               (neigh[0][2] + neigh[4][2] + neigh[2][0] + neigh[2][4]) / 4;

        rgb[1] = (neigh[2][1] + neigh[1][2] + neigh[3][2] + neigh[2][3]) / 4 +
                 gamma * lapl;
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
