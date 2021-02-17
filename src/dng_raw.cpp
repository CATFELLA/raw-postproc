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

  std::cout << "height=" << images[0].height << std::endl;
  std::cout << "width=" << images[0].width << std::endl;
  std::cout << "bps=" << images[0].bits_per_sample << std::endl;
  std::cout << "sample format" << images[0].sample_format << std::endl;
  std::cout << "black level " << images[0].black_level[0] << std::endl;
  std::cout << "white level " << images[0].white_level[0] << std::endl;

  return ret;
}

void DNG_raw::next_frame() { frame_count = (frame_count + 1) % images.size(); }
void DNG_raw::prev_frame() {
  frame_count =
      (frame_count - 1 > images.size()) ? images.size() - 1 : frame_count - 1;
}

size_t DNG_raw::get_width() { return images[frame_count].width; }
size_t DNG_raw::get_height() { return images[frame_count].height; }

int DNG_raw::get_white_level() { return images[frame_count].white_level[0]; }

std::vector<float> DNG_raw::develop(const base_debayer &debayer,
                                    const class settings &settings) {
  std::vector<uint16_t> unpacked = unpack(images[frame_count]);

  std::vector<float> pre_color_corrected;
  pre_color_correction(pre_color_corrected, unpacked, images[frame_count].width,
                       images[frame_count].height,
                       images[frame_count].black_level[0],
                       images[frame_count].white_level[0]);

  std::vector<float> debayed =
      debayer.debay(pre_color_corrected, images[frame_count].width,
                    images[frame_count].height, settings.cfa_offset);

  double srgb_color_matrix[3][3];
  compute_color_matrix(srgb_color_matrix, images[frame_count],
                       settings.raw_white_balance);

  color_correction(debayed, images[frame_count].width,
                   images[frame_count].height, srgb_color_matrix);

  brightness(debayed, images[frame_count].width, images[frame_count].height,
             settings.brightness);

  contrast(debayed, images[frame_count].width, images[frame_count].height,
           settings.contrast);

  return debayed;
}
