#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "furi_hal_gpio.h"
#include "core/thread.h"
#include "furi_hal.h"

#define FURI_CONFIG_THREAD_STACK_SIZE 1024

void read_gpio_pins(uint8_t* message, size_t length) {
    if (!message) {
        return; // Ensure the message pointer is valid
    }

    for (size_t i = 0; i < length; i++) {
        GpioPin pin = {
            .port = GPIOC, // Assuming GPIOC is used; adjust as needed
            .pin = (1U << i) // Assigns the corresponding pin based on index
        };

        // Initialize the GPIO pin as input
        furi_hal_gpio_init(&pin, GpioModeInput, GpioPullUp, GpioSpeedLow);

        // Read the pin state and store it in the message buffer
        message[i] = (uint8_t)furi_hal_gpio_read(&pin);
    }
}


int transmit_message(uint8_t* message, size_t length) {
    furi_hal_subghz_init();

    // Set frequency and antenna path
    uint32_t real_frequency = furi_hal_subghz_set_frequency_and_path(433920000);
    if(!furi_hal_subghz_is_frequency_valid(real_frequency)) {
        FURI_LOG_E("SubGHz", "Invalid frequency: %lu Hz", (unsigned long)real_frequency);
        furi_hal_subghz_sleep();
        return -1;
    }

    // Flush TX FIFO to ensure clean transmission
    furi_hal_subghz_flush_tx();

    // Load the message into the TX FIFO
    furi_hal_subghz_write_packet(message, (uint8_t)length);

    // Start transmission
    if(furi_hal_subghz_tx()) {
        FURI_LOG_I("SubGHz", "Message transmitted successfully.");
    } else {
        FURI_LOG_E("SubGHz", "Message transmission failed.");
        return -1;
    }

    // Put the SubGHz hardware into sleep mode after transmission
    furi_hal_subghz_sleep();
    return 0;
}


long int flipper_transmission(void* p) {
    UNUSED(p);

    uint8_t encrypted_message[8] = {0}; // Ensure the buffer is zero-initialized

    // Read GPIO pins into the encrypted_message buffer
    read_gpio_pins(encrypted_message, sizeof(encrypted_message));

    // Transmit the read GPIO data
    if (transmit_message(encrypted_message, sizeof(encrypted_message)) != 0) {
        return -1; // Indicate error
    }

    return 0;
}

int main() {
    // Create the flipper_transmission thread
    FuriThread* thread = furi_thread_alloc_ex(
        "flipper_transmission",
        FURI_CONFIG_THREAD_STACK_SIZE, // Stack size (use appropriate value or constant)
        flipper_transmission,       // Thread function
        NULL
    );

    // Validate thread creation
    if (thread == NULL) {
        return -1;
    }

    furi_thread_start(thread);
    return 0;
}
