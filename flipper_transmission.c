#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "furi_hal_gpio.h"
#include "core/thread.h"
#include "furi_hal.h"
#include "furi_hal_subghz.h"


#define FURI_CONFIG_THREAD_STACK_SIZE 1024

/*
void display_data(uint8_t* message, size_t length, uint32_t frequency) {
    GpioApp* app = furi_record_open(RECORD_GPIO_APP); // Get the app context

    // Prepare the buffer for GPIO data and frequency
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "GPIO Data:");

    // Create a new view to display the GPIO pin states and frequency
    for (size_t i = 0; i < length; i++) {
        snprintf(buffer, sizeof(buffer), "Pin %zu: %d", i, message[i]);
        view_dispatcher_update_text(app->view_dispatcher, GpioAppViewGpioTest, buffer, i);
    }

    // Update frequency on the display
    snprintf(buffer, sizeof(buffer), "Freq: %lu Hz", frequency);
    view_dispatcher_update_text(app->view_dispatcher, GpioAppViewGpioTest, buffer, length);

    // Trigger a refresh to update the display
    view_dispatcher_run(app->view_dispatcher);
}
*/

/***
*  Takes readings from 8 GPIO pins for values in an 8-bit message.
*/
void read_gpio_pins(uint8_t* message, size_t length) {
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
}


/***
* Takes 8-bit message found in the GPIO pins and transmits it in 433.92 MHz radio
*/
int transmit_message(uint8_t* message, size_t length) {
    furi_hal_subghz_flush_tx();
    uint32_t real_frequency = furi_hal_subghz_set_frequency_and_path(433920000);
    if (!furi_hal_subghz_is_frequency_valid(real_frequency)) {
        FURI_LOG_E("SubGHz", "Invalid frequency: %lu Hz", (unsigned long)real_frequency);
        furi_hal_subghz_sleep();
        return -1;
    }

    furi_hal_subghz_write_packet(message, (uint8_t)length);

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
* Combines the pin readout and message transmission.
*/
long int flipper_transmission(void* p) {
    UNUSED(p);

    uint8_t encrypted_message[8] = {0}; // Ensure the buffer is zero-initialized

    read_gpio_pins(encrypted_message, sizeof(encrypted_message));

    if (transmit_message(encrypted_message, sizeof(encrypted_message)) != 0) {
        return -1; // Indicate error
    }

    return 0;
}

int main() {
    // Create the flipper_transmission thread
    FuriThread* thread = furi_thread_alloc_ex(
        "flipper_transmission",
        FURI_CONFIG_THREAD_STACK_SIZE,
        flipper_transmission,
        NULL
    );

    // Validate thread creation
    if (thread == NULL) {
        return -1;
    }

    furi_thread_start(thread);
    return 0;
}
