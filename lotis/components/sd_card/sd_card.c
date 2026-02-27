/**
 * @file sd_card.c
 * @brief ESP-IDF SD Card Component Implementation
 * 
 * This implementation uses ESP-IDF's VFS (Virtual File System) and SD card
 * driver with SPI interface. Compatible with 74LVC125A level shifter.
 * 
 * 74LVC125A Connection Notes:
 * - Supply voltage (VCC): 3.3V (compatible with ESP32)
 * - Input tolerance: Up to 5.5V (protects ESP32 from SD card voltage)
 * - Propagation delay: ~2.5ns @ 3.3V (minimal signal degradation)
 * - Enable pins (nOE): Tied to GND for continuous operation
 * 
 * @version 1.0
 * @date 2025
 */

#include "sd_card.h"
#include <string.h>
#include <errno.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_log.h"

static const char *TAG = "SD_CARD";

// Global variables
static sdmmc_card_t *sd_card = NULL;
static sd_card_config_t sd_config;
static bool is_initialized = false;
static const char *mount_point = "/sdcard";

/**
 * @brief Initialize SD card configuration with default values
 */
void sd_card_config_init(sd_card_config_t *config) {
    if (config == NULL) {
        return;
    }
    
    config->spi_host = SPI2_HOST;
    config->mosi_pin = -1;
    config->miso_pin = -1;
    config->sclk_pin = -1;
    config->cs_pin = -1;
    config->max_freq_khz = 20000;  // 20MHz default (safe for 74LVC125A)
    config->format_if_mount_failed = false;
    config->max_files = 5;
    config->allocation_unit_size = 0;
    config->use_1_line_mode = true;  // SPI mode uses 1-line
}

/**
 * @brief Setup SD card with specified configuration
 */
int sd_card_setup(sd_card_config_t *config) {
    if (config == NULL) {
        ESP_LOGE(TAG, "Configuration is NULL");
        return -1;
    }
    
    if (config->mosi_pin < 0 || config->miso_pin < 0 || 
        config->sclk_pin < 0 || config->cs_pin < 0) {
        ESP_LOGE(TAG, "Invalid pin configuration");
        return -1;
    }
    
    // Store configuration
    memcpy(&sd_config, config, sizeof(sd_card_config_t));
    
    ESP_LOGI(TAG, "SD card setup complete");
    ESP_LOGI(TAG, "SPI Pins - MOSI:%d MISO:%d SCLK:%d CS:%d", 
             config->mosi_pin, config->miso_pin, 
             config->sclk_pin, config->cs_pin);
    ESP_LOGI(TAG, "Max frequency: %d kHz", config->max_freq_khz);
    
    return 0;
}

/**
 * @brief Initialize and mount SD card
 */
int sd_card_init(void) {
    if (is_initialized) {
        ESP_LOGW(TAG, "SD card already initialized");
        return 1;
    }
    
    esp_err_t ret;
    
    // Configure SPI bus
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = sd_config.mosi_pin,
        .miso_io_num = sd_config.miso_pin,
        .sclk_io_num = sd_config.sclk_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    
    // Initialize SPI bus with DMA
    // Use SPI_DMA_CH_AUTO to allow sharing with other SPI devices
    // Track if we initialized the bus so we can safely clean up on error
    bool spi_bus_initialized_by_us = false;

    ret = spi_bus_initialize(sd_config.spi_host, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret == ESP_OK) {
        spi_bus_initialized_by_us = true;
    } else if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "SPI bus already initialized, reusing it");
        spi_bus_initialized_by_us = false;
    } else {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return 0;
    }
    
    // Mount filesystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = sd_config.format_if_mount_failed,
        .max_files = sd_config.max_files,
        .allocation_unit_size = sd_config.allocation_unit_size
    };
    
    // Configure SD/MMC slot for SPI mode
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    
    // Configure device slot
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = sd_config.cs_pin;
    
    // Set frequency (74LVC125A max propagation delay is ~6ns @ 3.3V,
    // so it supports high-speed operation up to ~40MHz theoretical,
    // but we use conservative 20MHz default for reliability)
    if (sd_config.max_freq_khz > 0) {
        host.max_freq_khz = sd_config.max_freq_khz;
    }
    
    ESP_LOGI(TAG, "Mounting filesystem...");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, 
                                   &mount_config, &sd_card);
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want to format the card, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SD card: %s", esp_err_to_name(ret));
        }
        // Free SPI bus only if we initialized it
        if (spi_bus_initialized_by_us) {
            spi_bus_free(sd_config.spi_host);
        }
        return 0;
    }
    
    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, sd_card);
    
    is_initialized = true;
    ESP_LOGI(TAG, "SD card initialized successfully");
    
    return ESP_OK;
}

