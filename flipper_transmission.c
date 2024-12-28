#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "furi_hal_gpio.h"
#include "core/thread.h"
#include "furi_hal.h"
#include "furi_hal_subghz.h"
#include "gui/gui.h"


#define FURI_CONFIG_THREAD_STACK_SIZE 1024
#define FREQUENCY 433920000

/***
*  Takes readings from 2 gpio pins for I2C communication.
*/
void read_gpio_pins(uint8_t scl_pin, uint8_t sda_pin, uint64_t* message, size_t length) {
    /* 8-bit message without I2C
    if (!message) {
        return; // Ensure the message pointer is valid
    }

    for (size_t i = 0; i < length; i++) {
        GpioPin pin = {
            .port = GPIOC,
            .pin = (1U << i) // Assigns the corresponding pin based on index
        };

        furi_hal_gpio_init(&pin, GpioModeInput, GpioPullUp, GpioSpeedLow);
        message[i] = (uint8_t)furi_hal_gpio_read(&pin);
    }
     */

    if (!message || length == 0) {
        return; // Validate inputs
    }

    GpioPin scl = {
        .port = GPIOC,
        .pin = (1U << scl_pin)
    };

    GpioPin sda = {
        .port = GPIOC,
        .pin = (1U << sda_pin)
    };

    // Initialize GPIO pins for input
    furi_hal_gpio_init(&scl, GpioModeInput, GpioPullUp, GpioSpeedLow);
    furi_hal_gpio_init(&sda, GpioModeInput, GpioPullUp, GpioSpeedLow);

    size_t byte_index = 0;
    uint8_t bit_index = 0;
    uint8_t current_byte = 0;

    while (byte_index < length) {
        // Wait for SCL to go high (clock synchronization)
        while (!furi_hal_gpio_read(&scl)) {}

        // Read SDA for the data bit
        uint8_t bit = furi_hal_gpio_read(&sda);

        // Accumulate the bit into the current byte
        current_byte = (current_byte << 1) | bit;
        bit_index++;

        if (bit_index == 8) { // One byte complete
            message[byte_index++] = current_byte;
            current_byte = 0;
            bit_index = 0;
        }

        // Wait for SCL to go low (end of clock cycle)
        while (furi_hal_gpio_read(&scl)) {}
    }
}


/***
* Takes message found in the GPIO pins and transmits it in 433.92 MHz radio
*/
int transmit_message(uint64_t* message, size_t length) {
    furi_hal_subghz_flush_tx();
    uint32_t real_frequency = furi_hal_subghz_set_frequency_and_path(FREQUENCY);

    if (!furi_hal_subghz_is_frequency_valid(real_frequency)) {
        FURI_LOG_E("SubGHz", "Invalid frequency: %lu Hz", (unsigned long)real_frequency);
        furi_hal_subghz_sleep();
        return -1;
    }

    uint8_t chopped_message[8];
    for (int i = 0; i < 8; i++) {
        chopped_message[i] = (*message >> (8 * (7 - i))) & 0xFF; // Extract each byte
    }
    furi_hal_subghz_write_packet(chopped_message, (uint64_t)length);

    if (furi_hal_subghz_tx()) {
        FURI_LOG_I("SubGHz", "Message transmitted successfully.");
    } else {
        FURI_LOG_E("SubGHz", "Message transmission failed.");
        return -1;
    }

    furi_hal_subghz_sleep();
    return 0;
}

/***
* Renders the GUI with frequency, encrypted message, and a transmit button.
*/
void render_gui(uint64_t* message, size_t length, bool transmitted) {
    ViewPort* viewport = view_port_alloc();
    Canvas* canvas = viewport->canvas;

    char display_message[64];
    snprintf(display_message, sizeof(display_message), "Freq: %lu Hz\nMessage: ", FREQUENCY);

    for (size_t i = 0; i < length; i++) {
        snprintf(display_message + strlen(display_message), sizeof(display_message) - strlen(display_message), "%02X", message[i]);
    }

    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, display_message);
    canvas_draw_str(canvas, 0, 30, transmitted ? "Status: Transmitted" : "Status: Ready");
    canvas_draw_str(canvas, 0, 50, "Press OK to Transmit");

    view_port_update(viewport);
}

/***
* Function ran on entry
*/
int flipper_transmission(void* p) {
    UNUSED(p);

    uint64_t* encrypted_message;
    // Change for whatever pins I want to use for I2C
    read_gpio_pins(4, 5, encrypted_message, sizeof(encrypted_message));

    render_gui(encrypted_message, sizeof(encrypted_message), false);

    while (1) {
        if (transmit_message(encrypted_message, sizeof(encrypted_message)) == 0) {
            render_gui(encrypted_message, sizeof(encrypted_message), true);
        } else {
            render_gui(encrypted_message, sizeof(encrypted_message), false);
        }
    }
    return 0;
}

// Entry point for application
int main() {
    // Create the flipper_transmission thread
    FuriThread* thread = furi_thread_alloc_ex(
        "flipper_transmission",
        FURI_CONFIG_THREAD_STACK_SIZE,
        flipper_transmission,
        NULL
    );
    if (thread == NULL) {
        return -1;
    }
    furi_thread_start(thread);
    return 0;
}
