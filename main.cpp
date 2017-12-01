#include "mbed.h"
#include "colorsensor.h"

Serial serial(USBTX, USBRX);
iotea::ColorSensor color_sensor(D6, D5, D4, D3, D2);
DigitalIn button(D12);

int main() {
    serial.baud(115200);
    while (true) {
        if (!button) {
            iotea::ColorData color = color_sensor.getData();
            serial.printf("#%d %d %d\r\n", color.red, color.green, color.blue);
        }
    }
}