#pragma once

#include <QRgb>
#include <memory>
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
  virtual int get_white_level() = 0;
  virtual size_t get_frame_count() = 0;
  virtual size_t get_frame_counter() = 0;
  virtual void set_frame(size_t frame) = 0;
  virtual std::vector<float>
  develop(const std::shared_ptr<base_debayer> debayer,
          const std::shared_ptr<settings> settings) = 0;
};
