#include "colorsensor.h"
#include <unordered_map>

using namespace iotea;

namespace {
    const std::unordered_map<size_t, ColorSensor::Filter> stateFilter{
            {0u, ColorSensor::Filter::RED},
            {1u, ColorSensor::Filter::GREEN},
            {2u, ColorSensor::Filter::BLUE}
    };

    template<typename T>
    T clamp(T value, T low, T high) {
        if (value < low) {
            return low;
        } else if (value > high) {
            return high;
        } else {
            return value;
        }
    }
}

ColorSensor::ColorSensor(
        PinName s0, PinName s1, PinName s2, PinName s3, PinName out) noexcept
        : s0{s0}, s1{s1}, s2{s2}, s3{s3}, out{out} {
    // Set defaults
    setFrequency(ColorSensor::Frequency::SCALE_100);
    // Start measuring
    state = 0;
    setFilter(stateFilter.at(state));
    this->out.mode(PullUp);
    this->out.rise([&]{ counter++; });
    ticker.attach([&]{ readCounter(); }, .01);
}

void ColorSensor::setFrequency(ColorSensor::Frequency frequency) noexcept {
    auto f = static_cast<uint8_t>(frequency);
    s0.write(f & 0b10);
    s1.write(f & 0b01);
}

void ColorSensor::setFilter(ColorSensor::Filter filter) noexcept {
    auto f = static_cast<uint8_t>(filter);
    s2.write(f & 0b10);
    s3.write(f & 0b01);
}

void ColorSensor::calibrateBlack(ColorData colorData) noexcept {
    calibrationBlack = colorData;
}

void ColorSensor::calibrateWhite(ColorData colorData) noexcept {
    calibrationWhite = colorData;
}

ColorRGB ColorSensor::getRGB() const noexcept {
    ColorRGB colorRGB(0, 0, 0);
    convertRGB(&colorRGB);
    return colorRGB;
}

ColorData ColorSensor::getData() const {
    return colorData;
}

void ColorSensor::readCounter() noexcept {
    int64_t *data = reinterpret_cast<int64_t *>(&colorData);
    *(data + state) = counter;
    state = (state + 1) % 3;
    setFilter(stateFilter.at(state));
    counter = 0;
}

void ColorSensor::convertRGB(ColorRGB *colorRGB) const noexcept {
    int32_t value;
    auto rgb = reinterpret_cast<uint8_t *>(colorRGB);
    auto data = const_cast<int32_t *>(reinterpret_cast<const int32_t *>(&colorData));
    auto black = const_cast<int32_t *>(reinterpret_cast<const int32_t *>(&calibrationBlack));
    auto white = const_cast<int32_t *>(reinterpret_cast<const int32_t *>(&calibrationWhite));

    for (size_t i = 0; i < 3; i++) {
        value = (*(data + i) - *(black + i)) * 256;
        value /= *(white + i) - *(black + i);
        *(rgb + i) = static_cast<uint8_t>(clamp(value, 0L, 255L));
    }
}
