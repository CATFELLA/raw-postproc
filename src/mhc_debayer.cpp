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

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int iy = 0; iy < height; iy++) {
    /* Neigh holds a copy of the 5x5 neighborhood around the current point */
    float Neigh[5][5];
    /* NeighPresence is used for boundary handling.  It is set to 0 if the
       neighbor is beyond the boundaries of the image and 1 otherwise. */
    int NeighPresence[5][5];

    for (int ix = 0; ix < width; ix++) {

      int x = ix + debayerOffset[0];
      int y = iy + debayerOffset[1];

      /* 5x5 neighborhood around the point (x,y) is copied into Neigh */
      for (int ny = -2, j = x + width * (y - 2); ny <= 2; ny++, j += width) {
        for (int nx = -2; nx <= 2; nx++) {
          if (0 <= x + nx && x + nx < width && 0 <= y + ny && y + ny < height) {
            Neigh[2 + nx][2 + ny] = in[j + nx];
            NeighPresence[2 + nx][2 + ny] = 1;
          } else {
            Neigh[2 + nx][2 + ny] = 0;
            NeighPresence[2 + nx][2 + ny] = 0;
          }
        }
      }

      float rgb[3];

      if ((x & 1) == debayerOffset[0] && (y & 1) == debayerOffset[1]) {
        /* Center pixel is red */
        rgb[0] = in[iy * width + ix];
        rgb[1] = (2 * (Neigh[2][1] + Neigh[1][2] + Neigh[3][2] + Neigh[2][3]) +
                  (NeighPresence[0][2] + NeighPresence[4][2] +
                   NeighPresence[2][0] + NeighPresence[2][4]) *
                      Neigh[2][2] -
                  Neigh[0][2] - Neigh[4][2] - Neigh[2][0] - Neigh[2][4]) /
                 (2 * (NeighPresence[2][1] + NeighPresence[1][2] +
                       NeighPresence[3][2] + NeighPresence[2][3]));
        rgb[2] = (4 * (Neigh[1][1] + Neigh[3][1] + Neigh[1][3] + Neigh[3][3]) +
                  3 * ((NeighPresence[0][2] + NeighPresence[4][2] +
                        NeighPresence[2][0] + NeighPresence[2][4]) *
                           Neigh[2][2] -
                       Neigh[0][2] - Neigh[4][2] - Neigh[2][0] - Neigh[2][4])) /
                 (4 * (NeighPresence[1][1] + NeighPresence[3][1] +
                       NeighPresence[1][3] + NeighPresence[3][3]));
      } else if ((x & 1) == BlueX && (y & 1) == BlueY) {
        /* Center pixel is blue */
        rgb[0] = (4 * (Neigh[1][1] + Neigh[3][1] + Neigh[1][3] + Neigh[3][3]) +
                  3 * ((NeighPresence[0][2] + NeighPresence[4][2] +
                        NeighPresence[2][0] + NeighPresence[2][4]) *
                           Neigh[2][2] -
                       Neigh[0][2] - Neigh[4][2] - Neigh[2][0] - Neigh[2][4])) /
                 (4 * (NeighPresence[1][1] + NeighPresence[3][1] +
                       NeighPresence[1][3] + NeighPresence[3][3]));
        rgb[1] = (2 * (Neigh[2][1] + Neigh[1][2] + Neigh[3][2] + Neigh[2][3]) +
                  (NeighPresence[0][2] + NeighPresence[4][2] +
                   NeighPresence[2][0] + NeighPresence[2][4]) *
                      Neigh[2][2] -
                  Neigh[0][2] - Neigh[4][2] - Neigh[2][0] - Neigh[2][4]) /
                 (2 * (NeighPresence[2][1] + NeighPresence[1][2] +
                       NeighPresence[3][2] + NeighPresence[2][3]));
        rgb[2] = in[iy * width + ix];

      } else {
        /* Center pixel is green */
        rgb[1] = in[iy * width + ix];

        if ((y & 1) == debayerOffset[1]) {
          /* Left and right neighbors are red */
          rgb[0] = (8 * (Neigh[1][2] + Neigh[3][2]) +
                    (2 * (NeighPresence[1][1] + NeighPresence[3][1] +
                          NeighPresence[0][2] + NeighPresence[4][2] +
                          NeighPresence[1][3] + NeighPresence[3][3]) -
                     NeighPresence[2][0] - NeighPresence[2][4]) *
                        Neigh[2][2] -
                    2 * (Neigh[1][1] + Neigh[3][1] + Neigh[0][2] + Neigh[4][2] +
                         Neigh[1][3] + Neigh[3][3]) +
                    Neigh[2][0] + Neigh[2][4]) /
                   (8 * (NeighPresence[1][2] + NeighPresence[3][2]));
          rgb[2] = (8 * (Neigh[2][1] + Neigh[2][3]) +
                    (2 * (NeighPresence[1][1] + NeighPresence[3][1] +
                          NeighPresence[2][0] + NeighPresence[2][4] +
                          NeighPresence[1][3] + NeighPresence[3][3]) -
                     NeighPresence[0][2] - NeighPresence[4][2]) *
                        Neigh[2][2] -
                    2 * (Neigh[1][1] + Neigh[3][1] + Neigh[2][0] + Neigh[2][4] +
                         Neigh[1][3] + Neigh[3][3]) +
                    Neigh[0][2] + Neigh[4][2]) /
                   (8 * (NeighPresence[2][1] + NeighPresence[2][3]));
        } else {
          /* Left and right neighbors are blue */
          rgb[0] = (8 * (Neigh[2][1] + Neigh[2][3]) +
                    (2 * (NeighPresence[1][1] + NeighPresence[3][1] +
                          NeighPresence[2][0] + NeighPresence[2][4] +
                          NeighPresence[1][3] + NeighPresence[3][3]) -
                     NeighPresence[0][2] - NeighPresence[4][2]) *
                        Neigh[2][2] -
                    2 * (Neigh[1][1] + Neigh[3][1] + Neigh[2][0] + Neigh[2][4] +
                         Neigh[1][3] + Neigh[3][3]) +
                    Neigh[0][2] + Neigh[4][2]) /
                   (8 * (NeighPresence[2][1] + NeighPresence[2][3]));
          rgb[2] = (8 * (Neigh[1][2] + Neigh[3][2]) +
                    (2 * (NeighPresence[1][1] + NeighPresence[3][1] +
                          NeighPresence[0][2] + NeighPresence[4][2] +
                          NeighPresence[1][3] + NeighPresence[3][3]) -
                     NeighPresence[2][0] - NeighPresence[2][4]) *
                        Neigh[2][2] -
                    2 * (Neigh[1][1] + Neigh[3][1] + Neigh[0][2] + Neigh[4][2] +
                         Neigh[1][3] + Neigh[3][3]) +
                    Neigh[2][0] + Neigh[2][4]) /
                   (8 * (NeighPresence[1][2] + NeighPresence[3][2]));
        }
      }

      out[3 * (iy * width + ix) + 0] = rgb[0];
      out[3 * (iy * width + ix) + 1] = rgb[1];
      out[3 * (iy * width + ix) + 2] = rgb[2];
    }
  }

  return out;
}
