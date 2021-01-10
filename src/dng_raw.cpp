#include "dng_raw.h"

bool DNG_raw::load_images(std::string file_name) {
  std::string warn, err;

  bool ret =
      tinydng::LoadDNG(file_name.c_str(), custom_fields, &images, &warn, &err);

  if (!warn.empty()) {
    std::cerr << "ERR: " << err.data() << std::endl;
  }
  if (!err.empty()) {
    std::cerr << "ERR: " << err.data() << std::endl;
  }

  return ret;
}

void DNG_raw::next_frame() { frame_count = (frame_count + 1) % images.size(); }
void DNG_raw::prev_frame() {
  frame_count =
      (frame_count - 1 > images.size()) ? images.size() - 1 : frame_count - 1;
}

size_t DNG_raw::get_width() { return images[frame_count].width; }
size_t DNG_raw::get_height() { return images[frame_count].height; }

static uint16_t cclamp(uint16_t x, uint16_t minx, uint16_t maxx) {
  if (x < minx)
    return minx;
  if (x > maxx)
    return maxx;
  return x;
}

std::vector<uint16_t> DNG_raw::develop(const base_debayer &debayer,
                                       const class settings &settings) {
  std::vector<uint16_t> unpacked = unpack(images[frame_count]);

  // const float inv_scale = 1.0f / (images[frame_count].white_level[0] -
  //                              images[frame_count].black_level[0]);
  // needed ?

  std::vector<uint16_t> pre_color_corrected = pre_color_correction(
      unpacked, images[frame_count].width, images[frame_count].height,
      images[frame_count].black_level[0]);

  std::cout << "precol" << pre_color_corrected[0] << std::endl;

  std::vector<uint16_t> debayed =
      debayer.debay(pre_color_corrected, images[frame_count].width,
                    images[frame_count].height, settings.cfa_offset);

  std::cout << "debay" << debayed[0] << std::endl;

  double srgb_color_matrix[3][3];
  compute_color_matrix(srgb_color_matrix, images[frame_count].color_matrix1,
                       settings.raw_white_balance);

  std::vector<uint16_t> color_corrected =
      color_correction(debayed, images[frame_count].width,
                       images[frame_count].height, srgb_color_matrix);

  std::cout << "colorcor" << color_corrected[0] << std::endl;

  std::cout << "btw the black level is" << images[frame_count].black_level[0]
            << std::endl;

  std::vector<uint16_t> res(images[frame_count].width *
                            images[frame_count].height * 3);

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

  return res;
}
