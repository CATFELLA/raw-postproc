#pragma once

#include "base_debayer.h"

class simple_debayer : public base_debayer {
public:
  simple_debayer() = default;
  ~simple_debayer() override = default;
  std::vector<float> debay(const std::vector<float> &in, int width, int height,
                           const int debayerOffset[2]) const override;
};
