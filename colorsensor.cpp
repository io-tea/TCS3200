#include "colorsensor.h"
#include <algorithm>

using namespace iotea;

ColorSensor::ColorSensor(
    PinName s0, PinName s1, PinName s2, PinName s3, PinName out) noexcept
    : s0(s0), s1(s1), s2(s2), s3(s3), out(out) {
  setFrequency(ColorSensor::Frequency::SCALE_100);
  state = ColorSensor::Filter::RED;
  setFilter(state);
  out.mode(PullUp);
  out.rise(this, &ColorSensor:increaseCounter);
  ticker.attach(this, &ColorSensor::readCounter, .01);
}

void ColorSensor::setFrequency(ColorSensor::Frequency frequency) noexcept {
  uint8_t f = static_cast<uint8_t>(frequency);
  s0.write(f & 0b10);
  s1.write(f & 0b01);
}

void ColorSensor::setFilter(ColorSensor::Filter filter) noexcept {
  uint8_t f = static_cast<uint8_t>(filter);
  s2.write(f & 0b10);
  s3.write(f & 0b01);
}

void ColorSensor::calibrateBlack(ColorData colorData) noexcept {
  calibrationBlack = colorData;
}

void ColorSensor::calibrationWhite(ColorData colorData) noexcept {
  calibrationWhite = colorData;
}

ColorSensor::ColorRGB ColorSensor::getRGB() const noexcept {
  ColorRGB colorRGB(0, 0, 0);
  convertRGB(&colorRGB);
  return colorRGB;
}

ColorSensor::Filter& operator++(ColorSensor::Filter &filter) noexcept {
  using IntType = typename std::underlying_type<ColorSensor::Filter>::type;
  IntType value = (static_cast<IntType>(filter) + 1) % 4;
  filter = static_cast<ColorSensor::Filter>(value);
  return filter;
}

void ColorSensor::increaseCounter() {
  counter++;
}

void ColorSensor::readCounter() {
  if (state != ColorSensor::Filter::CLEAR) {
    int64_t *data = reinterpret_cast<int64_t *>(&colorData);
    // Filter::GREEN is no. 3, but in a struct it is on *(data + 2)
    // We omit Filter::CLEAR so it won't be affected by clamp
    uint8_t s = std::clamp(static_cast<uint8_t>(state), 0, 2);
    *(data + s) = counter;
  }
  setFilter(++state);
  counter = 0;
}

void ColorSensor::convertRGB(ColorRGB *colorRGB) const noexcept {
  int32_t value;
  uint8_t *rgb = reinterpret_cast<uint8_t *>(colorRGB);
  int32_t *data = reinterpret_cast<int32_t *>(&colorData);
  int32_t *black = reinterpret_cast<int32_t *>(&calibrationBlack);
  int32_t *white = reinterpret_cast<int32_t *>(&calibrationWhite);

  for (size_t i = 0; i < 3; i++) {
    value = (*(data + i) - *(black + i)) * 256;
    value /= *(white + i) - *(black + i);
    *(rgb + i) = static_cast<uint8_t>(std::clamp(value, 0, 255));
  }
}
