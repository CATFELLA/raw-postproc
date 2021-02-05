#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QMessageBox>
#include <cmath>
#include <iostream>

#define TINY_DNG_LOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINY_DNG_NO_EXCEPTION

#include "colored_bayer.h"
#include "dng_raw.h"
#include "renderer.h"
#include "settings.h"
#include "simple_debayer.h"
#include "tiny_dng_loader.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

private slots:
  void handle_frame(std::shared_ptr<std::vector<float>> frame);

  void on_intensity_slider_valueChanged(int value);

  void on_develop_button_clicked();

  void on_save_button_clicked();

  void on_tint_slider_sliderReleased();

  void on_actionOpen_DNG_triggered();

  void on_actionExit_triggered();

private:
  Ui::MainWindow *ui;
  QGraphicsScene *scene;

  std::shared_ptr<settings> set;
  std::shared_ptr<DNG_raw> seq;

  std::unique_ptr<renderer> render_thread;

  bool opened = false;
};
#endif // MAINWINDOW_H