/**
 * @brief Deinitialize and unmount SD card
 */
void sd_card_deinit(void) {
    if (!is_initialized) {
        return;
    }
    
    // Unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, sd_card);
    ESP_LOGI(TAG, "SD card unmounted");
    
    // Free SPI bus
    spi_bus_free(sd_config.spi_host);
    
    sd_card = NULL;
    is_initialized = false;
}

/**
 * @brief Build full file path
 */
static void build_full_path(char *buffer, size_t buffer_size, 
                           const char *path, const char *file_name) {
    snprintf(buffer, buffer_size, "%s/%s", path, file_name);
}

/**
 * @brief Read file contents from the SD card
 *
 * This function reads the specified file into a dynamically allocated memory buffer.
 * The caller is responsible for releasing this memory using `free()` after use
 * to prevent memory leaks.
 *
 * Example usage:
 * @code
 * size_t len = 0;
 * uint8_t *data = read_file("/sdcard", "my_data.txt", &len);
 * if (data) {
 *     const char *text = sd_utils_bytes_to_string(data, len);
 *     printf("File content:\n%s\n", text);
 *     free(data);  // Important: release memory after use
 * }
 * @endcode
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to read (e.g., "my_data.txt")
 * @param out_size Pointer to a variable to store the file size in bytes
 *                 (can be NULL if not needed)
 *
 * @return Pointer to a dynamically allocated buffer containing the file data,
 *         or NULL on failure. Caller must call `free()` on the returned buffer.
 */
uint8_t* read_file(const char *path, const char *file_name, size_t *out_size) {
    if (!is_initialized) {
        ESP_LOGE(TAG, "SD card not initialized");
        return NULL;
    }
    
    if (path == NULL || file_name == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return NULL;
    }
    
    char full_path[256];
    build_full_path(full_path, sizeof(full_path), path, file_name);
    
    ESP_LOGI(TAG, "Reading file: %s", full_path);
    
    FILE *f = fopen(full_path, "rb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return NULL;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (file_size < 0) {
        ESP_LOGE(TAG, "Failed to get file size");
        fclose(f);
        return NULL;
    }
    
    // Allocate buffer
    uint8_t *buffer = (uint8_t*)malloc(file_size + 1);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory");
        fclose(f);
        return NULL;
    }
    
    // Read file
    size_t read_size = fread(buffer, 1, file_size, f);
    fclose(f);
    
    if (read_size != file_size) {
        ESP_LOGE(TAG, "Failed to read complete file");
        free(buffer);
        return NULL;
    }
    
    buffer[file_size] = '\0';  // Null terminate
    
    if (out_size != NULL) {
        *out_size = file_size;
    }
    
    ESP_LOGI(TAG, "File read successfully, size: %ld bytes", file_size);
    return buffer;
}

