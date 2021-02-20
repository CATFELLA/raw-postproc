#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  scene = new QGraphicsScene(this);

  qRegisterMetaType<std::vector<float> *>();

  deb = std::make_shared<colored_bayer>();
  seq = std::make_shared<DNG_raw>();

  set = std::make_shared<settings>();
}

MainWindow::~MainWindow() {
  delete scene;
  delete ui;
}

void MainWindow::on_develop_button_clicked() {
  if (opened) {
    if (!render_thread) {
      render_thread = std::make_unique<renderer>(deb, seq, set);

      connect(render_thread.get(), &renderer::frame_ready, this,
              &MainWindow::handle_frame);
    }

    render_thread->single_render();
    ui->status->setText("Start rendering");
  }
}

void MainWindow::handle_frame(std::vector<float> *frame) {
  int f_width = static_cast<int>(seq->get_width());
  int f_height = static_cast<int>(seq->get_height());

  QImage qimage(f_width, f_height, QImage::Format_RGBX64);

  struct rgba64 {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t alpha;
  };
  rgba64 *bits = reinterpret_cast<rgba64 *>(qimage.bits());

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (int y = 0; y < f_height; y++) {
    for (int x = 0; x < f_width; x++) {
      bits[y * f_width + x].red = (*frame)[3 * (y * f_width + x) + 0] * 65365;
      bits[y * f_width + x].green = (*frame)[3 * (y * f_width + x) + 1] * 65365;
      bits[y * f_width + x].blue = (*frame)[3 * (y * f_width + x) + 2] * 65365;
    }
  }

  scene->addPixmap(QPixmap::fromImage(qimage));
  ui->graphicsView->setScene(scene);
  ui->frame_slider->setValue(seq->get_frame_counter());

  ui->status->setText("Frame handled");

  delete frame;
}

void MainWindow::on_save_button_clicked() {}

void MainWindow::on_actionOpen_DNG_triggered() {
  opened = false;

  QString qfile_name = QFileDialog::getOpenFileName(
      this, tr("Open DNG"), "./", tr("Digital Negatives (*.dng)"));

  try {
    seq->load_images(qfile_name.toUtf8().constData());
  } catch (std::runtime_error &e) {
    std::cerr << "An exception occurred. Reason: " << e.what() << std::endl;
    QMessageBox::critical(this, "Ошибка!", e.what());
    ui->status->setText("Error while loading the file");

    return;
  }

  opened = true;
  ui->frame_slider->setMaximum(seq->get_frame_count() - 1);

  ui->status->setText("File loaded");

  emit ui->develop_button->clicked();
}

void MainWindow::on_actionExit_triggered() { QApplication::quit(); }

void MainWindow::on_brightness_slider_sliderReleased() {
  set->brightness = ui->brightness_slider->value() / 300.;

  emit ui->develop_button->clicked();
}

void MainWindow::on_contrast_slider_sliderReleased() {
  set->contrast = ui->contrast_slider->value() / 100.;

  emit ui->develop_button->clicked();
}

void MainWindow::on_tint_slider_sliderReleased() {
  set->raw_white_balance[1] = ui->tint_slider->value() / 100.;
  emit ui->develop_button->clicked();
}

void MainWindow::on_R_slider_sliderReleased() {
  set->raw_white_balance[0] =
      (ui->R_slider->maximum() - ui->R_slider->value()) / 100.;
  set->raw_white_balance[2] = ui->R_slider->value() / 100.;
  emit ui->develop_button->clicked();
}

void MainWindow::on_comboBox_currentTextChanged(const QString &arg1) {
  deb = nullptr; // ok why... doesn't work without it

  if (arg1 == "colored_bayer") {
    deb = std::make_shared<colored_bayer>();
  }
  if (arg1 == "bilinear") {
    deb = std::make_shared<bilinear_debayer>();
  }
  if (arg1 == "mhc") {
    deb = std::make_shared<mhc_debayer>();
  }
  if (arg1 == "sh") {
    deb = std::make_shared<sh_debayer>();
  }
  if (arg1 == "sh_mhc") {
    deb = std::make_shared<sh_mhc_debayer>();
  }

  emit ui->develop_button->clicked();
}

void MainWindow::on_play_button_clicked() {
  if (opened) {

    if (ui->play_button->text() == "Play") {
      render_thread->start();
      ui->play_button->setText("Stop");
    } else {
      render_thread->stop_render();
      ui->play_button->setText("Play");
    }
  }
}

void MainWindow::on_frame_slider_sliderReleased() {
  seq->set_frame(ui->frame_slider->value());

  emit ui->develop_button->clicked();
}

void MainWindow::on_R_slider_2_sliderReleased() {
  set->saturation = ui->R_slider_2->value() /
                    static_cast<double>(ui->R_slider_2->maximum() / 2);

  emit ui->develop_button->clicked();
}
