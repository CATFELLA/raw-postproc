#ifndef RAW_UTILS_H
#define RAW_UTILS_H

#include <assert.h>
#include <cmath>
#include <cstring>
#include <vector>

#include "../lib/tiny_dng_loader.h"

int clamp(int x, int minx, int maxx);

float fetch(const std::vector<float> &in, int x, int y, int w, int h);

std::vector<uint16_t> unpack(const tinydng::DNGImage &raw);

float fclamp(float x, float minx, float maxx);

void pre_color_correction(std::vector<float> &ret,
                          const std::vector<uint16_t> &in, int width,
                          int height, int black_level, int white_level);

void compute_color_matrix(double dst[3][3], const tinydng::DNGImage &image,
                          const double wb[3]);

void color_correction(std::vector<float> &in, int width, int height,
                      const double color_matrix[3][3]);

void brightness(std::vector<float> &in, int width, int height,
                const double intesity);

void contrast(std::vector<float> &in, int width, int height,
              const double contrast);

void saturation(std::vector<float> &in, int width, int height,
                const double saturation);

#endif // RAW_UTILS_H