/**
 * @brief Read a specific line (or the last line) from a text file on the SD card
 *
 * This function reads the specified line number (1-based) from a text file.
 * If `line_count` is set to `-1`, the function will return the **last line** in the file.
 *
 * The returned data is dynamically allocated and null-terminated.
 * The caller is responsible for calling `free()` on the returned pointer to
 * avoid memory leaks.
 *
 * Example usage:
 * @code
 * // Read the 3rd line
 * uint8_t *line = read_file_line("/sdcard", "log.txt", 3);
 * if (line) {
 *     printf("Line 3: %s\n", (char*)line);
 *     free(line);
 * }
 *
 * // Read the last line
 * uint8_t *last = read_file_line("/sdcard", "log.txt", -1);
 * if (last) {
 *     printf("Last line: %s\n", (char*)last);
 *     free(last);
 * }
 * @endcode
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to read (e.g., "log.txt")
 * @param line_count Line number to read (1 = first line, -1 = last line)
 * @return Pointer to dynamically allocated buffer containing the line,
 *         or NULL on failure. Caller must call `free()` when done.
 */
uint8_t *read_file_line(const char *path, const char *file_name, int line_count)
{
    if (!is_initialized) {
        ESP_LOGE(TAG, "SD card not initialized");
        return NULL;
    }

    if (path == NULL || file_name == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return NULL;
    }

    char full_path[256];
    build_full_path(full_path, sizeof(full_path), path, file_name);

    FILE *f = fopen(full_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", full_path);
        return NULL;
    }

    char line_buf[512];
    uint8_t *result = NULL;

    // Case 1: Read a specific line
    if (line_count > 0) {
        int current_line = 0;
        while (fgets(line_buf, sizeof(line_buf), f)) {
            current_line++;
            if (current_line == line_count) {
                size_t len = strlen(line_buf);
                result = (uint8_t *)malloc(len + 1);
                if (!result) {
                    ESP_LOGE(TAG, "Memory allocation failed");
                    fclose(f);
                    return NULL;
                }
                memcpy(result, line_buf, len + 1);
                break;
            }
        }
    }
    // Case 2: Read the last line
    else if (line_count == -1) {
        char last_line[512] = {0};
        while (fgets(line_buf, sizeof(line_buf), f)) {
            strncpy(last_line, line_buf, sizeof(last_line) - 1);
        }
        if (strlen(last_line) > 0) {
            size_t len = strlen(last_line);
            result = (uint8_t *)malloc(len + 1);
            if (!result) {
                ESP_LOGE(TAG, "Memory allocation failed");
                fclose(f);
                return NULL;
            }
            memcpy(result, last_line, len + 1);
        } else {
            ESP_LOGW(TAG, "File is empty");
        }
    } else {
        ESP_LOGE(TAG, "Invalid line_count value: %d", line_count);
    }

    fclose(f);

    if (!result) {
        if (line_count == -1)
            ESP_LOGW(TAG, "Failed to read last line (file may be empty)");
        else
            ESP_LOGW(TAG, "Line %d not found in file", line_count);
    } else {
        ESP_LOGI(TAG, "Read line %d successfully: %zu bytes", line_count, strlen((char*)result));
    }

    return result;
}

/**
 * @brief Read a file line by line, invoking a callback for each line
 *
 * This function opens the specified file and reads it line by line from the
 * first line to the last. For each line, the provided callback function is
 * invoked with the line content, line number (1-based), and user data.
 *
 * The callback can return `false` to stop reading early.
 *
 * Example usage - Print all lines:
 * @code
 * bool print_line(const char *line, int line_num, void *user_data) {
 *     printf("Line %d: %s", line_num, line);
 *     return true;  // continue reading
 * }
 *
 * int count = read_file_lines("/sdcard", "data.txt", print_line, NULL);
 * printf("Total lines: %d\n", count);
 * @endcode
 *
 * Example usage - Search for a keyword:
 * @code
 * typedef struct {
 *     const char *keyword;
 *     int found_line;
 * } search_ctx_t;
 *
 * bool search_line(const char *line, int line_num, void *user_data) {
 *     search_ctx_t *ctx = (search_ctx_t *)user_data;
 *     if (strstr(line, ctx->keyword) != NULL) {
 *         ctx->found_line = line_num;
 *         return false;  // stop reading
 *     }
 *     return true;  // continue
 * }
 *
 * search_ctx_t ctx = { .keyword = "ERROR", .found_line = -1 };
 * read_file_lines("/sdcard", "log.txt", search_line, &ctx);
 * if (ctx.found_line > 0) {
 *     printf("Found at line %d\n", ctx.found_line);
 * }
 * @endcode
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to read (e.g., "log.txt")
 * @param callback Function to call for each line (must not be NULL)
 * @param user_data Optional pointer passed to callback (can be NULL)
 * @return Number of lines read, or -1 on error
 */
