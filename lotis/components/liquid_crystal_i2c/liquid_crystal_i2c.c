#include "liquid_crystal_i2c.h"
#include "freertos/projdefs.h"
#include "i2c_helper.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

/* HD44780 commands */
#define LCD_CLEAR_DISPLAY   0x01
#define LCD_RETURN_HOME     0x02
#define LCD_ENTRY_MODE_SET  0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_FUNCTION_SET    0x20
#define LCD_SET_DDRAM_ADDR  0x80

/* Flags */
#define LCD_ENTRY_LEFT      0x02
#define LCD_DISPLAY_ON      0x04
#define LCD_2LINE           0x08
#define LCD_4BIT_MODE       0x00

/* PCF8574 bits */
#define LCD_BACKLIGHT       0x08
#define LCD_ENABLE          0x04
#define LCD_RS              0x01

static lcd_i2c_config_t lcd;
static uint8_t backlight = LCD_BACKLIGHT;

/* ---- low-level helpers ---- */

static void lcd_write(uint8_t data)
{
    i2c_helper_write_byte(lcd.i2c_port, lcd.i2c_address, data | backlight);
}

static void lcd_pulse(uint8_t data)
{
    lcd_write(data | LCD_ENABLE);
	vTaskDelay(pdMS_TO_TICKS(1));
    ets_delay_us(1);
    lcd_write(data & ~LCD_ENABLE);
    ets_delay_us(50);
}

static void lcd_write4(uint8_t nibble, uint8_t mode)
{
    uint8_t data = (nibble & 0xF0) | mode;
    lcd_pulse(data);
}

static void lcd_cmd(uint8_t cmd)
{
    lcd_write4(cmd & 0xF0, 0);
    lcd_write4((cmd << 4) & 0xF0, 0);
}

static void lcd_data(uint8_t data)
{
    lcd_write4(data & 0xF0, LCD_RS);
    lcd_write4((data << 4) & 0xF0, LCD_RS);
}

/* ---- public API ---- */

esp_err_t lcd_i2c_init(const lcd_i2c_config_t *cfg)
{
    if (!cfg) return ESP_ERR_INVALID_ARG;
    lcd = *cfg;

    vTaskDelay(pdMS_TO_TICKS(50));

    /* Init sequence */
    lcd_write4(0x30, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write4(0x30, 0);
    ets_delay_us(150);
    lcd_write4(0x30, 0);
    lcd_write4(0x20, 0);   // 4-bit mode

    uint8_t function = LCD_FUNCTION_SET | LCD_4BIT_MODE;
    if (lcd.rows > 1) function |= LCD_2LINE;

    lcd_cmd(function);
    lcd_cmd(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON);
    lcd_cmd(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT);
    lcd_i2c_clear();

    return ESP_OK;
}

void lcd_i2c_clear(void)
{
    lcd_cmd(LCD_CLEAR_DISPLAY);
    vTaskDelay(pdMS_TO_TICKS(2));
}

void lcd_i2c_clear_row(uint8_t row)
{
    // Clamp row to valid range
    if (row >= lcd.rows) {
        row = lcd.rows - 1;
    }

    // Move cursor to beginning of the row
    lcd_i2c_set_cursor(row, 0);

    // Fill the entire row with spaces
    for (uint8_t i = 0; i < lcd.cols; i++) {
        lcd_data(' ');
    }

    // Move cursor back to beginning of the cleared row
    lcd_i2c_set_cursor(row, 0);
}

void lcd_i2c_custom_clear(const uint8_t rows[], const uint8_t cols[], uint8_t count)
{
    // Clear each specified position
    for (uint8_t i = 0; i < count; i++) {
        uint8_t row = rows[i];
        uint8_t col = cols[i];

        // Clamp values to valid range
        if (row >= lcd.rows) {
            row = lcd.rows - 1;
        }
        if (col >= lcd.cols) {
            col = lcd.cols - 1;
        }

        // Set cursor to position and write space
        lcd_i2c_set_cursor(row, col);
        lcd_data(' ');
    }
}

void lcd_i2c_home(void)
{
    lcd_cmd(LCD_RETURN_HOME);
    vTaskDelay(pdMS_TO_TICKS(2));
}

void lcd_i2c_set_cursor(uint8_t row, uint8_t col)
{
    static const uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    if (row >= lcd.rows) row = lcd.rows - 1;

    lcd_cmd(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]));
}

void lcd_i2c_print(const char *str)
{
    while (*str) {
        lcd_data((uint8_t)*str++);
    }
}

void lcd_i2c_backlight_on(void)
{
    backlight = LCD_BACKLIGHT;
    lcd_write(0);
}

void lcd_i2c_backlight_off(void)
{
    backlight = 0x00;
    lcd_write(0);
}

/**
 * @brief Create a custom character and display it
 * @param location CGRAM location 0-7
 * @param charmap 8-byte bitmap array for the character
 */
void lcd_i2c_create_char(uint8_t location, const uint8_t charmap[8])
{
    if (location > 7) location = 7;  // Only 0-7 valid
    lcd_cmd(0x40 | (location << 3));  // Set CGRAM address

    for (uint8_t i = 0; i < 8; i++) {
        lcd_data(charmap[i]);
    }

    lcd_i2c_home(); // Optional: return cursor home
}

/**
 * @brief Print a custom character already loaded in CGRAM
 * @param location CGRAM location 0-7
 */
void lcd_i2c_print_custom(uint8_t location)
{
    if (location > 7) location = 7;
    lcd_data(location);
}
