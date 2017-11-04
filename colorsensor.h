#pragma once
#include "mbed.h"
#include "color.h"

namespace iotea {

using ColorRGB = typename Color<uint8_t>;
using ColorData = typename Color<int32_t>;

class ColorSensor {
public:
  enum class Frequency {
    DOWN      = 0;
    SCALE_2   = 1;
    SCALE_20  = 2;
    SCALE_100 = 3;
  };

  ColorSensor(
      PinName s0, PinName s1, PinName s2, PinName s3, PinName out) noexcept;

  /**
   * Sets frequency prescaler.
   * Defaults to Frequency::SCALE_100
   **/
  void setFrequency(Frequency frequency) noexcept;
  /**
   * Sets calibration data for black color.
   **/
  void calibrateBlack(ColorData colorData) noexcept;
  /**
   * Sets calibration data for white color.
   **/
  void calibrateWhite(ColorData colorData) noexcept;

  /**
   * Read sensor data in standard RGB form.
   * @returns RGB color.
   **/
  ColorRGB getRGB() const noexcept;
  /**
   * Reads sensor data in raw form.
   * @returns color raw data.
   **/
  ColorData getData() const noexcept;

protected:
  /// Output pin
  PinName out;
  /// Pins used for frequency scaler
  PinName s0, s1;
  /// Pins used for photodide filter
  PinName s2, s3;
  /// Sensor raw data in current read
  ColorData colorData(0, 0, 0);
  /// Sensor calibration data
  ColorData calibrationBlack(6000, 6000, 6000);
  ColorData calibrationWhite(55000, 55000, 55000);

private:
  enum class Filter {
    RED   = 0;
    BLUE  = 1;
    CLEAR = 2;
    GREEN = 3;
  };

  /**
   * Sets photodide filter pins.
   **/
  void setFilter(Filter filter) noexcept;
  /**
   * Increases current read counter.
   **/
  void increaseCounter() noexcept;
  /**
   * Reads counter value and saves it to the current state.
   **/
  void readCounter() noexcept;

  /// Current state of the counter
  Filter state;
  Ticker ticker;
  int32_t counter;

  void convertRGB(ColorRGB *colorRGB) const noexcept;
}

}