int read_file_lines(const char *path, const char *file_name, sd_line_callback_t callback, void *user_data)
{
    if (!is_initialized) {
        ESP_LOGE(TAG, "SD card not initialized");
        return -1;
    }

    if (path == NULL || file_name == NULL || callback == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return -1;
    }

    char full_path[256];
    build_full_path(full_path, sizeof(full_path), path, file_name);

    FILE *f = fopen(full_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", full_path);
        return -1;
    }

    char line_buf[512];
    int line_number = 0;
    bool should_continue = true;

    while (should_continue && fgets(line_buf, sizeof(line_buf), f) != NULL) {
        line_number++;
        should_continue = callback(line_buf, line_number, user_data);
    }

    fclose(f);

    ESP_LOGI(TAG, "Read %d lines from file: %s", line_number, full_path);
    return line_number;
}

/**
 * @brief Create a new file with initial content
 */
int create_new_file(const char *path, const char *file_name, const char *data) {
    if (!is_initialized) {
        ESP_LOGE(TAG, "SD card not initialized");
        return -1;
    }

    if (path == NULL || file_name == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return -1;
    }

    char full_path[256];
    build_full_path(full_path, sizeof(full_path), path, file_name);

    ESP_LOGI(TAG, "Creating file: %s", full_path);

    FILE *f = fopen(full_path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to create file: %s (errno: %d - %s)",
                 full_path, errno, strerror(errno));
        return -1;
    }

    if (data != NULL) {
        size_t data_len = strlen(data);
        if (data_len > 0) {
            size_t written = fwrite(data, 1, data_len, f);
            if (written != data_len) {
                ESP_LOGE(TAG, "Failed to write initial data: wrote %zu/%zu bytes", written, data_len);
                fclose(f);
                return -1;
            }
        }
    }

    fclose(f);
    ESP_LOGI(TAG, "File created successfully");
    return 0;
}

/**
 * @brief Write data to file (overwrites existing content)
 *
 * This function writes the given string data to a file on the SD card.
 * The file content will be fully overwritten.
 *
 * For improved reliability, the function forces the write buffers to flush
 * using `fflush()` and `fsync()` before closing the file. This ensures data
 * is physically written to the SD card, reducing the risk of data loss if
 * the device loses power right after writing.
 *
 * Example usage:
 * @code
 * if (write_file("/sdcard", "log.txt", "System started\n") == 0) {
 *     ESP_LOGI(TAG, "Write OK");
 * } else {
 *     ESP_LOGE(TAG, "Write failed");
 * }
 * @endcode
 *
 * Performance notes:
 * - `fflush()` pushes the C library's internal buffer to the filesystem.
 * - `fsync()` ensures that the FAT and data sectors are written to the SD card.
 * - Both add a small delay (~5–20 ms depending on card speed) but guarantee persistence.
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to write (e.g., "data.txt")
 * @param data String data to write to the file
 * @return 0 on success, -1 on failure
 */
int write_file(const char *path, const char *file_name, const char *data)
{
    if (!is_initialized) {
        ESP_LOGE(TAG, "SD card not initialized");
        return -1;
    }

    if (path == NULL || file_name == NULL || data == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return -1;
    }

    char full_path[256];
    build_full_path(full_path, sizeof(full_path), path, file_name);

    ESP_LOGI(TAG, "Writing to file: %s", full_path);

    FILE *f = fopen(full_path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return -1;
    }

    size_t len = strlen(data);
    size_t written = fwrite(data, 1, len, f);

    if (written != len) {
        ESP_LOGE(TAG, "Incomplete write: %zu/%zu bytes", written, len);
        fclose(f);
        return -1;
    }

    // 🧩 Flush buffers for data integrity
    fflush(f);                // flush stdio buffer
    fsync(fileno(f));         // ensure SD sectors written

    fclose(f);

    ESP_LOGI(TAG, "File written and flushed successfully");
    return 0;
}

