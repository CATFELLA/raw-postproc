#include "dng_raw.h"

bool DNG_raw::load_images(std::string file_name) {
  std::string warn, err;

  // not loading a sequence
  bool ret =
      tinydng::LoadDNG(file_name.c_str(), custom_fields, &images, &warn, &err);

  if (!warn.empty()) {
    std::cerr << warn << std::endl;
  }

  if (!err.empty()) {
    throw std::runtime_error(err);
  }

  std::cout << "bps=" << images[0].bits_per_sample << std::endl;
  std::cout << "sample format" << images[0].sample_format << std::endl;
  std::cout << "black level " << images[0].black_level[0] << std::endl;
  std::cout << "white level " << images[0].white_level[0] << std::endl;

  return ret;
}

void DNG_raw::next_frame() {
  std::cout << "new frame is " << (frame_count + 1) % images.size()
            << std::endl;
  frame_count = (frame_count + 1) % images.size();
}
void DNG_raw::prev_frame() {
  frame_count =
      (frame_count - 1 > images.size()) ? images.size() - 1 : frame_count - 1;
}

size_t DNG_raw::get_width() { return images[frame_count].width; }
size_t DNG_raw::get_height() { return images[frame_count].height; }

int DNG_raw::get_white_level() { return images[frame_count].white_level[0]; }

static uint16_t cclamp(uint16_t x, uint16_t minx, uint16_t maxx) {
  if (x < minx)
    return minx;
  if (x > maxx)
    return maxx;
  return x;
}

std::vector<float> DNG_raw::develop(const base_debayer &debayer,
                                    const class settings &settings) {
  std::vector<uint16_t> unpacked = unpack(images[frame_count]);

  std::vector<float> pre_color_corrected;
  pre_color_correction(pre_color_corrected, unpacked, images[frame_count].width,
                       images[frame_count].height,
                       images[frame_count].black_level[0],
                       images[frame_count].white_level[0]);

  std::cout << "precol" << pre_color_corrected[0] << std::endl;

  std::cout << "fm111 " << images[frame_count].forward_matrix1 << std::endl;
  std::cout << "fm211 " << images[frame_count].forward_matrix2 << std::endl;

  std::vector<float> debayed =
      debayer.debay(pre_color_corrected, images[frame_count].width,
                    images[frame_count].height, settings.cfa_offset);

  std::cout << "debay" << debayed[0] << std::endl;

  double srgb_color_matrix[3][3];
  compute_color_matrix(srgb_color_matrix, images[frame_count],
                       settings.raw_white_balance);

  color_correction(debayed, images[frame_count].width,
                   images[frame_count].height, srgb_color_matrix);

  return debayed;

  std::cout << "colorcor" << debayed[0] << std::endl;

  std::cout << "btw the black level is" << images[frame_count].black_level[0]
            << std::endl;

  std::vector<float> res(images[frame_count].width *
                         images[frame_count].height * 3);
  /*
  #ifdef _OPENMP
  #pragma omp parallel for
  #endif
    for (size_t y = 0; y < images[frame_count].height; y++) {
      for (size_t x = 0; x < images[frame_count].width; x++) {
        res[3 * (y * images[frame_count].width + x) + 0] = cclamp(
            settings.intensity *
                color_corrected[3 * (y * images[frame_count].width + x) + 0],
            0, 65535);
        res[3 * (y * images[frame_count].width + x) + 1] = cclamp(
            settings.intensity *
                color_corrected[3 * (y * images[frame_count].width + x) + 1],
            0, 65535);
        res[3 * (y * images[frame_count].width + x) + 2] = cclamp(
            settings.intensity *
                color_corrected[3 * (y * images[frame_count].width + x) + 2],
            0, 65535);
      }
    }
    */
}
