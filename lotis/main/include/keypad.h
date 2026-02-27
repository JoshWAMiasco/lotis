#pragma once

/**
 * @file keypad.h
 * @brief 3x4 Matrix Keypad Driver using PCF8574 I/O Expander
 *
 * This driver interfaces with a 3x4 matrix keypad through a PCF8574
 * I2C I/O expander. It provides key press and release detection with
 * debouncing support.
 *
 * PCF8574 Pin Mapping:
 *   P0 -> Column 2
 *   P1 -> Row 1
 *   P2 -> Column 1
 *   P3 -> Row 4
 *   P4 -> Column 3
 *   P5 -> Row 3
 *   P6 -> Row 2
 *   P7 -> Not connected
 *
 * Keypad Layout:
 *   [1] [2] [3]   <- Row 1
 *   [4] [5] [6]   <- Row 2
 *   [7] [8] [9]   <- Row 3
 *   [*] [0] [#]   <- Row 4
 */

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/** PCF8574 I2C address (A0=A1=A2=LOW) */
#define KEYPAD_PCF8574_ADDR     0x20

/** Key event types */
typedef enum {
    KEYPAD_EVENT_PRESS,     /**< Key was pressed */
    KEYPAD_EVENT_RELEASE    /**< Key was released */
} keypad_event_type_t;

/** Key event structure */
typedef struct {
    char key;                       /**< The key character ('0'-'9', '*', '#') */
    keypad_event_type_t event;      /**< Event type (press or release) */
} keypad_event_t;

/** Callback function type for key events */
typedef void (*keypad_callback_t)(keypad_event_t event);

/**
 * @brief Initialize the keypad driver
 *
 * @param i2c_port I2C port number (must be already initialized)
 * @return 0 on success, -1 on failure
 */
int keypad_init(i2c_port_t i2c_port);

/**
 * @brief Scan the keypad for pressed keys
 *
 * @return The pressed key character, or '\0' if no key is pressed
 */
char keypad_scan(void);

/**
 * @brief Register a callback for key events
 *
 * @param callback Function to call when a key event occurs
 */
void keypad_set_callback(keypad_callback_t callback);

/**
 * @brief Start the keypad scanning task
 *
 * Creates a FreeRTOS task that continuously scans the keypad
 * and invokes callbacks for press/release events.
 */
void keypad_start_task(void);

/**
 * @brief Stop the keypad scanning task
 */
void keypad_stop_task(void);

#ifdef __cplusplus
}
#endif
