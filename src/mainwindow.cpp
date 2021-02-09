#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  scene = new QGraphicsScene(this);

  set.reset(new settings);
  set->brightness = 0.1;
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_develop_button_clicked() {
  if (opened) {
    if (!render_thread) {
      std::unique_ptr<base_debayer> deb = std::make_unique<simple_debayer>();
      render_thread = std::make_unique<renderer>(deb, seq, set);

      qRegisterMetaType<std::shared_ptr<std::vector<float>>>();

      connect(render_thread.get(), &renderer::frame_ready, this,
              &MainWindow::handle_frame);
    }

    render_thread->start();
  }
}

void MainWindow::handle_frame(std::shared_ptr<std::vector<float>> frame) {
  int f_width = static_cast<int>(seq->get_width());
  int f_height = static_cast<int>(seq->get_height());

  std::cout << "handling width=" << f_width << " height=" << f_height
            << std::endl;

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

#define MX 1300
#define YX 300
      if ((x == MX || x == MX + 1 || x == MX + 2) && y == YX) {
        std::cout << x << " <-x y-> " << y
                  << " valR =" << qimage.pixelColor(x, y).red()
                  << " valG =" << qimage.pixelColor(x, y).green()
                  << " valB =" << qimage.pixelColor(x, y).blue() << std::endl;

        QRgba64 nval;
        nval.setRed(65365);
        qimage.setPixelColor(x, y, nval);
      }
    }
  }

  scene->addPixmap(QPixmap::fromImage(qimage));
  ui->graphicsView->setScene(scene);
}

void MainWindow::on_save_button_clicked() {}

void MainWindow::on_intensity_slider_valueChanged(int value) {
  set->intensity = value / 100. * 2;
  emit ui->develop_button->clicked();
}

void MainWindow::on_tint_slider_sliderReleased() {
  set->raw_white_balance[1] = ui->tint_slider->value() / 100.;
  emit ui->develop_button->clicked();
}

void MainWindow::on_actionOpen_DNG_triggered() {
  seq.reset(new DNG_raw);

  QString qfile_name = QFileDialog::getOpenFileName(
      this, tr("Open DNG"), "./", tr("Digital Negatives (*.dng)"));

  try {
    seq->load_images(qfile_name.toUtf8().constData());
  } catch (std::runtime_error &e) {
    std::cerr << "An exception occurred. Reason: " << e.what() << std::endl;
    QMessageBox::critical(this, "Ошибка!", e.what());

    return;
  }

  opened = true;

  emit ui->develop_button->clicked();
}

void MainWindow::on_actionExit_triggered() { QApplication::quit(); }

void MainWindow::on_brightness_slider_sliderReleased() {
  // CHTO
  emit ui->develop_button->clicked();
}
