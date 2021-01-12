#include "raw_utils.h"

//
// Decode 14bit integer image into floating point HDR image
//
static std::vector<uint16_t> unpack14(const unsigned char *data, int width,
                                      int height) {
  std::vector<uint16_t> ret(width * height);

  int offsets[4][3] = {{0, 0, 1}, {1, 2, 3}, {3, 4, 5}, {5, 5, 6}};
  int bit_shifts[4] = {2, 4, 6, 0};

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      unsigned char buf[7];

      // Calculate load addres for 14bit pixel(three 8 bit pixels)
      int n = int(y * width + x);

      // 56 = 14bit * 4 pixel, 8bit * 7 pixel
      int n4 = n % 4;          // used for offset & bitshifts
      int addr7 = (n / 4) * 7; // 8bit pixel pos

      int offset[3];
      offset[0] = offsets[n4][0];
      offset[1] = offsets[n4][1];
      offset[2] = offsets[n4][2];

      int bit_shift;
      bit_shift = bit_shifts[n4];

      memcpy(buf, &data[addr7], 7);

      unsigned int b0 = static_cast<unsigned int>(buf[offset[0]] & 0xff);
      unsigned int b1 = static_cast<unsigned int>(buf[offset[1]] & 0xff);
      unsigned int b2 = static_cast<unsigned int>(buf[offset[2]] & 0xff);

      // unsigned int val = (b0 << 16) | (b1 << 8) | b2;
      // unsigned int val = (b2 << 16) | (b0 << 8) | b0;
      unsigned int val = (b0 << 16) | (b1 << 8) | b2;
      // unsigned int val = b2;
      val = 0x3fff & (val >> bit_shift);

      ret[y * width + x] = static_cast<uint16_t>(val);
    }
  }

  return ret;
}

static std::vector<uint16_t> unpack16(const unsigned char *data, int width,
                                      int height) {
  std::vector<uint16_t> ret(width * height);
  const uint16_t *ptr = reinterpret_cast<const uint16_t *>(data);

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      uint16_t val = ptr[y * width + x];

      // [0, 65535]
      ret[y * width + x] = val;
    }
  }

  return ret;
}

std::vector<uint16_t> unpack(const tinydng::DNGImage &raw) {
  if (raw.bits_per_sample == 14) {
    return unpack14(raw.data.data(), raw.width, raw.height);
  } else if (raw.bits_per_sample == 16) {
    return unpack16(raw.data.data(), raw.width, raw.height);
  } else {
    throw std::invalid_argument(
        std::to_string(raw.bits_per_sample) +
        std::string(" bits per sample are not implemented."));
  }
}

double fclamp(double x, double minx, double maxx) {
  if (x < minx)
    return minx;
  if (x > maxx)
    return maxx;
  return x;
}

static int64_t lclamp(int64_t x, int64_t minx, int64_t maxx) {
  if (x < minx)
    return minx;
  if (x > maxx)
    return maxx;
  return x;
}

std::vector<uint16_t> pre_color_correction(const std::vector<uint16_t> &in,
                                           int width, int height,
                                           int black_level, int white_level) {
  std::vector<uint16_t> ret(in.size());

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int64_t val = in[y * width + x];

      // blacklevel is same for all channels cuz whateva...
      val -= black_level;

      ret[y * width + x] =
          static_cast<uint16_t>(lclamp(val, 0, white_level - black_level));
    }
  }

  return ret;
}

static void matrix_mult33(double dst[3][3], const double a[3][3],
                          const double b[3][3]) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      dst[i][j] = 0;
      for (int c = 0; c < 3; c++) {
        dst[i][j] += a[i][c] * b[c][j];
      }
    }
  }
}

static inline void inverse_matrix33(double dst[3][3], double m[3][3]) {
  double det = m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
               m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
               m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

  double invdet = 1 / det;

  dst[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invdet;
  dst[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invdet;
  dst[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invdet;
  dst[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invdet;
  dst[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invdet;
  dst[1][2] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * invdet;
  dst[2][0] = (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invdet;
  dst[2][1] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * invdet;
  dst[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invdet;
}

// sRGB = XYZD50_to_sRGB * ForwardMatrix(wbCCT) * rawWB * CameraRaw
static const double xyzD50_to_sRGB[3][3] = {{3.1338561, -1.6168667, -0.4906146},
                                            {-0.9787684, 1.9161415, 0.0334540},
                                            {0.0719453, -0.2289914, 1.4052427}};

static const double xyzD50_to_adobeRGB[3][3] = {
    {1.9624274, -0.6105343, -0.3413404},
    {-0.9787684, 1.9161415, 0.0334540},
    {0.0286869, -0.1406752, 1.3487655}};

static const double xyzD50_to_widegammut[3][3] = {
    {1.4628067, -0.1840623, -0.2743606},
    {-0.5217933, 1.4472381, 0.0677227},
    {0.0349342, -0.0968930, 1.2884099}};

void compute_color_matrix(double dst[3][3], const double color_matrix[3][3],
                          const double wb[3]) {
  //
  // "Mapping Camera Color Space to CIE XYZ Space" DNG spec
  //
  // CM = ColorMatrix
  // CC = CameraCalibration
  // AB = AnalogBalance
  // RM = ReductionMatrix
  // FM = ForwardMatrix
  //

  // CM(Color Matrix) is a matrix which converts XYZ value to camera space,
  // Thus we need to invert it to get XYZ value from camera's raw censor value
  // (Unless ForwardMatrix is not available)

  // XYZtoCamera = AB * (CC) * CM;
  double xyz_to_camera[3][3];

  const double analog_balance[3][3] = {
      {wb[0], 0, 0}, {0, wb[1], 0}, {0, 0, wb[2]}};

  matrix_mult33(xyz_to_camera, analog_balance, color_matrix);

  // camera_to_xyz = Inverse(XYZtoCamer);
  double camera_to_xyz[3][3];
  inverse_matrix33(camera_to_xyz, xyz_to_camera);

  // add CB or nah
  // XYZD50tosRGB * (CB) * CameraToXYZ

  matrix_mult33(dst, xyzD50_to_sRGB, camera_to_xyz);
}

int64_t sRGB_gamma_correction(int64_t yps) {
  return 100 * (1.055 * std::pow(yps, 1 / 2.2) - 0.055);
}

std::vector<uint16_t> color_correction(const std::vector<uint16_t> &in,
                                       int width, int height,
                                       const double color_matrix[3][3]) {
  std::vector<uint16_t> ret(in.size());

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      uint16_t r = in[3 * (y * width + x) + 0];
      uint16_t g = in[3 * (y * width + x) + 1];
      uint16_t b = in[3 * (y * width + x) + 2];

      int64_t R = color_matrix[0][0] * r + color_matrix[1][0] * g +
                  color_matrix[2][0] * b;
      int64_t G = color_matrix[0][1] * r + color_matrix[1][1] * g +
                  color_matrix[2][1] * b;
      int64_t B = color_matrix[0][2] * r + color_matrix[1][2] * g +
                  color_matrix[2][2] * b;

      ret[3 * (y * width + x) + 0] = lclamp(sRGB_gamma_correction(R), 0, 65535);
      ret[3 * (y * width + x) + 1] = lclamp(sRGB_gamma_correction(G), 0, 65535);
      ret[3 * (y * width + x) + 2] = lclamp(sRGB_gamma_correction(B), 0, 65535);
    }
  }

  return ret;
}
