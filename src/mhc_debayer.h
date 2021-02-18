#ifndef MHC_DEBAYER_H
#define MHC_DEBAYER_H

#include "base_debayer.h"
#include "raw_utils.h"

class mhc_debayer : public base_debayer {
public:
  mhc_debayer() = default;
  ~mhc_debayer() override = default;
  std::vector<float> debay(const std::vector<float> &in, int width, int height,
                           const int debayerOffset[2]) const override;
};
#endif // MHC_DEBAYER_H
