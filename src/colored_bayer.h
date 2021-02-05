#ifndef COLORED_BAYER_H
#define COLORED_BAYER_H

#include "base_debayer.h"

class colored_bayer : public base_debayer {
public:
  colored_bayer() = default;
  ~colored_bayer() override = default;
  std::vector<float> debay(const std::vector<float> &in, int width, int height,
                           const int debayerOffset[2]) const override;
};

#endif // COLORED_BAYER_H
