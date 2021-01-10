#pragma once

#include <QRgb>
#include <string>
#include <vector>

#include "base_debayer.h"
#include "settings.h"

class base_format {
public:
  virtual ~base_format() = default;
  virtual bool load_images(std::string file_name) = 0;
  virtual void next_frame() = 0;
  virtual void prev_frame() = 0;
  virtual size_t get_width() = 0;
  virtual size_t get_height() = 0;
  virtual std::vector<uint16_t> develop(const base_debayer &debayer,
                                        const class settings &settings) = 0;
};
