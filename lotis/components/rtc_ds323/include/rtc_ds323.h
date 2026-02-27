/**
 * @file rtc_ds323.h
 * @brief DS3231 Real-Time Clock (RTC) Driver Component for ESP-IDF
 *
 * This component provides a driver for the DS3231 high-precision I2C real-time
 * clock module. The DS3231 is a low-cost, extremely accurate RTC with an
 * integrated temperature-compensated crystal oscillator (TCXO) and crystal.
 *
 * Features:
 * - Set and read time (hours, minutes, seconds)
 * - Set and read date (year, month, day)
 * - Support for both 12-hour (AM/PM) and 24-hour time formats
 * - Temperature reading (Celsius and Fahrenheit)
 * - Battery backup capability (coin cell keeps time during power loss)
 * - I2C interface using i2c_helper component
 * - Accuracy: ±2ppm from 0°C to +40°C
 *
 * Hardware Connection:
 * - I2C Address: 0x68 (fixed)
 * - Connect SDA and SCL to ESP32 I2C pins
 * - 3.3V or 5V compatible
 * - Optional: Connect 32kHz output pin if needed
 * - Battery backup: CR2032 coin cell
 *
 * Example usage:
 * @code
 * rtc_config_t rtc_cfg = {
 *     .i2c_port = I2C_NUM_0,
 *     .i2c_address = DS3231_I2C_ADDR,
 *     .i2c_frequency = 100000,
 *     .sda = GPIO_NUM_21,
 *     .scl = GPIO_NUM_22,
 *     .enable_sda_pullup = true,
 *     .enable_scl_pullup = true
 * };
 * rtc_ds323_init(&rtc_cfg);
 * rtc_set_time(14, 30, 0);  // 2:30:00 PM
 * rtc_set_date(2026, 1, 5); // January 5, 2026
 * @endcode
 *
 * @author
 * @date 2026
 */

#ifndef RTC_DS323_H
#define RTC_DS323_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c.h"
#include "i2c_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DS3231 I2C Address */
#define DS3231_I2C_ADDR 0x68

/* DS3231 Register Addresses */
#define DS3231_REG_SECONDS      0x00
#define DS3231_REG_MINUTES      0x01
#define DS3231_REG_HOURS        0x02
#define DS3231_REG_DAY          0x03
#define DS3231_REG_DATE         0x04
#define DS3231_REG_MONTH        0x05
#define DS3231_REG_YEAR         0x06
#define DS3231_REG_ALARM1_SEC   0x07
#define DS3231_REG_ALARM1_MIN   0x08
#define DS3231_REG_ALARM1_HOUR  0x09
#define DS3231_REG_ALARM1_DATE  0x0A
#define DS3231_REG_ALARM2_MIN   0x0B
#define DS3231_REG_ALARM2_HOUR  0x0C
#define DS3231_REG_ALARM2_DATE  0x0D
#define DS3231_REG_CONTROL      0x0E
#define DS3231_REG_STATUS       0x0F
#define DS3231_REG_AGING        0x10
#define DS3231_REG_TEMP_MSB     0x11
#define DS3231_REG_TEMP_LSB     0x12

/* Time Format */
#define RTC_FORMAT_24HR 24
#define RTC_FORMAT_12HR 12

/* AM/PM */
#define RTC_AM 0
#define RTC_PM 1

/* Status Codes */
#define RTC_OK           0
#define RTC_ERR_FAIL    -1
#define RTC_ERR_INVALID -2
#define RTC_ERR_TIMEOUT -3
#define RTC_ERR_I2C     -4

/**
 * @brief DS3231 RTC configuration structure
 */
typedef struct {
    i2c_port_t i2c_port;           /**< I2C port number (I2C_NUM_0 or I2C_NUM_1) */
    uint8_t i2c_address;           /**< I2C address (DS3231_I2C_ADDR = 0x68) */
    uint32_t i2c_frequency;        /**< I2C clock frequency in Hz (typically 100000) */
    gpio_num_t sda;                /**< GPIO number for SDA line */
    gpio_num_t scl;                /**< GPIO number for SCL line */
    bool enable_sda_pullup;        /**< Enable internal pull-up on SDA */
    bool enable_scl_pullup;        /**< Enable internal pull-up on SCL */
} rtc_config_t;

/**
 * @brief Time and date structure for DS3231
 */
typedef struct {
    uint8_t hour;                  /**< Hour (0-23 in 24hr mode, 1-12 in 12hr mode) */
    uint8_t minute;                /**< Minutes (0-59) */
    uint8_t second;                /**< Seconds (0-59) */
    uint8_t format;                /**< Time format (RTC_FORMAT_24HR or RTC_FORMAT_12HR) */
    uint8_t is_pm;                 /**< AM/PM flag (RTC_AM or RTC_PM, only for 12hr mode) */
    uint8_t month;                 /**< Month (1-12) */
    uint8_t day;                   /**< Day of month (1-31) */
    uint16_t year;                 /**< Year (e.g., 2026) */
} rtc_time_t;

/* Initialization */

