#pragma once

#include <QRgb>
#include <vector>

class base_debayer {
public:
  virtual ~base_debayer() = default;
  virtual std::vector<uint16_t> debay(const std::vector<uint16_t> &in,
                                      int width, int height,
                                      const int debayerOffset[2]) const = 0;
};
