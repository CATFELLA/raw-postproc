#ifndef DNG_RAW_H
#define DNG_RAW_H

#include <QDebug>
#include <cmath>
#include <iostream>
#include <sys/stat.h>

#include "../lib/tiny_dng_loader.h"
#include "base_format.h"
#include "raw_utils.h"

class DNG_raw : public base_format {
  size_t frame_counter = 0;
  std::vector<tinydng::DNGImage> images;
  std::vector<tinydng::FieldInfo> custom_fields;

public:
  DNG_raw() = default;
  ~DNG_raw() override {}
  bool load_images(std::string file_name) override;
  void next_frame() override;
  void prev_frame() override;
  size_t get_width() override;
  size_t get_height() override;
  int get_white_level() override;
  size_t get_frame_count() override;
  size_t get_frame_counter() override;
  void set_frame(size_t frame) override;
  std::vector<float> develop(const base_debayer &debayer,
                             const class settings &settings) override;
};

#endif // DNG_RAW_H
