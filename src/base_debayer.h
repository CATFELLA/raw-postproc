#pragma once

#include <QRgb>
#include <vector>

class base_debayer {
public:
  virtual ~base_debayer() = default;
  virtual std::vector<float> debay(const std::vector<float> &in, int width,
                                   int height,
                                   const int debayerOffset[2]) const = 0;
};
