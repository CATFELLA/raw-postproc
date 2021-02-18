#pragma once

#include "base_debayer.h"
#include "raw_utils.h"

class bilinear_debayer : public base_debayer {
public:
  bilinear_debayer() = default;
  ~bilinear_debayer() override = default;
  std::vector<float> debay(const std::vector<float> &in, int width, int height,
                           const int debayerOffset[2]) const override;
};
