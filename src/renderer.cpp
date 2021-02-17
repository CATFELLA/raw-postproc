#include "renderer.h"

renderer::renderer(const std::shared_ptr<base_debayer> &debayer,
                   const std::shared_ptr<base_format> &file,
                   const std::shared_ptr<settings> &set) {
  _debayer.reset(debayer.get());
  _file.reset(file.get());
  _set.reset(set.get());
}

void renderer::set_debayer(const std::shared_ptr<base_debayer> &debayer) {
  { _debayer = debayer; }
}

void renderer::set_file(const std::shared_ptr<base_format> &file) {
  _file = file;
}

void renderer::set_settings(const std::shared_ptr<settings> &set) {
  _set = set;
}

void renderer::run() {
  std::shared_ptr<std::vector<float>> result =
      std::make_shared<std::vector<float>>();
  *result = _file->develop(*_debayer.get(), *_set.get());

  emit frame_ready(result);
}
