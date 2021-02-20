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

void renderer::timed_render() {
  single_render();
  _file->next_frame();

  if (!stop) {
    QTimer::singleShot(0, this, &renderer::timed_render);
  }
}

void renderer::single_render() {
  std::vector<float> *result = new std::vector<float>;
  *result = _file->develop(_debayer, _set);

  emit frame_ready(result);
}

void renderer::run() {
  stop = false;
  QTimer::singleShot(0, this, &renderer::timed_render);
}