/**
 * @brief Initialize the DS3231 RTC driver
 *
 * Stores the configuration for the DS3231 RTC. The I2C bus should be
 * initialized separately before calling this function to avoid double
 * driver installation.
 *
 * @param config Pointer to RTC configuration structure
 * @return RTC_OK on success
 * @return RTC_ERR_INVALID if config is NULL
 *
 * @note The I2C bus must be initialized separately using i2c_helper_init()
 * @note This function only stores the configuration and does not communicate with the device
 */
int rtc_ds323_init(const rtc_config_t *config);

/* Time/Date */

/**
 * @brief Set the current time on the RTC
 *
 * Sets the hours, minutes, and seconds on the DS3231. Values are converted
 * to BCD format internally before writing to the device.
 *
 * @param hr Hour (0-23 for 24hr mode, 1-12 for 12hr mode)
 * @param min Minutes (0-59)
 * @param sec Seconds (0-59)
 * @return RTC_OK on success
 * @return RTC_ERR_I2C on I2C communication failure
 */
int rtc_set_time(uint8_t hr, uint8_t min, uint8_t sec);

/**
 * @brief Set the current date on the RTC
 *
 * Sets the year, month, and day on the DS3231. Year is stored as an offset
 * from 2000 (e.g., 2026 is stored as 26).
 *
 * @param year Full year (e.g., 2026)
 * @param month Month (1-12, where 1=January)
 * @param day Day of month (1-31)
 * @return RTC_OK on success
 * @return RTC_ERR_I2C on I2C communication failure
 *
 * @note Years before 2000 are not supported
 */
int rtc_set_date(uint16_t year, uint8_t month, uint8_t day);

/**
 * @brief Set the time format (12-hour or 24-hour)
 *
 * Configures the RTC to use either 12-hour (with AM/PM) or 24-hour time format.
 *
 * @param format RTC_FORMAT_12HR or RTC_FORMAT_24HR
 * @return RTC_OK on success
 * @return RTC_ERR_I2C on I2C communication failure
 */
int rtc_set_time_format(uint8_t format);

/**
 * @brief Set AM/PM indicator for 12-hour mode
 *
 * Sets the AM/PM flag when using 12-hour time format. Only applicable
 * when the RTC is configured for 12-hour mode.
 *
 * @param is_pm RTC_AM (0) for AM, RTC_PM (1) for PM
 * @return RTC_OK on success
 * @return RTC_ERR_I2C on I2C communication failure
 *
 * @note This function automatically enables 12-hour mode
 */
int rtc_set_ampm(uint8_t is_pm);

/* Getters */

/**
 * @brief Read current time and date from the RTC
 *
 * Reads all time and date registers from the DS3231 and populates the
 * provided rtc_time_t structure. Values are converted from BCD format.
 *
 * @param time Pointer to rtc_time_t structure to store the result
 * @return RTC_OK on success
 * @return RTC_ERR_INVALID if time pointer is NULL
 * @return RTC_ERR_I2C on I2C communication failure
 */
int rtc_get_time(rtc_time_t *time);

/**
 * @brief Read temperature from the DS3231 in Celsius
 *
 * Reads the internal temperature sensor of the DS3231. The sensor has
 * 0.25°C resolution. The returned value is rounded to the nearest integer.
 *
 * @return Temperature in degrees Celsius (as integer)
 * @return RTC_ERR_I2C on I2C communication failure
 *
 * @note Temperature range: -40°C to +85°C
 * @note Accuracy: ±3°C
 */
int rtc_get_temp_c(void);

/**
 * @brief Read temperature from the DS3231 in Fahrenheit
 *
 * Reads the internal temperature sensor and converts to Fahrenheit.
 * This is a convenience function that calls rtc_get_temp_c() internally.
 *
 * @return Temperature in degrees Fahrenheit (as integer)
 * @return RTC_ERR_I2C on I2C communication failure
 */
int rtc_get_temp_f(void);

/**
 * @brief Get Unix epoch timestamp from RTC
 *
 * Reads the current time and date from the DS3231 and converts it to Unix
 * epoch time (seconds since January 1, 1970 00:00:00 UTC). This is useful
 * for timestamp validation and time comparisons.
 *
 * @return Unix epoch timestamp (10-digit seconds since Jan 1, 1970)
 * @return RTC_ERR_I2C on I2C communication failure
 * @return RTC_ERR_FAIL on conversion error
 *
 * @note Assumes RTC time is set correctly
 * @note Does not account for timezone - uses RTC time as-is
 * @note Valid range: 1970-2099 (limited by DS3231 year range)
 */
int rtc_get_epoch(void);

/* Utilities */

/**
 * @brief Get a formatted date/time string
 *
 * Returns a pointer to a static buffer containing a formatted date/time string.
 *
 * @return Pointer to static datetime string buffer
 *
 * @warning The returned pointer points to a static buffer that may be overwritten
 *          by subsequent calls to this function
 * @note This function returns the buffer address but does not update its contents.
 *       The buffer must be populated elsewhere in the application.
 */
const char* rtc_get_datetime_str(void);

#ifdef __cplusplus
}
#endif

#endif /* RTC_DS323_H */
