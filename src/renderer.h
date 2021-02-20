#ifndef RENDERER_H
#define RENDERER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <memory>

#include "base_debayer.h"
#include "base_format.h"
#include "settings.h"

Q_DECLARE_METATYPE(std::vector<float> *);

class renderer : public QThread {
  Q_OBJECT
public:
  renderer(const std::shared_ptr<base_debayer> &debayer,
           const std::shared_ptr<base_format> &file,
           const std::shared_ptr<settings> &_set);

  void set_debayer(const std::shared_ptr<base_debayer> &debayer);

  void set_file(const std::shared_ptr<base_format> &file);

  void set_settings(const std::shared_ptr<settings> &_set);

  void stop_render() { stop = true; }

  void single_render();

  void run() override;
signals:
  void frame_ready(std::vector<float> *frame);

private:
  void timed_render();

  std::shared_ptr<base_debayer> _debayer;
  std::shared_ptr<base_format> _file;
  std::shared_ptr<settings> _set;

  bool stop = false;
};

#endif // RENDERER_H
