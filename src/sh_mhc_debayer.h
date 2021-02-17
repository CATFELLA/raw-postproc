#ifndef SH_MHC_DEBAYER_H
#define SH_MHC_DEBAYER_H

#include <iostream>

#include "base_debayer.h"

class sh_mhc_debayer : public base_debayer {
public:
  sh_mhc_debayer() = default;
  ~sh_mhc_debayer() override = default;
  std::vector<float> debay(const std::vector<float> &in, int width, int height,
                           const int debayerOffset[2]) const override;
};

#endif // SH_MHC_DEBAYER_H
