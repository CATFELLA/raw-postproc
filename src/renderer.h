#ifndef RENDERER_H
#define RENDERER_H

#include <QObject>
#include <QThread>
#include <memory>

#include "base_debayer.h"
#include "base_format.h"
#include "settings.h"

Q_DECLARE_METATYPE(std::shared_ptr<std::vector<float>>);

class renderer : public QThread {
  Q_OBJECT
public:
  renderer(std::unique_ptr<base_debayer> &debayer,
           const std::shared_ptr<base_format> &file,
           const std::shared_ptr<settings> &_set);

  void set_debayer(std::unique_ptr<base_debayer> &debayer);

  void set_file(const std::shared_ptr<base_format> &file);

  void run() override;
signals:
  void frame_ready(std::shared_ptr<std::vector<float>> frame);

private:
  std::unique_ptr<base_debayer> _debayer;
  std::shared_ptr<base_format> _file;
  std::shared_ptr<settings> _set;

  bool stop = false;
};

#endif // RENDERER_H
