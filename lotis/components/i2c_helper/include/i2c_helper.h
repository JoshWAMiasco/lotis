#pragma once

/**
 * @file i2c_helper.h
 * @brief I2C Helper Component for ESP-IDF
 *
 * This component provides a simplified interface for I2C master mode operations
 * on ESP32 devices. It wraps the ESP-IDF I2C driver to provide easy-to-use
 * functions for common I2C transactions.
 *
 * Features:
 * - I2C bus initialization and deinitialization
 * - Simple byte and multi-byte write operations
 * - Read operations from I2C devices
 * - Combined write-read transactions (register read pattern)
 * - Automatic timeout handling (100ms default)
 *
 * Example usage:
 * @code
 * i2c_bus_cfg_t cfg = {
 *     .port = I2C_NUM_0,
 *     .sda_io = 21,
 *     .scl_io = 22,
 *     .clk_speed = 100000
 * };
 * i2c_helper_init(&cfg);
 * i2c_helper_write_byte(I2C_NUM_0, 0x50, 0xAB);
 * @endcode
 */

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief I2C bus configuration structure
 */
typedef struct {
    i2c_port_t port;      /**< I2C port number (I2C_NUM_0 or I2C_NUM_1) */
    int sda_io;           /**< GPIO number for SDA line */
    int scl_io;           /**< GPIO number for SCL line */
    uint32_t clk_speed;   /**< I2C clock frequency in Hz (e.g., 100000 for 100kHz) */
} i2c_bus_cfg_t;

/* Bus control */

/**
 * @brief Initialize I2C bus in master mode
 *
 * Configures and installs the I2C driver with the specified parameters.
 * Enables internal pull-ups for both SDA and SCL lines.
 *
 * @param cfg Pointer to I2C bus configuration structure
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if cfg is NULL
 * @return ESP_ERR_INVALID_STATE if port is already initialized (returns ESP_OK)
 * @return Other ESP-IDF error codes from i2c_driver_install
 */
esp_err_t i2c_helper_init(const i2c_bus_cfg_t *cfg);

/**
 * @brief Deinitialize I2C bus and free resources
 *
 * @param port I2C port number to deinitialize
 * @return ESP_OK on success
 * @return ESP-IDF error codes from i2c_driver_delete
 */
esp_err_t i2c_helper_deinit(i2c_port_t port);

/* Basic transactions */

/**
 * @brief Write a single byte to an I2C device
 *
 * Convenience function for writing a single byte of data.
 *
 * @param port I2C port number
 * @param addr I2C device address (7-bit, not shifted)
 * @param data Byte value to write
 * @return ESP_OK on success
 * @return ESP-IDF error codes on failure (timeout, NACK, etc.)
 */
esp_err_t i2c_helper_write_byte(i2c_port_t port, uint8_t addr, uint8_t data);

/**
 * @brief Write multiple bytes to an I2C device
 *
 * Sends a START condition, device address with write bit, data bytes,
 * and STOP condition. Timeout is set to 100ms.
 *
 * @param port I2C port number
 * @param addr I2C device address (7-bit, not shifted)
 * @param data Pointer to data buffer to write
 * @param len Number of bytes to write
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if data is NULL or len is 0
 * @return ESP-IDF error codes on failure (timeout, NACK, etc.)
 */
esp_err_t i2c_helper_write(i2c_port_t port, uint8_t addr,
                           const uint8_t *data, size_t len);

/**
 * @brief Read multiple bytes from an I2C device
 *
 * Sends a START condition, device address with read bit, reads data bytes
 * with NACK on last byte, and STOP condition. Timeout is set to 100ms.
 *
 * @param port I2C port number
 * @param addr I2C device address (7-bit, not shifted)
 * @param data Pointer to buffer to store read data
 * @param len Number of bytes to read
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if data is NULL or len is 0
 * @return ESP-IDF error codes on failure (timeout, NACK, etc.)
 */
esp_err_t i2c_helper_read(i2c_port_t port, uint8_t addr,
                          uint8_t *data, size_t len);

/* Combined write → read */

/**
 * @brief Write then read from an I2C device (register read pattern)
 *
 * Performs a combined transaction: writes data (typically a register address),
 * issues repeated START, then reads data back. This is commonly used for
 * reading device registers. Timeout is set to 100ms.
 *
 * @param port I2C port number
 * @param addr I2C device address (7-bit, not shifted)
 * @param wdata Pointer to data to write (e.g., register address)
 * @param wlen Number of bytes to write
 * @param rdata Pointer to buffer to store read data
 * @param rlen Number of bytes to read
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if wdata or rdata is NULL
 * @return ESP-IDF error codes on failure (timeout, NACK, etc.)
 */
esp_err_t i2c_helper_write_read(i2c_port_t port, uint8_t addr,
                                const uint8_t *wdata, size_t wlen,
                                uint8_t *rdata, size_t rlen);

#ifdef __cplusplus
}
#endif
