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
#define MESSAGE_LENGTH 8

/***
*  Takes readings from 2 gpio pins for I2C communication.
*/
void read_gpio_pins(uint8_t scl_pin, uint8_t sda_pin, uint8_t* message, size_t length) {
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

    if (!message || length != MESSAGE_LENGTH) {
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

    uint8_t bit_index = 0;
    uint8_t current_byte = 0;
    size_t byte_index = 0;

    while (byte_index < length) {
        size_t timeout = 1000;
        // Wait for SCL to go high (clock synchronization)
        while (!furi_hal_gpio_read(&scl) && timeout--) {
            furi_delay_ms(1);
        }
        if (timeout == 0) break;

        // Read SDA for the data bit
        uint8_t bit = furi_hal_gpio_read(&sda);
        current_byte = (current_byte << 1) | bit;
        bit_index++;

        if (bit_index == 8) { // One byte complete
            message[byte_index++] = current_byte;
            current_byte = 0;
            bit_index = 0;
        }

        // Wait for SCL to go low (end of clock cycle)
        timeout = 1000;
        while (furi_hal_gpio_read(&scl) && timeout--) {
            furi_delay_ms(1);
        }
        if (timeout == 0) break;
    }
}


/***
* Takes message found in the GPIO pins and transmits it in 433.92 MHz radio
*/
int transmit_message(uint8_t* message, size_t length) {
    if (length != MESSAGE_LENGTH) {
        return -1; // Ensure correct length
    }

    furi_hal_subghz_flush_tx();
    uint32_t real_frequency = furi_hal_subghz_set_frequency_and_path(FREQUENCY);

    if (!furi_hal_subghz_is_frequency_valid(real_frequency)) {
        FURI_LOG_E("SubGHz", "Invalid frequency: %lu Hz", (unsigned long)real_frequency);
        furi_hal_subghz_sleep();
        return -1;
    }

    furi_hal_subghz_write_packet(message, length);

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
void render_gui(Canvas* canvas, uint8_t* message, size_t length, bool transmitted) {
    if (!message || length != MESSAGE_LENGTH) {
        return; // Validate inputs
    }

    char display_message[128] = {0};
    snprintf(display_message, sizeof(display_message), "Freq: %lu Hz\nMessage: ", (unsigned long)FREQUENCY);

    for (size_t i = 0; i < length; i++) {
        snprintf(display_message + strlen(display_message),
                 sizeof(display_message) - strlen(display_message),
                 "%02X", message[i]);
    }

    // Draw on the canvas
    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, display_message);
    canvas_draw_str(canvas, 0, 30, transmitted ? "Status: Transmitted" : "Status: Ready");
    canvas_draw_str(canvas, 0, 50, "Press OK to Transmit");
}

/***
 * Callback to handle drawing on the ViewPort.
 */
void draw_callback(Canvas* canvas, void* context) {
    uint8_t* encrypted_message = ((uint8_t**)context)[0];
    bool* transmitted = ((bool**)context)[1];
    render_gui(canvas, encrypted_message, MESSAGE_LENGTH, *transmitted);
}

/***
 * Function ran on entry
 */
long flipper_transmission(void* p) {
    UNUSED(p);
    uint8_t encrypted_message[MESSAGE_LENGTH] = {0};
    bool transmitted = false;

    // Simulated message reading
    read_gpio_pins(4, 5, encrypted_message, MESSAGE_LENGTH);

    // Allocate ViewPort and set up the GUI
    ViewPort* view_port = view_port_alloc();
    if (view_port == NULL) {
        FURI_LOG_E("GUI", "Failed to allocate ViewPort");
        return -1;
    }

    void* context[] = {encrypted_message, &transmitted};

    view_port_draw_callback_set(view_port, draw_callback, context);

    // Attach the ViewPort to the GUI
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while (1) {
        transmitted = transmit_message(encrypted_message, MESSAGE_LENGTH) == 0;
        view_port_update(view_port); // Trigger GUI update
        furi_delay_ms(1000);        // Prevent CPU overload
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close("gui");

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