/**
 * @brief Append data to a file on the SD card (adds a new line and the new content)
 *
 * This function opens the specified file in append mode and writes the provided
 * string data at the end of the file. If the file does not exist, it will be created.
 * A newline character is automatically added before the appended data for readability.
 *
 * For improved reliability, the function flushes data to disk using `fflush()` and
 * `fsync()` before closing the file. This ensures that the appended data is
 * physically written to the SD card and not just cached in memory.
 *
 * Example usage:
 * @code
 * if (append_write_file("/sdcard", "log.txt", "New log entry") == 0) {
 *     ESP_LOGI(TAG, "Append successful");
 * } else {
 *     ESP_LOGE(TAG, "Append failed");
 * }
 * @endcode
 *
 * Performance notes:
 * - `fflush()` forces buffered data to be written from the C I/O layer.
 * - `fsync()` ensures data and FAT metadata are physically written to the SD card.
 * - These add a small delay (~5–20 ms depending on card speed) but increase safety.
 * - For frequent log writes, you may omit `fsync()` to improve performance.
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to append to (e.g., "log.txt")
 * @param data String data to append
 * @return 0 on success, -1 on failure
 */
int append_write_file(const char *path, const char *file_name, const char *data)
{
    if (!is_initialized) {
        ESP_LOGE(TAG, "SD card not initialized");
        return -1;
    }

    if (path == NULL || file_name == NULL || data == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return -1;
    }

    char full_path[256];
    build_full_path(full_path, sizeof(full_path), path, file_name);

    ESP_LOGI(TAG, "Appending to file: %s", full_path);

    FILE *f = fopen(full_path, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for appending");
        return -1;
    }

    // Write a newline followed by the data
    int written = fprintf(f, "\n%s", data);
    if (written < 0) {
        ESP_LOGE(TAG, "Failed to append data");
        fclose(f);
        return -1;
    }

    // 🧩 Flush and sync to ensure data is saved to the SD card
    fflush(f);
    fsync(fileno(f));

    fclose(f);

    ESP_LOGI(TAG, "Data appended and flushed successfully");
    return 0;
}



/**
 * @brief Delete a file
 */
int delete_file(const char *path, const char *file_name) {
    if (!is_initialized) {
        ESP_LOGE(TAG, "SD card not initialized");
        return -1;
    }
    
    if (path == NULL || file_name == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return -1;
    }
    
    char full_path[256];
    build_full_path(full_path, sizeof(full_path), path, file_name);
    
    ESP_LOGI(TAG, "Deleting file: %s", full_path);
    
    struct stat st;
    if (stat(full_path, &st) != 0) {
        ESP_LOGE(TAG, "File does not exist");
        return -1;
    }
    
    if (unlink(full_path) != 0) {
        ESP_LOGE(TAG, "Failed to delete file");
        return -1;
    }
    
    ESP_LOGI(TAG, "File deleted successfully");
    return 0;
}

/**
 * @brief Create a new directory
 */
int create_new_directory(const char *path, const char *folder_name) {
    if (!is_initialized) {
        ESP_LOGE(TAG, "SD card not initialized");
        return -1;
    }
    
    if (path == NULL || folder_name == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return -1;
    }
    
    char full_path[256];
    build_full_path(full_path, sizeof(full_path), path, folder_name);
    
    ESP_LOGI(TAG, "Creating directory: %s", full_path);
    
    struct stat st;
    if (stat(full_path, &st) == 0) {
        ESP_LOGW(TAG, "Directory already exists");
        return 0;
    }
    
    if (mkdir(full_path, 0775) != 0) {
        ESP_LOGE(TAG, "Failed to create directory");
        return -1;
    }
    
    ESP_LOGI(TAG, "Directory created successfully");
    return 0;
}

