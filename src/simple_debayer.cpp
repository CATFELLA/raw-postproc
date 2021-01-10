#include "simple_debayer.h"

static inline int clamp(int x, int minx, int maxx) {
  if (x < minx)
    return minx;
  if (x > maxx)
    return maxx;
  return x;
}

static inline uint16_t fetch(const std::vector<uint16_t> &in, int x, int y,
                             int w, int h) {
  int xx = clamp(x, 0, w - 1);
  int yy = clamp(y, 0, h - 1);
  return in[yy * w + xx];
}

// Simple debayer.
// debayerOffset = pixel offset to make CFA pattern RGGB.
std::vector<uint16_t> simple_debayer::debay(const std::vector<uint16_t> &in,
                                            int width, int height,
                                            const int debayerOffset[2]) const {
  std::vector<uint16_t> out(width * height * 3);

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      int x = ix + debayerOffset[0];
      int y = iy + debayerOffset[1];

      int xCoord[4] = {x - 2, x - 1, x + 1, x + 2};
      int yCoord[4] = {y - 2, y - 1, y + 1, y + 2};

      float C = fetch(in, x, y, width, height);
      float kC[4] = {4.0 / 8.0, 6.0 / 8.0, 5.0 / 8.0, 5.0 / 8.0};

      int altername[2];
      altername[0] = ix % 2;
      altername[1] = iy % 2;

      float Dvec[4];
      Dvec[0] = fetch(in, xCoord[1], yCoord[1], width, height);
      Dvec[1] = fetch(in, xCoord[1], yCoord[2], width, height);
      Dvec[2] = fetch(in, xCoord[2], yCoord[1], width, height);
      Dvec[3] = fetch(in, xCoord[2], yCoord[2], width, height);

      // vec4 PATTERN = (kC.xyz * C).xyzz;
      float PATTERN[4];
      PATTERN[0] = kC[0] * C;
      PATTERN[1] = kC[1] * C;
      PATTERN[2] = kC[2] * C;
      PATTERN[3] = kC[3] * C;

      // D = Dvec[0] + Dvec[1] + Dvec[2] + Dvec[3]
      Dvec[0] += Dvec[1] + Dvec[2] + Dvec[3];

      float value[4];
      value[0] = fetch(in, x, yCoord[0], width, height);
      value[1] = fetch(in, x, yCoord[1], width, height);
      value[2] = fetch(in, xCoord[0], y, width, height);
      value[3] = fetch(in, xCoord[1], y, width, height);

      float temp[4];
      temp[0] = fetch(in, x, yCoord[3], width, height);
      temp[1] = fetch(in, x, yCoord[2], width, height);
      temp[2] = fetch(in, xCoord[3], y, width, height);
      temp[3] = fetch(in, xCoord[2], y, width, height);

      float kA[4] = {-1.0 / 8.0, -1.5 / 8.0, 0.5 / 8.0, -1.0 / 8.0};
      float kB[4] = {2.0 / 8.0, 0.0 / 8.0, 0.0 / 8.0, 4.0 / 8.0};
      float kD[4] = {0.0 / 8.0, 2.0 / 8.0, -1.0 / 8.0, -1.0 / 8.0};

      value[0] += temp[0];
      value[1] += temp[1];
      value[2] += temp[2];
      value[3] += temp[3];

      float kE[4];
      kE[0] = kA[0];
      kE[1] = kA[1];
      kE[2] = kA[3];
      kE[3] = kA[2];

      float kF[4];
      kF[0] = kB[0];
      kF[1] = kB[1];
      kF[2] = kB[3];
      kF[3] = kB[2];

      PATTERN[1] += kD[1] * Dvec[0];
      PATTERN[2] += kD[2] * Dvec[0];
      PATTERN[3] += kD[2] * Dvec[0];

      PATTERN[0] += kA[0] * value[0] + kE[0] * value[2];
      PATTERN[1] += kA[1] * value[0] + kE[1] * value[2];
      PATTERN[2] += kA[2] * value[0] + kE[0] * value[2];
      PATTERN[3] += kA[0] * value[0] + kE[3] * value[2];

      PATTERN[0] += kB[0] * value[1];
      PATTERN[3] += kB[3] * value[1];

      PATTERN[0] += kF[0] * value[3];
      PATTERN[2] += kF[2] * value[3];

      float rgb[3];
      if (altername[1] == 0) {
        if (altername[0] == 0) {
          rgb[0] = C;
          rgb[1] = PATTERN[0];
          rgb[2] = PATTERN[1];
        } else {
          rgb[0] = PATTERN[2];
          rgb[1] = C;
          rgb[2] = PATTERN[3];
        }
      } else {
        if (altername[0] == 0) {
          rgb[0] = PATTERN[3];
          rgb[1] = C;
          rgb[2] = PATTERN[2];
        } else {
          rgb[0] = PATTERN[1];
          rgb[1] = PATTERN[0];
          rgb[2] = C;
        }
      }

      out[3 * (iy * width + ix) + 0] = static_cast<uint16_t>(rgb[0]);
      out[3 * (iy * width + ix) + 1] = static_cast<uint16_t>(rgb[1]);
      out[3 * (iy * width + ix) + 2] = static_cast<uint16_t>(rgb[2]);
    }
  }

  return out;
}
