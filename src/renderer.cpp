#include "renderer.h"

renderer::renderer(std::unique_ptr<base_debayer> &debayer,
                   const std::shared_ptr<base_format> &file,
                   const std::shared_ptr<settings> &set) {
  _debayer.reset(debayer.release());
  _file.reset(file.get());
  _set.reset(set.get());
}

void renderer::set_debayer(std::unique_ptr<base_debayer> &debayer) {
  { _debayer.reset(debayer.release()); }
}

void renderer::set_file(const std::shared_ptr<base_format> &file) {
  _file.reset(file.get());
}

void renderer::run() {
  std::shared_ptr<std::vector<float>> result =
      std::make_shared<std::vector<float>>();
  *result = _file->develop(*_debayer.get(), *_set.get());

  emit frame_ready(result);
}
