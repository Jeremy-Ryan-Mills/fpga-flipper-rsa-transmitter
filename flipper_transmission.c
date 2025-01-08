#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "furi_hal_gpio.h"
#include "core/thread.h"
#include "furi_hal.h"
#include "furi_hal_subghz.h"
#include "gui/gui.h"

/***
*  DEBUGGING BRANCH FOR I2C GPIO PIN READING
*/


// Macro Constants
#define FURI_CONFIG_THREAD_STACK_SIZE 1024
#define FREQUENCY 433920000
#define MESSAGE_LENGTH 8

// Pin number to STM naming convention
const uint32_t LL_GPIO_PIN_LOOKUP[] = {
    LL_GPIO_PIN_0, LL_GPIO_PIN_1, LL_GPIO_PIN_2, LL_GPIO_PIN_3,
    LL_GPIO_PIN_4, LL_GPIO_PIN_5, LL_GPIO_PIN_6, LL_GPIO_PIN_7,
    LL_GPIO_PIN_8, LL_GPIO_PIN_9, LL_GPIO_PIN_10, LL_GPIO_PIN_11,
    LL_GPIO_PIN_12, LL_GPIO_PIN_13, LL_GPIO_PIN_14, LL_GPIO_PIN_15
};

// Struct for passing data on message
typedef struct {
    uint8_t message[MESSAGE_LENGTH];
    bool transmitted;
} GuiContext;

/***
*  @brief Takes readings from 2 gpio pins for I2C communication.
*  @param scl_pin the pin number for the clock for I2C
*  @param sda_pin the pin number for the data transfer for I2C
*  @param message the pointer to a size length array of ints to hold the encrypted message
*  @param length the length of the array that message points to
*
*/
void read_gpio_pins(uint8_t scl_pin, uint8_t sda_pin, uint8_t* message, size_t length) {

    if (!message || length != MESSAGE_LENGTH) {
        return; // Validate inputs
    }


    GpioPin scl = {
        .port = GPIOA, // Maps to GPIOA
        .pin = (uint16_t) LL_GPIO_PIN_LOOKUP[scl_pin] // Maps to Pin 7 (A7)
    };

    GpioPin sda = {
        .port = GPIOA, // Maps to GPIOA
        .pin = (uint16_t) LL_GPIO_PIN_LOOKUP[sda_pin] // Maps to Pin 6 (A6)
    };


    // Initialize GPIO pins for input
    furi_hal_gpio_init(&scl, GpioModeInput, GpioPullUp, GpioSpeedLow);
    furi_hal_gpio_init(&sda, GpioModeInput, GpioPullUp, GpioSpeedLow);

    uint8_t bit_index = 0;
    uint8_t current_byte = 0;
    size_t byte_index = 0;


    while (byte_index < length) {
        size_t timeout = 10000;
        // Wait for SCL to go high (clock synchronization)
        // Wait for 10s to allow Arduino to run for testing or for FPGA to run
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
        furi_delay_ms(5);

        // Wait for SCL to go low (end of clock cycle)
        // Reset timeout clock to 10s
        timeout = 10000;
        while (furi_hal_gpio_read(&scl) && timeout--) {
            furi_delay_ms(1);
        }
        if (timeout == 0) break;
    }

}


/***
* @brief Takes message found in the GPIO pins and transmits it in FREQUENCY
* @param message the message that is to be transmitted
* @param length the length of message
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
void render_gui(Canvas* canvas, GuiContext* context) {
    if (!context || MESSAGE_LENGTH != sizeof(context->message)) {
        FURI_LOG_E("GUI", "Invalid context");
        return;
    }

    char display_message[128] = {0};
    snprintf(display_message, sizeof(display_message), "Freq: %lu Hz\nMessage: ", (unsigned long)FREQUENCY);

    for (size_t i = 0; i < MESSAGE_LENGTH; i++) {
        snprintf(display_message + strlen(display_message),
                 sizeof(display_message) - strlen(display_message),
                 "%02X", context->message[i]);
    }

    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, display_message);
    canvas_draw_str(canvas, 0, 30, context->transmitted ? "Status: Transmitted" : "Status: Ready");
    //canvas_draw_str(canvas, 0, 50, "Press OK to Transmit");
}

/***
 * Callback to handle drawing on the ViewPort.
 */
void draw_callback(Canvas* canvas, void* context) {
    render_gui(canvas, (GuiContext*)context);
}

/***
 * Function ran on entry
 */
int32_t flipper_transmission() {
    //UNUSED(p);

    GuiContext gui_context = {.transmitted = false};
    read_gpio_pins(7, 6, gui_context.message, MESSAGE_LENGTH);

    ViewPort* view_port = view_port_alloc();
    if (!view_port) {
        FURI_LOG_E("GUI", "Failed to allocate ViewPort");
        return -1;
    }

    view_port_draw_callback_set(view_port, draw_callback, &gui_context);
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while (1) {
        gui_context.transmitted = (transmit_message(gui_context.message, MESSAGE_LENGTH) == 0);
        view_port_update(view_port);
        furi_delay_ms(1000);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close("gui");

    return 0;
}


/*
// Entry point for application
int main() {

    //uint8_t message[MESSAGE_LENGTH];
    //read_gpio_pins(7, 6, message, MESSAGE_LENGTH);
    //transmit_message(message, MESSAGE_LENGTH);


    // Create the flipper_transmission thread
    FuriThread* thread = furi_thread_alloc_ex(
        "flipper_transmission",
        FURI_CONFIG_THREAD_STACK_SIZE,
        flipper_transmission,
        NULL
    );
    if (thread == NULL) {
        FURI_LOG_E("THREAD", "Failed to allocate thread");
    }
    //furi_thread_start(thread);
    return 0;

}

 */
