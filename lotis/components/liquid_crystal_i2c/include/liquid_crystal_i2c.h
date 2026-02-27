#pragma once

/**
 * @file liquid_crystal_i2c.h
 * @brief HD44780 LCD I2C Driver Component for ESP-IDF
 *
 * This component provides a driver for HD44780-compatible character LCD displays
 * connected via I2C using a PCF8574 I/O expander. Common displays include 16x2
 * and 20x4 character LCDs widely available for embedded projects.
 *
 * Features:
 * - Support for multiple LCD sizes (16x2, 20x4, etc.)
 * - Text display with cursor positioning
 * - Display clear and home functions
 * - Backlight control (on/off)
 * - Custom character creation (up to 8 custom characters)
 * - 4-bit communication mode over I2C
 * - Compatible with PCF8574 I2C I/O expander
 *
 * Hardware Connection:
 * - The LCD is connected to ESP32 via I2C bus using a PCF8574 backpack
 * - Common I2C addresses: 0x27 or 0x3F
 * - Requires pull-up resistors on SDA and SCL lines
 *
 * Example usage:
 * @code
 * lcd_i2c_config_t lcd_cfg = {
 *     .i2c_port = I2C_NUM_0,
 *     .i2c_address = 0x27,
 *     .cols = 16,
 *     .rows = 2
 * };
 * lcd_i2c_init(&lcd_cfg);
 * lcd_i2c_clear();
 * lcd_i2c_set_cursor(0, 0);
 * lcd_i2c_print("Hello World!");
 * @endcode
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LCD I2C configuration structure
 */
typedef struct {
    i2c_port_t i2c_port;    /**< I2C port number (I2C_NUM_0 or I2C_NUM_1) */
    uint8_t    i2c_address; /**< I2C address of PCF8574 (typically 0x27 or 0x3F) */
    uint8_t    cols;        /**< Number of columns (e.g., 16 or 20) */
    uint8_t    rows;        /**< Number of rows (e.g., 2 or 4) */
} lcd_i2c_config_t;

/* Lifecycle */

/**
 * @brief Initialize the LCD display
 *
 * Configures the HD44780 LCD controller in 4-bit mode and prepares it for use.
 * Sets display on, cursor off, and clears the screen. The I2C bus must be
 * initialized before calling this function.
 *
 * @param cfg Pointer to LCD configuration structure
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if cfg is NULL
 *
 * @note The I2C bus must be initialized separately using i2c_helper_init
 * @note Initialization includes a 50ms startup delay as required by HD44780
 */
esp_err_t lcd_i2c_init(const lcd_i2c_config_t *cfg);

/* Display control */

/**
 * @brief Clear the LCD display and reset cursor to home position
 *
 * Clears all characters from the display and moves cursor to position (0,0).
 * This operation takes approximately 2ms to complete.
 */
void lcd_i2c_clear(void);

/**
 * @brief Clear a specific row on the LCD display
 *
 * Clears all characters from the specified row by filling it with spaces.
 * The cursor is positioned at the beginning of the cleared row after completion.
 *
 * @param row Row number to clear (0-based, 0 to rows-1)
 *
 * @note If row exceeds available rows, it will be clamped to the last row
 * @note For a 16x2 display: row 0-1
 * @note For a 20x4 display: row 0-3
 *
 * Example usage:
 * @code
 * lcd_i2c_clear_row(0);  // Clear first row
 * lcd_i2c_clear_row(1);  // Clear second row
 * @endcode
 */
void lcd_i2c_clear_row(uint8_t row);

/**
 * @brief Clear multiple specific positions on the LCD display
 *
 * Clears characters at multiple specific positions by writing spaces to each
 * location. Uses parallel arrays where rows[i] and cols[i] define each position.
 *
 * @param rows Array of row positions (0-based)
 * @param cols Array of column positions (0-based)
 * @param count Number of positions to clear (length of arrays)
 *
 * @note Invalid row/column values will be clamped to display boundaries
 * @note The cursor position after completion is at the last cleared position
 *
 * Example usage:
 * @code
 * uint8_t rows[] = {0, 1, 2};
 * uint8_t cols[] = {12, 12, 5};
 * lcd_i2c_custom_clear(rows, cols, 3);  // Clear (0,12), (1,12), (2,5)
 * @endcode
 */
void lcd_i2c_custom_clear(const uint8_t rows[], const uint8_t cols[], uint8_t count);

/**
 * @brief Move cursor to home position (0, 0)
 *
 * Returns cursor to the top-left corner without clearing the display content.
 * This operation takes approximately 2ms to complete.
 */
void lcd_i2c_home(void);

/* Cursor */

/**
 * @brief Set cursor position on the LCD
 *
 * Positions the cursor at the specified row and column. Subsequent calls to
 * lcd_i2c_print() will output text starting from this position.
 *
 * @param row Row number (0-based, 0 to rows-1)
 * @param col Column number (0-based, 0 to cols-1)
 *
 * @note If row exceeds available rows, it will be clamped to the last row
 * @note For a 16x2 display: row 0-1, col 0-15
 */
void lcd_i2c_set_cursor(uint8_t row, uint8_t col);

/* Text */

/**
 * @brief Print a string at the current cursor position
 *
 * Outputs a null-terminated string to the LCD starting at the current cursor
 * position. The cursor automatically advances with each character.
 *
 * @param str Null-terminated string to display
 *
 * @note String will wrap or be cut off at display boundaries
 * @note Use lcd_i2c_set_cursor() to position text before printing
 */
void lcd_i2c_print(const char *str);

/* Backlight */

/**
 * @brief Turn on the LCD backlight
 *
 * Enables the backlight LED on displays equipped with PCF8574 backlight control.
 */
void lcd_i2c_backlight_on(void);

/**
 * @brief Turn off the LCD backlight
 *
 * Disables the backlight LED while keeping the display content visible
 * (depending on ambient light and display type).
 */
void lcd_i2c_backlight_off(void);

/**
 * @brief Create a custom character in CGRAM
 *
 * Defines a custom 5x8 pixel character and stores it in one of 8 available
 * CGRAM locations. Each character is defined by an 8-byte array where each
 * byte represents a row, and the lower 5 bits of each byte define the pixels.
 *
 * @param location CGRAM slot (0-7) where the character will be stored
 * @param charmap 8-byte bitmap array defining the character pattern
 *
 * @note Only locations 0-7 are valid; values above 7 will be clamped to 7
 * @note Each byte in charmap uses bits 0-4 (LSB) for pixel data
 * @note After creation, use lcd_i2c_print_custom() to display the character
 * @note This function moves the cursor to home position after completion
 *
 * Example - Create a heart character:
 * @code
 * uint8_t heart[8] = {
 *     0b00000,
 *     0b01010,
 *     0b11111,
 *     0b11111,
 *     0b01110,
 *     0b00100,
 *     0b00000,
 *     0b00000
 * };
 * lcd_i2c_create_char(0, heart);
 * lcd_i2c_print_custom(0);  // Display the heart
 * @endcode
 */
void lcd_i2c_create_char(uint8_t location, const uint8_t charmap[8]);

/**
 * @brief Print a previously created custom character at current cursor
 *
 * Outputs a custom character that was previously defined using
 * lcd_i2c_create_char(). The character is printed at the current cursor
 * position, which then advances by one column.
 *
 * @param location CGRAM slot (0-7) of the character to display
 *
 * @note Character must be created first using lcd_i2c_create_char()
 * @note Values above 7 will be clamped to 7
 */
void lcd_i2c_print_custom(uint8_t location);

#ifdef __cplusplus
}
#endif
