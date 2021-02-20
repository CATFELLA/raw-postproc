#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMainWindow>
#include <QMessageBox>
#include <QPixmap>
#include <cmath>
#include <iostream>

#define TINY_DNG_LOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINY_DNG_NO_EXCEPTION

#include "../lib/tiny_dng_loader.h"
#include "bilinear_debayer.h"
#include "colored_bayer.h"
#include "dng_raw.h"
#include "mhc_debayer.h"
#include "renderer.h"
#include "settings.h"
#include "sh_debayer.h"
#include "sh_mhc_debayer.h"
#include "timer.hpp"

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
  void handle_frame(std::vector<float> *frame);

  void on_develop_button_clicked();

  void on_save_button_clicked();

  void on_tint_slider_sliderReleased();

  void on_actionOpen_DNG_triggered();

  void on_actionExit_triggered();

  void on_brightness_slider_sliderReleased();

  void on_contrast_slider_sliderReleased();

  void on_R_slider_sliderReleased();

  void on_comboBox_currentTextChanged(const QString &arg1);

  void on_play_button_clicked();

  void on_frame_slider_sliderReleased();

  void on_R_slider_2_sliderReleased();

private:
  Ui::MainWindow *ui;
  QGraphicsScene *scene;
  QGraphicsPixmapItem *pixmapitem;
  QPixmap pixmap;

  std::shared_ptr<settings> set;
  std::shared_ptr<DNG_raw> seq;
  std::shared_ptr<base_debayer> deb;

  std::unique_ptr<renderer> render_thread;

  bool opened = false;
};
#endif // MAINWINDOW_H
