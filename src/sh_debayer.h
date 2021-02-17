#ifndef SH_DEBAYER_H
#define SH_DEBAYER_H

#include "base_debayer.h"

class sh_debayer : public base_debayer {
public:
  sh_debayer() = default;
  ~sh_debayer() override = default;
  std::vector<float> debay(const std::vector<float> &in, int width, int height,
                           const int debayerOffset[2]) const override;
};

#endif // SH_DEBAYER_H
