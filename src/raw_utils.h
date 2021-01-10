#ifndef RAW_UTILS_H
#define RAW_UTILS_H

#include <assert.h>
#include <cmath>
#include <cstring>
#include <vector>

#include "tiny_dng_loader.h"

std::vector<uint16_t> unpack(const tinydng::DNGImage &raw);

double fclamp(double x, double minx, double maxx);

std::vector<uint16_t> pre_color_correction(const std::vector<uint16_t> &in,
                                           int width, int height,
                                           int black_level);

void compute_color_matrix(double dst[3][3], const double color_matrix[3][3],
                          const double wb[3]);

std::vector<uint16_t> color_correction(const std::vector<uint16_t> &in,
                                       int width, int height,
                                       const double color_matrix[3][3]);

#endif // RAW_UTILS_H
