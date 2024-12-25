#include <furi.h>
#include <furi_hal.h>
#include <api-hal-subgz.h>
#include <api-hal-gpio.h>

void read_gpio_pins(uint8_t* message, size_t length) {
    for (size_t i = 0; i < length; i++) {
        message[i] = furi_hal_gpio_read((GpioPin)(GpioPin_PC0 + i));
    }
}

void transmit_message(uint8_t* message, size_t length) {
    SubGhzConfig subghz_config = {
        .frequency = 433920000,
        .bitrate = 9600,
        .power = SubGhzPowerHigh,
        .deviation = 5000,
    };

    furi_hal_subghz_init(&subghz_config);
    furi_hal_subghz_transmit(message, length);
    furi_hal_subghz_deinit();
}

int32_t flipper_transmission(void* p) {
    UNUSED(p);

    uint8_t encrypted_message[8];
    read_gpio_pins(encrypted_message, 8);
    transmit_message(encrypted_message, 8);

    return 0;
}
