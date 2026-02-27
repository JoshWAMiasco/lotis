/**
 * @file sd_card.h
 * @brief SD Card Driver Component for ESP-IDF with SPI Interface
 *
 * This component provides a complete SD card interface for ESP32 using the SPI
 * bus. It supports SD/SDHC/SDXC cards and includes optional support for the
 * 74LVC125A level shifter for voltage translation and signal integrity.
 *
 * Features:
 * - SPI-based SD card communication (1-line mode)
 * - FAT filesystem support via ESP-IDF VFS
 * - File operations: read, write, append, delete
 * - Directory management: create directories, check file existence
 * - Card information: capacity, available space
 * - Line-by-line file reading for text files
 * - Automatic card initialization and mounting
 * - Optional automatic formatting on mount failure
 * - DMA support for efficient data transfer
 *
 * Hardware Connection:
 * - Connect SD card to ESP32 via SPI (MOSI, MISO, SCLK, CS pins)
 * - Optional: Use 74LVC125A level shifter for signal buffering
 * - Default SPI frequency: 20MHz (safe for most SD cards)
 * - Supports SPI2_HOST and SPI3_HOST (HSPI/VSPI)
 *
 * 74LVC125A Level Shifter (Optional):
 * - 5V tolerant inputs (protects ESP32 from SD card voltage)
 * - 3.3V outputs (compatible with ESP32)
 * - Schmitt-trigger inputs for noise immunity
 * - Low propagation delay (~2.5ns @ 3.3V)
 * - Connect nOE pins to GND for continuous operation
 *
 * File System:
 * - Uses FAT32 filesystem
 * - Default mount point: /sdcard
 * - Supports long file names
 * - Configurable maximum open files (default: 5)
 *
 * Example usage:
 * @code
 * sd_card_config_t sd_conf = {
 *     .spi_host = SPI2_HOST,
 *     .mosi_pin = 23,
 *     .miso_pin = 19,
 *     .sclk_pin = 18,
 *     .cs_pin = 5,
 *     .max_freq_khz = 20000,
 *     .format_if_mount_failed = false,
 *     .max_files = 5,
 *     .allocation_unit_size = 16384,
 *     .use_1_line_mode = true
 * };
 *
 * sd_card_setup(&sd_conf);
 * sd_card_init();
 *
 * write_file("/sdcard", "test.txt", "Hello World!");
 * size_t len;
 * uint8_t *data = read_file("/sdcard", "test.txt", &len);
 * free(data);
 *
 * sd_card_deinit();
 * @endcode
 *
 * @version 1.0
 * @date 2025
 */

#ifndef SD_CARD_H
#define SD_CARD_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Sample Config
sd_card_config_t sd_conf = {
        .spi_host = SPI2_HOST,              // SPI2_HOST or SPI3_HOST (HSPI/VSPI)
        .mosi_pin = 23,                     // GPIO23 - Master Out Slave In
        .miso_pin = 19,                     // GPIO19 - Master In Slave Out
        .sclk_pin = 18,                     // GPIO18 - SPI Clock
        .cs_pin = 5,                        // GPIO5 - Chip Select
        .max_freq_khz = 20000,              // 20MHz (safe for 74LVC125A)
        .format_if_mount_failed = false,    // Don't auto-format (set true to format on fail)
        .max_files = 5,                     // Maximum 5 files open simultaneously
        .allocation_unit_size = 16384,      // 16KB allocation units (0 = auto)
        .use_1_line_mode = true             // Use 1-line SPI mode (always true for SPI)
    };
*/

/**
 * @brief SD card configuration structure
 */
typedef struct {
    int spi_host;           ///< SPI host device (HSPI_HOST or VSPI_HOST)
    int mosi_pin;           ///< MOSI pin number
    int miso_pin;           ///< MISO pin number
    int sclk_pin;           ///< SCLK pin number
    int cs_pin;             ///< Chip Select pin number
    int max_freq_khz;       ///< Maximum SPI frequency in kHz (default: 20000)
    bool format_if_mount_failed; ///< Format SD card if mount fails
    int max_files;          ///< Maximum number of open files (default: 5)
    int allocation_unit_size; ///< Allocation unit size for formatting (0 = auto)
    bool use_1_line_mode;   ///< Use 1-line SD mode instead of 4-line (for SPI)
} sd_card_config_t;

/**
 * @brief Initialize SD card configuration with default values
 * 
 * @param config Pointer to configuration structure
 */
void sd_card_config_init(sd_card_config_t *config);

/**
 * @brief Setup SD card with specified configuration
 * 
 * This function configures the SPI bus and prepares for SD card initialization.
 * The 74LVC125A should be connected as follows:
 * - ESP32 GPIO -> 74LVC125A inputs (1A, 2A, 3A, 4A)
 * - 74LVC125A outputs (1Y, 2Y, 3Y, 4Y) -> SD Card
 * - nOE pins -> GND (always enabled)
 * - VCC -> 3.3V
 * 
 * @param config Pointer to SD card configuration
 * @return 0 on success, -1 on failure
 */
int sd_card_setup(sd_card_config_t *config);

/**
 * @brief Initialize and mount SD card
 * 
 * @return 1 on success, 0 on failure
 */
int sd_card_init(void);

/**
 * @brief Deinitialize and unmount SD card
 */
void sd_card_deinit(void);

/**
 * @brief Read file contents
 * 
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to read
 * @param out_size Pointer to store the size of read data (optional, can be NULL)
 * @return Pointer to dynamically allocated buffer containing file data, 
 *         or NULL on failure. Caller must free the returned buffer.
 */
