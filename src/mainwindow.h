#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QGraphicsScene>
#include <QMainWindow>
#include <cmath>
#include <iostream>

#define TINY_DNG_LOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINY_DNG_NO_EXCEPTION

#include "dng_raw.h"
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
  void on_intensity_slider_valueChanged(int value);

  void on_develop_button_clicked();

  void on_save_button_clicked();

private:
  Ui::MainWindow *ui;
  QGraphicsScene *scene;
  QImage *qimage;
  settings set;
};
#endif // MAINWINDOW_H
