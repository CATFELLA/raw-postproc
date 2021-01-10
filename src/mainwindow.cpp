#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }

static inline int clamp(uint16_t x, uint16_t minx, uint16_t maxx) {
  if (x < minx)
    return minx;
  if (x > maxx)
    return maxx;
  return x;
}

void MainWindow::on_intensity_slider_valueChanged(int value) {
  set.intensity = value / 100. * 2;
  emit ui->develop_button->clicked();
}

void MainWindow::on_develop_button_clicked() {
  DNG_raw seq;

  bool ret = seq.load_images("rock.dng");

  if (ret) {
    simple_debayer deb;
    std::vector<uint16_t> developed_frame = seq.develop(deb, set);

    size_t f_width = seq.get_width();
    size_t f_height = seq.get_height();

    std::cout << "zeroeth" << developed_frame[0] << std::endl;
    std::cout << "1th" << developed_frame[1] << std::endl;
    std::cout << "2th" << developed_frame[2] << std::endl;

    qimage = new QImage(f_width, f_height, QImage::Format_RGBX64);
    qimage->fill(Qt::white); // debug

    for (int y = 0; y < f_height; y++) {
      for (int x = 0; x < f_width; x++) {
        QRgba64 val;
        if (true) {
          val.setRed(developed_frame[3 * (y * f_width + x) + 0] *
                     set.intensity * 10);
          val.setGreen(developed_frame[3 * (y * f_width + x) + 1] *
                       set.intensity * 10);
          val.setBlue(developed_frame[3 * (y * f_width + x) + 2] *
                      set.intensity * 10);
        } else {
          val.setRed(developed_frame[(y * f_width + x) + 0] * set.intensity *
                     10);
          val.setGreen(developed_frame[(y * f_width + x) + 0] * set.intensity *
                       10);
          val.setBlue(developed_frame[(y * f_width + x) + 0] * set.intensity *
                      10);
        }
        // QRgba64 val;
        /*val.setRed(framebuffer[3 * (y * images[frame_count].width + x) + 0]);
        val.setGreen(framebuffer[3 * (y * images[frame_count].width + x) + 1]);
        val.setBlue(framebuffer[3 * (y * images[frame_count].width + x) + 2]);*/

        qimage->setPixelColor(x, y, val);

        int mx = 1300;
        if ((x == mx || x == mx + 1 || x == mx + 2) && y == 100) {
          std::cout << x << " <-x y-> " << y
                    << " valR =" << developed_frame[3 * (y * f_width + x) + 0]
                    << " valG =" << developed_frame[3 * (y * f_width + x) + 1]
                    << " valB =" << developed_frame[3 * (y * f_width + x) + 2]
                    << std::endl;

          QRgba64 nval;
          nval.setRed(65365);
          // nval.setGreen(65000);
          // nval.setBlue(65000);
          qimage->setPixelColor(x, y, nval);
        }
      }
    }

    scene = new QGraphicsScene(this);
    scene->addPixmap(QPixmap::fromImage(*qimage));
    ui->graphicsView->setScene(scene);
  }
}

void MainWindow::on_save_button_clicked() {}
