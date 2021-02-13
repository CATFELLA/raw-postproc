#include "mhc_debayer.h"

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

std::vector<float> mhc_debayer::debay(const std::vector<float> &in, int width,
                                      int height,
                                      const int debayerOffset[2]) const {
  std::vector<float> out(width * height * 3);

  const int BlueX = 1 - debayerOffset[0];
  const int BlueY = 1 - debayerOffset[1];

  const float alpha = 1 / 2, beta = 5 / 8, gamma = 3 / 4;

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
        rgb[2] = (neigh[1][1] + neigh[1][3] + neigh[3][3] + neigh[3][1]) / 4 +
                 alpha * lapl;
      } else if ((x & 1) == BlueX && (y & 1) == BlueY) {
        // Center pixel is blue
        lapl = neigh[2][2] -
               (neigh[0][2] + neigh[4][2] + neigh[2][0] + neigh[2][4]) / 4;

        rgb[0] = (neigh[1][1] + neigh[3][1] + neigh[1][3] + neigh[3][3]) / 4 +
                 gamma * lapl;
        rgb[1] = (neigh[2][1] + neigh[1][2] + neigh[3][2] + neigh[2][3]) / 4 +
                 gamma * lapl;
        rgb[2] = in[iy * width + ix];
      } else {
        // Center pixel is green
        lapl = (neigh[2][2] -
                (neigh[1][1] + neigh[3][1] + neigh[0][2] + neigh[4][2] +
                 neigh[1][3] + neigh[3][3] + neigh[2][0] + neigh[2][4]) /
                    8);
        rgb[1] = in[iy * width + ix];

        if ((y & 1) == debayerOffset[1]) {
          // Left and right neighbors are red
          rgb[0] = (neigh[1][2] + neigh[3][2]) / 2 + beta * lapl;
          rgb[2] = (neigh[2][1] + neigh[2][3]) / 2 + beta * lapl;
        } else {
          // Left and right neighbors are blue
          rgb[0] = (neigh[2][1] + neigh[2][3]) / 2 + beta * lapl;
          rgb[2] = (neigh[1][2] + neigh[3][2]) / 2 + beta * lapl;
        }
      }

      out[3 * (iy * width + ix) + 0] = rgb[0];
      out[3 * (iy * width + ix) + 1] = rgb[1];
      out[3 * (iy * width + ix) + 2] = rgb[2];
    }
  }

  return out;
}
