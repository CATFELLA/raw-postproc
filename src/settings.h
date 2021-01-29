#ifndef SETTINGS_H
#define SETTINGS_H

class settings {
public:
  double intensity;

  double gamma;
  int cfa_offset[2];
  double raw_white_balance[3] = {1, 1, 1};

  double hue;
  double saturation;
  double brightness;
  double contrast;
  double vibrance;

  settings() {
    this->intensity = 1.0;
    this->gamma = 0.9;
    this->cfa_offset[0] = 0;
    this->cfa_offset[1] = 0;

    this->hue = 0.0;
    this->saturation = 0.0;
    this->brightness = 0.0;
    this->contrast = 0.0;
    this->vibrance = 0.0;
  }
};

#endif // SETTINGS_H
