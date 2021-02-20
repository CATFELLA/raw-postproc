#include "dng_raw.h"

static inline bool exists_test(const std::string &name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}

bool DNG_raw::load_images(std::string file_name) {
  std::string warn, err;

  bool ret =
      tinydng::LoadDNG(file_name.c_str(), custom_fields, &images, &warn, &err);

  if (!warn.empty()) {
    std::cerr << warn << std::endl;
  }

  if (!err.empty()) {
    throw std::runtime_error(err);
  }

  try {
    std::size_t found = file_name.find_last_of("_");
    std::size_t dot_pos = file_name.find_last_of(".");
    std::string file = file_name.substr(0, found + 1);
    int num = std::stoi(file_name.substr(found + 1, dot_pos - found - 1));
    std::string ext = file_name.substr(dot_pos);

    while (ret) {
      num++;

      ret = exists_test(file + std::to_string(num) + ext);
    }

    images.resize(num);

#pragma omp for
    for (int i = 1; i < num; i++) {
      std::vector<tinydng::DNGImage> other_images;

      ret = tinydng::LoadDNG((file + std::to_string(i) + ext).c_str(),
                             custom_fields, &other_images, &warn, &err);

      images[i] = other_images[0];
    }

  } catch (std::invalid_argument) {
    // means it's not a sequence
  }

  std::cout << "height=" << images[0].height << std::endl;
  std::cout << "width=" << images[0].width << std::endl;
  std::cout << "bps=" << images[0].bits_per_sample << std::endl;
  std::cout << "sample format" << images[0].sample_format << std::endl;
  std::cout << "black level " << images[0].black_level[0] << std::endl;
  std::cout << "white level " << images[0].white_level[0] << std::endl;
  std::cout << "amount of frames " << images.size() << std::endl;

  return ret;
}

void DNG_raw::next_frame() {
  frame_counter = (frame_counter + 1) % images.size();
}
void DNG_raw::prev_frame() {
  frame_counter = (frame_counter - 1 > images.size()) ? images.size() - 1
                                                      : frame_counter - 1;
}

size_t DNG_raw::get_width() { return images[frame_counter].width; }
size_t DNG_raw::get_height() { return images[frame_counter].height; }

size_t DNG_raw::get_frame_count() { return images.size(); }

size_t DNG_raw::get_frame_counter() { return frame_counter; }

void DNG_raw::set_frame(size_t frame) { frame_counter = frame; }

int DNG_raw::get_white_level() { return images[frame_counter].white_level[0]; }

std::vector<float> DNG_raw::develop(const base_debayer &debayer,
                                    const class settings &settings) {
  std::vector<uint16_t> unpacked = unpack(images[frame_counter]);

  std::vector<float> pre_color_corrected;
  pre_color_correction(
      pre_color_corrected, unpacked, images[frame_counter].width,
      images[frame_counter].height, images[frame_counter].black_level[0],
      images[frame_counter].white_level[0]);

  std::vector<float> debayed =
      debayer.debay(pre_color_corrected, images[frame_counter].width,
                    images[frame_counter].height, settings.cfa_offset);

  double srgb_color_matrix[3][3];
  compute_color_matrix(srgb_color_matrix, images[frame_counter],
                       settings.raw_white_balance);

  color_correction(debayed, images[frame_counter].width,
                   images[frame_counter].height, srgb_color_matrix);

  brightness(debayed, images[frame_counter].width, images[frame_counter].height,
             settings.brightness);

  contrast(debayed, images[frame_counter].width, images[frame_counter].height,
           settings.contrast);

  saturation(debayed, images[frame_counter].width, images[frame_counter].height,
             settings.saturation);

  return debayed;
}