uint8_t* read_file(const char *path, const char *file_name, size_t *out_size);

/**
 * @brief Read a specific line from a text file
 *
 * Reads the specified line number (1-based) from a file.
 * The returned buffer is dynamically allocated and must be freed by the caller.
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name (e.g., "data.txt")
 * @param line_count Line number to read (1 = first line)
 * @return Pointer to dynamically allocated buffer containing the line,
 *         or NULL if not found or on error.
 */
uint8_t *read_file_line(const char *path, const char *file_name, int line_count);

/**
 * @brief Callback function type for line-by-line file reading
 *
 * @param line The current line content (null-terminated, includes newline if present)
 * @param line_number The current line number (1-based)
 * @param user_data User-provided context pointer
 * @return true to continue reading, false to stop early
 */
typedef bool (*sd_line_callback_t)(const char *line, int line_number, void *user_data);

/**
 * @brief Read a file line by line, invoking a callback for each line
 *
 * This function opens the specified file and reads it line by line from the
 * first line to the last. For each line, the provided callback function is
 * invoked with the line content, line number, and user data.
 *
 * The callback can return `false` to stop reading early (useful for searching
 * or processing until a condition is met).
 *
 * This approach is memory-efficient as it only keeps one line in memory at a
 * time, making it suitable for large files on memory-constrained devices.
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to read (e.g., "log.txt")
 * @param callback Function to call for each line
 * @param user_data Optional pointer passed to callback (can be NULL)
 * @return Number of lines read, or -1 on error
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
 * Example usage - Collect lines into array:
 * @code
 * typedef struct {
 *     char **lines;
 *     int count;
 *     int capacity;
 * } collect_ctx_t;
 *
 * bool collect_line(const char *line, int line_num, void *user_data) {
 *     collect_ctx_t *ctx = (collect_ctx_t *)user_data;
 *     if (ctx->count < ctx->capacity) {
 *         ctx->lines[ctx->count] = strdup(line);
 *         ctx->count++;
 *     }
 *     return true;
 * }
 *
 * char *lines[100];
 * collect_ctx_t ctx = { .lines = lines, .count = 0, .capacity = 100 };
 * read_file_lines("/sdcard", "data.txt", collect_line, &ctx);
 * // Don't forget to free each line when done
 * @endcode
 */
int read_file_lines(const char *path, const char *file_name, sd_line_callback_t callback, void *user_data);

/**
 * @brief Create a new file with initial content
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to create (including extension, e.g., "config.txt")
 * @param data Initial string data to write (can be NULL for empty file)
 * @return 0 on success, -1 on failure
 */
int create_new_file(const char *path, const char *file_name, const char *data);

/**
 * @brief Write data to file (overwrites existing content)
 * 
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to write
 * @param data String data to write
 * @return 0 on success, -1 on failure
 */
int write_file(const char *path, const char *file_name, const char *data);

/**
 * @brief Append data to file (adds new line and content)
 * 
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to append to
 * @param data String data to append
 * @return 0 on success, -1 on failure
 */
int append_write_file(const char *path, const char *file_name, const char *data);

/**
 * @brief Delete a file
 * 
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to delete
 * @return 0 on success, -1 on failure
 */
int delete_file(const char *path, const char *file_name);

/**
 * @brief Create a new directory
 * 
 * @param path Parent directory path (e.g., "/sdcard")
 * @param folder_name Directory name to create
 * @return 0 on success, -1 on failure
 */
int create_new_directory(const char *path, const char *folder_name);

/**
 * @brief Get available space on SD card in bytes
 * 
 * @return Available space in bytes, or -1 on error
 */
int64_t get_available_size_b(void);

/**
 * @brief Get total SD card size in bytes
 * 
 * @return Total size in bytes, or -1 on error
 */
int64_t get_sd_size_b(void);

/**
 * @brief Convert bytes to megabytes
 * 
 * @param bytes Size in bytes
 * @return Size in megabytes (integer division)
 */
int sd_utils_bytes_to_mb(int64_t bytes);

/**
 * @brief Convert bytes to gigabytes
 * 
 * @param bytes Size in bytes
 * @return Size in gigabytes (integer division)
 */
int sd_utils_bytes_to_gb(int64_t bytes);

/**
 * @brief Convert bytes to human-readable string
 * 
 * @param bytes Size in bytes
 * @return Pointer to static string buffer containing formatted size,
 *         or "Error" on failure. Do not free this pointer.
 */
const char* sd_utils_bytes_to_string(uint8_t *bytes, size_t len);

/**
 * @brief Check if a file exists on the SD card
 *
 * Verifies whether the specified file exists in the given directory using
 * the stat() system call. Returns false if the path exists but is a directory.
 *
 * @param path Directory path (e.g., "/sdcard")
 * @param file_name File name to check (e.g., "config.txt")
 * @return true if file exists and is a regular file
 * @return false if file doesn't exist, is a directory, SD card not initialized, or on error
 *
 * @note This function only checks for regular files, not directories
 *
 * Example:
 * @code
 * if (file_exists("/sdcard", "config.txt")) {
 *     ESP_LOGI(TAG, "Config file found");
 * } else {
 *     create_new_file("/sdcard", "config.txt", NULL);
 * }
 * @endcode
 */
bool file_exists(const char *path, const char *file_name);

#ifdef __cplusplus
}
#endif

#endif /* SD_CARD_H */