/**
 * @brief Get available space on SD card in bytes
 */
int64_t get_available_size_b(void) {
    if (!is_initialized || sd_card == NULL) {
        ESP_LOGE(TAG, "SD card not initialized");
        return -1;
    }
    
    FATFS *fs;
    DWORD free_clusters;
    
    if (f_getfree("0:", &free_clusters, &fs) != FR_OK) {
        ESP_LOGE(TAG, "Failed to get free space");
        return -1;
    }
    
    int64_t total_bytes = ((int64_t)free_clusters * fs->csize * fs->ssize);
    
    ESP_LOGI(TAG, "Available space: %lld bytes", total_bytes);
    return total_bytes;
}

/**
 * @brief Get total SD card size in bytes
 */
int64_t get_sd_size_b(void) {
    if (!is_initialized || sd_card == NULL) {
        ESP_LOGE(TAG, "SD card not initialized");
        return -1;
    }
    
    int64_t total_bytes = ((int64_t)sd_card->csd.capacity) * sd_card->csd.sector_size;
    
    ESP_LOGI(TAG, "Total SD card size: %lld bytes", total_bytes);
    return total_bytes;
}

/**
 * @brief Convert bytes to megabytes
 */
int sd_utils_bytes_to_mb(int64_t bytes) {
    if (bytes < 0) {
        return 0;
    }
    return (int)(bytes / (1024 * 1024));
}

/**
 * @brief Convert bytes to gigabytes
 */
int sd_utils_bytes_to_gb(int64_t bytes) {
    if (bytes < 0) {
        return 0;
    }
    return (int)(bytes / (1024 * 1024 * 1024));
}

/**
 * @brief Convert raw byte buffer (uint8_t*) to printable C string (char*)
 * 
 * @param bytes Pointer to byte buffer (e.g. from file)
 * @param len   Length of byte buffer
 * @return char* Dynamically allocated null-terminated string.
 *         Must be freed by the caller.
 *         Returns NULL if allocation or input invalid.
 */
const char *sd_utils_bytes_to_string(uint8_t *bytes, size_t len)
{
    if (!bytes || len == 0) {
        return NULL;
    }

    // Allocate memory (+1 for '\0')
    char *str = malloc(len + 1);
    if (!str) {
        return NULL;
    }

    // Copy bytes as-is
    memcpy(str, bytes, len);

    // Ensure null termination (safe for printing)
    str[len] = '\0';

    return str;
}

/**
 * @brief Check if a file exists on the SD card
 *
 * This function checks whether the specified file exists in the given directory.
 * It uses the `stat()` system call to verify file existence.
 *
 * Example usage:
 * @code
 * if (file_exists("/sdcard", "config.txt")) {
 *     ESP_LOGI(TAG, "Config file found");
 * } else {
 *     ESP_LOGI(TAG, "Config file not found, creating default");
 *     create_new_file("/sdcard", "config", "txt", NULL, 0);
 * }
 * @endcode
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to check (e.g., "config.txt")
 * @return true if file exists, false otherwise (or if SD card not initialized)
 *
 * @note This function returns false if the SD card is not initialized or if
 *       parameters are invalid. Check initialization status separately if needed.
 */
bool file_exists(const char *path, const char *file_name) {
    if (!is_initialized) {
        ESP_LOGE(TAG, "SD card not initialized");
        return false;
    }
    
    if (path == NULL || file_name == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return false;
    }
    
    char full_path[256];
    build_full_path(full_path, sizeof(full_path), path, file_name);
    
    struct stat st;
    if (stat(full_path, &st) == 0) {
        // Check if it's a regular file (not a directory)
        if (S_ISREG(st.st_mode)) {
            ESP_LOGD(TAG, "File exists: %s", full_path);
            return true;
        } else {
            ESP_LOGW(TAG, "Path exists but is not a file: %s", full_path);
            return false;
        }
    }
    
    ESP_LOGD(TAG, "File does not exist: %s", full_path);
    return false;
}