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
      ret[y * width + x] = ptr[y * width + x];
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

float fclamp(float x, float minx, float maxx) {
  if (x < minx)
    return minx;
  if (x > maxx)
    return maxx;
  return x;
}

void pre_color_correction(std::vector<float> &ret,
                          const std::vector<uint16_t> &in, int width,
                          int height, int black_level, int white_level) {
  ret.resize(in.size());
  float scale_factor = 1 / static_cast<float>(white_level - black_level);

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      float val = in[y * width + x];

      // blacklevel is same for all channels cuz whateva...
      val -= static_cast<float>(black_level);

      ret[y * width + x] = fclamp(val * scale_factor, 0, 1.0);
    }
  }
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

static const double xyzD50_to_ProPhoto[3][3] = {
    {1.3460, -0.2556, -0.0511}, {-0.5446, 1.5082, 0.0205}, {0.0, 0.0, 1.2123}};

static const double xyzD50_to_adobeRGB[3][3] = {
    {1.9624274, -0.6105343, -0.3413404},
    {-0.9787684, 1.9161415, 0.0334540},
    {0.0286869, -0.1406752, 1.3487655}};

static const double xyzD50_to_widegammut[3][3] = {
    {1.4628067, -0.1840623, -0.2743606},
    {-0.5217933, 1.4472381, 0.0677227},
    {0.0349342, -0.0968930, 1.2884099}};

void compute_color_matrix(double dst[3][3], const tinydng::DNGImage &image,
                          const double wb[3]) {
  double xyz_to_camera1[3][3];
  double xyz_to_camera2[3][3];

  const double AB[3][3] = {{wb[0], 0, 0}, {0, wb[1], 0}, {0, 0, wb[2]}};

  matrix_mult33(xyz_to_camera1, AB, image.camera_calibration1);
  matrix_mult33(xyz_to_camera2, xyz_to_camera1, image.color_matrix1);

  double camera_to_xyz[3][3];
  inverse_matrix33(camera_to_xyz, xyz_to_camera2);

  double camera_to_xyz_D50[3][3];

  matrix_mult33(dst, xyzD50_to_ProPhoto, camera_to_xyz);
}

// Gamma correction

float ProPhoto_gamma_correction(float yps) {
  if (yps < 0.001953f) {
    return 16 * yps;
  } else {
    return std::pow(yps, 1 / 1.8f);
  }
}

float sRGB_gamma_correction(float yps) {
  if (yps <= 0.0031308f) {
    return 12.92f * yps;
  } else {
    return 1.055f * std::pow(yps, 1 / 2.2f) - 0.055f;
  }
}

void color_correction(std::vector<float> &in, int width, int height,
                      const double color_matrix[3][3]) {
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      float r = in[3 * (y * width + x) + 0];
      float g = in[3 * (y * width + x) + 1];
      float b = in[3 * (y * width + x) + 2];

      float R = color_matrix[0][0] * r + color_matrix[1][0] * g +
                color_matrix[2][0] * b;
      float G = color_matrix[0][1] * r + color_matrix[1][1] * g +
                color_matrix[2][1] * b;
      float B = color_matrix[0][2] * r + color_matrix[1][2] * g +
                color_matrix[2][2] * b;

      in[3 * (y * width + x) + 0] = fclamp(ProPhoto_gamma_correction(R), 0, 1);
      in[3 * (y * width + x) + 1] = fclamp(ProPhoto_gamma_correction(G), 0, 1);
      in[3 * (y * width + x) + 2] = fclamp(ProPhoto_gamma_correction(B), 0, 1);
    }
  }
}

void brightness(std::vector<float> &in, int width, int height,
                const double intesity) {
  if (std::fabs(intesity) > __DBL_EPSILON__) {
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        in[3 * (y * width + x) + 0] =
            fclamp(in[3 * (y * width + x) + 0] + intesity, 0, 1);
        in[3 * (y * width + x) + 1] =
            fclamp(in[3 * (y * width + x) + 1] + intesity, 0, 1);
        in[3 * (y * width + x) + 2] =
            fclamp(in[3 * (y * width + x) + 2] + intesity, 0, 1);
        // slow... (change fclamp to a modifying func
      }
    }
  }
}
