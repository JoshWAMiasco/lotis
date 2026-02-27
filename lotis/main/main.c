/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "gap.h"
#include "gatt_svc.h"
#include "led.h"
#include "hal/spi_types.h"
#include "i2c_helper.h"
#include "liquid_crystal_i2c.h"
#include "rtc_ds323.h"
#include "sd_card.h"
#include "sim900.h"
#include "ble_protocol.h"
#include "cJSON.h"
#include "soc/gpio_num.h"


/* ========================================================================== */
/*                           GPIO Pin Definitions                             */
/* ========================================================================== */

// I2C pins for LCD and RTC communication
#define SDA_PORT 8
#define SCL_PORT 9

// SPI pins for SD card communication
#define SD_CS_PORT 10
#define SD_SCK_PORT 12
#define SD_MOSI_PORT 11
#define SD_MISO_PORT 13

// UART pins for GSM SIM900 module
#define TX_PORT 43
#define RX_PORT 44

// LCD I2C address
#define LCD_ADDRESS 0x27

// Relay GPIO pin definitions (8 relays)
#define RELAY_1 GPIO_NUM_4
#define RELAY_2 GPIO_NUM_5
#define RELAY_3 GPIO_NUM_6
#define RELAY_4 GPIO_NUM_7
#define RELAY_5 GPIO_NUM_15
#define RELAY_6 GPIO_NUM_16
#define RELAY_7 GPIO_NUM_17
#define RELAY_8 GPIO_NUM_18
#define BUZZER GPIO_NUM_20




/* ========================================================================== */
/*                           Global Variables                                 */
/* ========================================================================== */

uint8_t lcd_addr = 0x27;

// Global configuration stored as cJSON object
static cJSON *app_config = NULL;
TaskHandle_t upload_data_task_handler = NULL;
TaskHandle_t time_task_handler = NULL;

/* ========================================================================== */
/*                         Function Declarations                              */
/* ========================================================================== */

// Configuration helper function declarations
static const char* config_get_string(const char *key);
static int config_get_number(const char *key, int default_value);
static int config_set_string(const char *key, const char *value);
static int config_set_number(const char *key, int value);
static int save_config(cJSON *config);


/* ========================================================================== */
/*                         Hardware Configuration                             */
/* ========================================================================== */

// I2C bus configuration for LCD and RTC
static const i2c_bus_cfg_t bus = {
    .port = I2C_NUM_0,
    .sda_io = SDA_PORT,
    .scl_io = SCL_PORT,
    .clk_speed = 100000  // 100kHz
};

// SIM900 GSM module UART configuration

static const sim900_config_t sim_conf = {
    .baud_rate = 115200,
    .uart_port = UART_NUM_0,
    .rx_pin = RX_PORT,
    .tx_pin = TX_PORT,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122
};


// LCD 20x4 I2C configuration
static const lcd_i2c_config_t lcd_cfg = {
    .i2c_port = bus.port,
    .i2c_address = LCD_ADDRESS,
    .cols = 20,
    .rows = 4
};

// DS3231 RTC configuration
static const rtc_config_t rtc_cfg = {
    .i2c_port = bus.port,
    .i2c_address = DS3231_I2C_ADDR,
    .i2c_frequency = bus.clk_speed,
    .sda = bus.sda_io,
    .scl = bus.scl_io,
    .enable_sda_pullup = true,
    .enable_scl_pullup = true,
};

/* ========================================================================== */
/*                         LCD Custom Icons                                   */
/* ========================================================================== */

// Custom icon slot definitions for easy tracking
#define TEMPERATURE_ICON 0
#define CONNECTED_BLE_ICON 1
#define DISCONNECTED_BLE_ICON 2
#define DATA_RECEIVED_ICON 3
#define DATA_SENT_ICON 4
#define SIGNAL_ICON 5

// Custom character: Temperature icon
static const uint8_t temperature_icon[8] = {
    0b00111,
    0b00100,
    0b00111,
    0b00100,
    0b00100,
    0b01110,
    0b01110,
    0b01110
};

// Custom character: Bluetooth connected icon
static const uint8_t connected_ble_icon[8] = {
    0b01110,
    0b11111,
    0b11111,
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b00000
};

// Custom character: Bluetooth disconnected icon
static const uint8_t disconnected_ble_icon[8] = {
    0b01110,
    0b10001,
    0b10001,
    0b01010,
    0b10001,
    0b10001,
    0b11111,
    0b00000
};

// Custom character: Data received icon (downward arrow)
static const uint8_t data_received_icon[8] = {
    0b00100,
    0b01110,
    0b10101,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100
};

// Custom character: Data sent icon (upward arrow)
static const uint8_t data_sent_icon[8] = {
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b10101,
    0b01110,
    0b00100
};

static const uint8_t signal_icon[8] = {
  0b11111,
  0b10101,
  0b10101,
  0b01110,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};


/* ========================================================================== */
/*                         Utility Functions                                  */
/* ========================================================================== */

/**
 * @brief Delay function using FreeRTOS task delay
 * @param ms Milliseconds to delay
 */
void delay_ms(int ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/**
 * @brief Store custom icons in LCD memory (positions 0-4)
 */
void store_lcd_icons() {
    lcd_i2c_create_char(TEMPERATURE_ICON, temperature_icon);
    lcd_i2c_create_char(CONNECTED_BLE_ICON, connected_ble_icon);
    lcd_i2c_create_char(DISCONNECTED_BLE_ICON, disconnected_ble_icon);
    lcd_i2c_create_char(DATA_RECEIVED_ICON, data_received_icon);
    lcd_i2c_create_char(DATA_SENT_ICON, data_sent_icon);
	lcd_i2c_create_char(SIGNAL_ICON, signal_icon);
}

/**
 * @brief Get GPIO pin for relay number
 * @param relay_num Relay number (1-8)
 * @return GPIO number, or -1 if invalid
 */
static gpio_num_t get_relay_gpio(uint8_t relay_num) {
    switch (relay_num) {
        case 1: return RELAY_1;
        case 2: return RELAY_2;
        case 3: return RELAY_3;
        case 4: return RELAY_4;
        case 5: return RELAY_5;
        case 6: return RELAY_6;
        case 7: return RELAY_7;
        case 8: return RELAY_8;
        default: return (gpio_num_t)-1;
    }
}




/**
 * @brief Turn off all 8 relay outputs
 * Sets all relay GPIOs to HIGH (relay modules are typically active-low)
 */
void off_all_relay() {
    gpio_set_level(RELAY_1, 1);
    gpio_set_level(RELAY_2, 1);
    gpio_set_level(RELAY_3, 1);
    gpio_set_level(RELAY_4, 1);
    gpio_set_level(RELAY_5, 1);
    gpio_set_level(RELAY_6, 1);
    gpio_set_level(RELAY_7, 1);
    gpio_set_level(RELAY_8, 1);
}

/**
 * @brief Setup relay states from configuration
 * Reads relay states from app_config and applies them to physical relays
 */
static void setup_relay_status(void)
{
    lcd_i2c_set_cursor(0, 0);
    lcd_i2c_print("Initialize Relay");
    lcd_i2c_set_cursor(1, 0);
    lcd_i2c_print("Loading States...");

    if (app_config == NULL) {
        ESP_LOGE(TAG, "Config not initialized, skipping relay setup");
        lcd_i2c_clear_row(1);
        lcd_i2c_set_cursor(1, 0);
        lcd_i2c_print("Config Error!");
        delay_ms(1000);
        return;
    }

    // Load and apply relay states from config
    for (int i = 1; i <= 8; i++) {
        char relay_key[16];
        snprintf(relay_key, sizeof(relay_key), "relay_%d", i);

        // Get relay state from config (default to 0/OFF)
        int relay_state = config_get_number(relay_key, 0);

        // Get GPIO for this relay
        gpio_num_t gpio = get_relay_gpio(i);
        if (gpio != (gpio_num_t)-1) {
            // Set relay state (active-low: 0=ON, 1=OFF)
            gpio_set_level(gpio, relay_state ? 0 : 1);
            ESP_LOGI(TAG, "Relay %d restored to %s from config", i, relay_state ? "ON" : "OFF");
        }
    }

	lcd_i2c_clear_row(1);
    lcd_i2c_set_cursor(1, 0);
    lcd_i2c_print("Relay Ready");
}




/**
 * @brief Initialize relay GPIOs as outputs
 * Configures all 8 relay pins as output mode and sets them to OFF state (HIGH)
 * Relay modules are typically active-low, so HIGH = OFF
 */
void init_relay_and_buzzer() {
    // Configure GPIO pins for all 8 relays
    gpio_config_t io_conf = {
        .pin_bit_mask = ((1ULL << RELAY_1) |
                         (1ULL << RELAY_2) |	
                         (1ULL << RELAY_3) |
                         (1ULL << RELAY_4) |
                         (1ULL << RELAY_5) |
                         (1ULL << RELAY_6) |
                         (1ULL << RELAY_7) |
                         (1ULL << RELAY_8) |
						 (1ULL << BUZZER)),
						 
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    // Apply configuration
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure relay GPIOs: %d", ret);
        return;
    }

    // Turn off all relays (set to HIGH for active-low relays)
    off_all_relay();

    ESP_LOGI(TAG, "Relay GPIOs initialized successfully");
}





/* ========================================================================== */
/*                            LCD Display Functions                           */
/* ========================================================================== */

static void set_ble_connection_status_icon(bool connected) {
	lcd_i2c_set_cursor(2, 7);
	if(connected) {
		lcd_i2c_print_custom(CONNECTED_BLE_ICON);
	} else {
		lcd_i2c_print_custom(DISCONNECTED_BLE_ICON);
	}
}

static void set_ble_connection_status_display(uint8_t type) {
	lcd_i2c_set_cursor(2, 8);
	uint8_t rows[12] = {2, 2, 2, 2,2,2,2,2,2,2,2,2};
    uint8_t cols[12] = {8, 9, 10, 11, 12, 13,14,15,16,17,18,19};
 	lcd_i2c_custom_clear(rows, cols, 12);
	lcd_i2c_set_cursor(2, 8);
	printf("Connection status: %d", type);
	switch (type) {
		case 0:
			lcd_i2c_print("Ready");
			break;
		case 1:
			lcd_i2c_print("Admin");
			break;
		case 2:
			lcd_i2c_print("EHT");
			break;
		case 3:
			lcd_i2c_print("Error");
			break;
		
	}
	for (int i = 0; i < 2; i++) {
		gpio_set_level(BUZZER, 1);
		delay_ms(50);
		gpio_set_level(BUZZER, 0);
		delay_ms(50);
	}
}




/**
 * @brief Display main dashboard screen
 * Shows terminal number from config and temperature icon
 */
static void dashboard_display() {

    lcd_i2c_clear();
    lcd_i2c_set_cursor(0, 0);

    // Get terminal number from configuration
    const char *terminal_number = config_get_string("terminal_number");
    if (terminal_number != NULL) {
        lcd_i2c_print(terminal_number);
    } else {
        lcd_i2c_print("LT-PH-NE-01");  // Fallback if config not available
        ESP_LOGW(TAG, "Terminal number not found in config, using default");
    }

    lcd_i2c_set_cursor(2, 0);
    lcd_i2c_print_custom(TEMPERATURE_ICON);
	lcd_i2c_set_cursor(3, 0);
	lcd_i2c_print_custom(SIGNAL_ICON);
	set_ble_connection_status_icon(false);
	set_ble_connection_status_display(0);
}



/**
 * @brief Display current temperature from RTC
 * Reads temperature from DS3231 and displays on LCD
 */
static void display_latest_temperature() {
    int temp_c = rtc_get_temp_c();
    lcd_i2c_set_cursor(1, 1);
    char line3[32];

    // Check if temperature reading is valid
    if (temp_c != RTC_ERR_I2C && temp_c != RTC_ERR_FAIL) {
        snprintf(line3, sizeof(line3), "%dC", temp_c);
    } else {
        snprintf(line3, sizeof(line3), "--");
    }

    lcd_i2c_set_cursor(2, 2);
    lcd_i2c_print(line3);
}

/**
 * @brief Display current time and date from RTC
 * Reads time from DS3231 and displays on LCD along with temperature
 */
static void display_latest_time() {
    rtc_time_t t;

    if (rtc_get_time(&t) == RTC_OK) {
        char line1[32];

        // Format time based on 12hr or 24hr format
        if (t.format == RTC_FORMAT_12HR) {
            snprintf(line1, sizeof(line1), "%02d:%02d %s %02d-%02d-%02d",
                     t.hour, t.minute, t.is_pm ? "PM" : "AM", t.month, t.day, t.year);
        } else {
            snprintf(line1, sizeof(line1), "%02d:%02d %s %04d-%02d-%02d",
                     t.hour, t.minute, t.is_pm ? "PM" : "AM", t.year, t.month, t.day);
        }

        lcd_i2c_set_cursor(1, 0);
        lcd_i2c_print(line1);
        display_latest_temperature();
    }
}

static void	display_latest_signal() {
	// Get signal quality
	char *csq = send_at_command(AT_SIGNAL_QUALITY);
	if (csq == NULL) {
		lcd_i2c_set_cursor(3, 2);
		lcd_i2c_print("-");
		return;
	}

	// Parse RSSI value from response format: "+CSQ: <rssi>,<ber>"
	int rssi = 99; // Default to unknown
	char *csq_ptr = strstr(csq, "+CSQ:");
	if (csq_ptr != NULL) {
		// Skip "+CSQ: " to get the rssi value
		csq_ptr += 6; // length of "+CSQ: "
		rssi = atoi(csq_ptr);
	}

	ESP_LOGI(TAG, "Signal quality RSSI: %d", rssi);

	// Map RSSI to signal strength level based on documentation:
	// 0-1: Very poor (unusable) -> 0 (display "-")
	// 2-9: Poor -> 1
	// 10-14: Fair -> 2
	// 15-19: Good -> 3
	// 20-31: Very good/Excellent -> 4
	// 99: Unknown/No signal -> 0 (display "-")
	int signal_level = 0;
	if (rssi == 99 || rssi <= 1) {
		signal_level = 0; // No signal or very poor
	} else if (rssi >= 2 && rssi <= 9) {
		signal_level = 1; // Poor
	} else if (rssi >= 10 && rssi <= 14) {
		signal_level = 2; // Fair
	} else if (rssi >= 15 && rssi <= 19) {
		signal_level = 3; // Good
	} else if (rssi >= 20 && rssi <= 31) {
		signal_level = 4; // Very good
	}

	lcd_i2c_set_cursor(3, 2);
	if (signal_level == 0) {
		lcd_i2c_print("-");
	} else {
		char level_str[2];
		snprintf(level_str, sizeof(level_str), "%d", signal_level);
		lcd_i2c_print(level_str);
	}
}

/* ========================================================================== */
/*                         FreeRTOS Tasks                                     */
/* ========================================================================== */

/**
 * @brief FreeRTOS task to update time display every minute
 * @param pvParameters Task parameters (unused)
 */
void time_update_task(void *pvParameters) {
    while(1) {
        // Wait for 60 seconds (1 minute)
        delay_ms(60000);
        // Update the time display
        display_latest_time();
		delay_ms(500);
		display_latest_temperature();
		display_latest_signal();
    }
}


void data_upload_task(void *pvParameters) {
	while(1) {
		lcd_i2c_set_cursor(3, 7);
		lcd_i2c_print_custom(DATA_RECEIVED_ICON);
		delay_ms(1000);
		lcd_i2c_set_cursor(3, 7);
		lcd_i2c_print(" ");
		delay_ms(1000);
	}
}

void start_data_upload_rtos() {
	delay_ms(500);
	xTaskCreate(
		data_upload_task, 
		"DataUploadTask",
		4096, 
		NULL,
 		2, 
		&upload_data_task_handler
	);
}

void start_time_task() {
	delay_ms(500);
	xTaskCreate(
        time_update_task,       // Task function
        "TimeUpdateTask",       // Task name
        3072,                   // Stack size (bytes)
        NULL,                   // Parameters
        1,                      // Priority
        &time_task_handler                    // Task handle
    );
}

void stop_time_task() {
	vTaskDelete(time_task_handler);
	time_task_handler = NULL;
	delay_ms(500);
}

void stop_data_upload_rtos() {
	lcd_i2c_set_cursor(3, 7);
	lcd_i2c_print(" ");
	uint8_t rows[12] = {3, 3, 3, 3,3,3,3,3,3,3,3,3};
    uint8_t cols[12] = {8, 9, 10, 11, 12, 13,14,15,16,17,18,19};
 	lcd_i2c_custom_clear(rows, cols, 12);

	vTaskDelete(upload_data_task_handler);
    upload_data_task_handler = NULL;
}


/* ========================================================================== */
/*                         SD CARD CONFIG DATA                                */
/* ========================================================================== */

/*
 * Configuration System Usage Guide:
 *
 * The global 'app_config' cJSON object stores all application configuration.
 * It is loaded from SD card on startup and persists in memory.
 *
 * READING VALUES:
 *   const char *terminal = config_get_string("terminal_number");
 *   int relay1_state = config_get_number("relay_1", 0);
 *
 * WRITING VALUES:
 *   config_set_string("terminal_number", "LT-NE-PH-02");
 *   config_set_number("relay_1", 1);
 *   save_config(app_config);  // Save changes to SD card
 *
 * DIRECT cJSON ACCESS (for advanced operations):
 *   cJSON *item = cJSON_GetObjectItem(app_config, "relay_1");
 *   if (cJSON_IsNumber(item)) {
 *       ESP_LOGI(TAG, "Relay 1: %d", item->valueint);
 *   }
 *
 * MEMORY MANAGEMENT:
 *   - app_config is initialized in init_memory() and persists throughout program
 *   - Do NOT call cJSON_Delete(app_config) unless shutting down
 *   - Helper functions manage memory internally
 */

/**
 * @brief Create default configuration JSON object
 * @return cJSON* pointer to config object, or NULL on failure
 */
static cJSON* create_default_config(void) {
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to create JSON object");
        return NULL;
    }

    // Add default configuration values
    cJSON_AddStringToObject(root, "terminal_number", "LT-NE-PH-01");
    cJSON_AddNumberToObject(root, "relay_1", 0);
    cJSON_AddNumberToObject(root, "relay_2", 0);
    cJSON_AddNumberToObject(root, "relay_3", 0);
    cJSON_AddNumberToObject(root, "relay_4", 0);
    cJSON_AddNumberToObject(root, "relay_5", 0);
    cJSON_AddNumberToObject(root, "relay_6", 0);
    cJSON_AddNumberToObject(root, "relay_7", 0);
    cJSON_AddNumberToObject(root, "relay_8", 0);
    return root;
}

/**
 * @brief Save configuration to SD card
 * @param config cJSON object to save
 * @return 0 on success, -1 on failure
 */
static int save_config(cJSON *config) {
    if (config == NULL) {
        ESP_LOGE(TAG, "Config is NULL");
        return -1;
    }

    // Convert to formatted string (pretty print)
    char *json_str = cJSON_Print(config);
    if (json_str == NULL) {
        ESP_LOGE(TAG, "Failed to serialize JSON");
        return -1;
    }

    ESP_LOGI(TAG, "Saving config:\n%s", json_str);

    // Write to SD card (using .txt extension due to FAT filesystem limitations)
    int result = write_file("/sdcard", "config.txt", json_str);

    // Cleanup
    free(json_str);

    if (result == 0) {
        ESP_LOGI(TAG, "Config saved successfully");
    } else {
        ESP_LOGE(TAG, "Failed to write config file");
    }

    return result;
}

/**
 * @brief Load configuration from SD card
 * @return cJSON* pointer to config object, or NULL on failure
 */
static cJSON* load_config(void) {
    size_t len = 0;
    uint8_t *data = read_file("/sdcard", "config.txt", &len);
    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to read config file");
        return NULL;
    }

    // Parse JSON
    cJSON *config = cJSON_Parse((char*)data);
    free(data);

    if (config == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON config");
        return NULL;
    }

    ESP_LOGI(TAG, "Config loaded successfully");
    return config;
}

/**
 * @brief Initialize configuration (load from SD or create default)
 * @return ESP_OK on success, ESP_FAIL on failure
 */
static int init_memory(void) {
    ESP_LOGI(TAG, "Initializing configuration...");

    // Check if config file exists
    if (file_exists("/sdcard", "config.txt")) {
        ESP_LOGI(TAG, "Config file found, loading...");
        app_config = load_config();
        if (app_config == NULL) {
            ESP_LOGE(TAG, "Failed to load config, creating default");
            app_config = create_default_config();
            if (app_config != NULL) {
                save_config(app_config);
            }
        }
    } else {
        ESP_LOGI(TAG, "No config file found, creating default...");
        app_config = create_default_config();
        if (app_config != NULL) {
            save_config(app_config);
        }
    }

    if (app_config == NULL) {
        ESP_LOGE(TAG, "Failed to initialize configuration");
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
 * @brief Get string value from configuration
 * @param key Configuration key
 * @return const char* value or NULL if not found
 */
static const char* config_get_string(const char *key) {
    if (app_config == NULL) {
        ESP_LOGE(TAG, "Config not initialized");
        return NULL;
    }

    cJSON *item = cJSON_GetObjectItem(app_config, key);
    if (item == NULL || !cJSON_IsString(item)) {
        ESP_LOGW(TAG, "Config key '%s' not found or not a string", key);
        return NULL;
    }

    return item->valuestring;
}

/**
 * @brief Get number value from configuration
 * @param key Configuration key
 * @param default_value Default value if key not found
 * @return int value from config or default_value
 */
static int config_get_number(const char *key, int default_value) {
    if (app_config == NULL) {
        ESP_LOGE(TAG, "Config not initialized");
        return default_value;
    }

    cJSON *item = cJSON_GetObjectItem(app_config, key);
    if (item == NULL || !cJSON_IsNumber(item)) {
        ESP_LOGW(TAG, "Config key '%s' not found or not a number, using default: %d", key, default_value);
        return default_value;
    }

    return item->valueint;
}

/**
 * @brief Set string value in configuration
 * @param key Configuration key
 * @param value String value to set
 * @return 0 on success, -1 on failure
 */
static int config_set_string(const char *key, const char *value) {
    if (app_config == NULL) {
        ESP_LOGE(TAG, "Config not initialized");
        return -1;
    }

    cJSON *item = cJSON_GetObjectItem(app_config, key);
    if (item != NULL) {
        // Update existing key
        cJSON_SetValuestring(item, value);
    } else {
        // Add new key
        cJSON_AddStringToObject(app_config, key, value);
    }

    return 0;
}

/**
 * @brief Set number value in configuration
 * @param key Configuration key
 * @param value Number value to set
 * @return 0 on success, -1 on failure
 */
static int config_set_number(const char *key, int value) {
    if (app_config == NULL) {
        ESP_LOGE(TAG, "Config not initialized");
        return -1;
    }

    cJSON *item = cJSON_GetObjectItem(app_config, key);
    if (item != NULL) {
        // Update existing key
        cJSON_SetNumberValue(item, value);
    } else {
        // Add new key
        cJSON_AddNumberToObject(app_config, key, value);
    }

    return 0;
}


/**
 * @brief Callback for keypad events
 * Logs key press and release events
 */


/* ========================================================================== */
/*                   BLE Configuration and  Definitions                       */
/* ========================================================================== */

/* Library function declarations */
void ble_store_config_init(void);

/* Private function declarations */
static void on_stack_reset(int reason);
static void on_stack_sync(void);
static void nimble_host_config_init(void);
static void nimble_host_task(void *param);

/* Private functions */
/*
 *  Stack event callback functions
 *      - on_stack_reset is called when host resets BLE stack due to errors
 *      - on_stack_sync is called when host has synced with controller
 */
static void on_stack_reset(int reason) {
    /* On reset, print reset reason to console */
    ESP_LOGI(TAG, "nimble stack reset, reset reason: %d", reason);
}

static void on_stack_sync(void) {
    /* On stack sync, do advertising initialization */
    adv_init();
}

static void nimble_host_config_init(void) {
    /* Set host callbacks */
    ble_hs_cfg.reset_cb = on_stack_reset;
    ble_hs_cfg.sync_cb = on_stack_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Store host configuration */
    ble_store_config_init();
}

static void nimble_host_task(void *param) {
    /* Task entry log */
    ESP_LOGI(TAG, "nimble host task has been started!");

    /* This function won't return until nimble_port_stop() is executed */
    nimble_port_run();

    /* Clean up at exit */
    vTaskDelete(NULL);
}

/* ========================================================================== */
/*                         Application Functions                              */
/* ========================================================================== */

static void display_process_status(char* info) {
	uint8_t rows[12] = {3, 3, 3, 3,3,3,3,3,3,3,3,3};
    uint8_t cols[12] = {8, 9, 10, 11, 12, 13,14,15,16,17,18,19};
	lcd_i2c_custom_clear(rows, cols, 12);
	lcd_i2c_set_cursor(3, 8);
	lcd_i2c_print(info);
	
}

static void clear_process_status() {
	uint8_t rows[12] = {3, 3, 3, 3,3,3,3,3,3,3,3,3};
    uint8_t cols[12] = {8, 9, 10, 11, 12, 13,14,15,16,17,18,19};
	lcd_i2c_custom_clear(rows, cols, 12);	
}


static void send_sms(char* message) {
	const char* contacts = config_get_string("contacts");
	if (contacts && *contacts) {
		// Set SMS text mode
		send_at_command(AT_SMS_FORMAT "1\r");

		// Set character set to GSM
		send_at_command(AT_SET_CHARSET "\"GSM\"\r");


		// Create a copy of contacts string for parsing (strtok modifies the string)
		char contacts_copy[256];
		strncpy(contacts_copy, contacts, sizeof(contacts_copy) - 1);
		contacts_copy[sizeof(contacts_copy) - 1] = '\0';

		// Parse comma-separated phone numbers and send SMS to each
		char *phone_number = strtok(contacts_copy, ",");
		int sms_count = 0;

		while (phone_number != NULL) {
			// Trim leading whitespace
			while (*phone_number == ' ') phone_number++;

			// Skip empty entries
			if (*phone_number == '\0') {
				phone_number = strtok(NULL, ",");
				continue;
			}

			sms_count++;
			ESP_LOGI(TAG, "Sending SMS %d to: %s", sms_count, phone_number);

			// Send SMS command with phone number
			char sms_cmd[128];
			snprintf(sms_cmd, sizeof(sms_cmd), "%s\"%s\"\r", AT_SMS_SEND, phone_number);
			char *response = send_at_command(sms_cmd);

			if (response && strstr(response, ">")) {
				// Module ready for message text
				response = send_at_command(message);
				ESP_LOGI(TAG, "SMS %d response: %s", sms_count, response ? response : "NULL");
			} else {
				ESP_LOGW(TAG, "SMS %d failed to get prompt for: %s", sms_count, phone_number);
			}

			// Small delay between SMS sends
			delay_ms(500);

			// Get next phone number
			phone_number = strtok(NULL, ",");
		}

		ESP_LOGI(TAG, "Sent SMS to %d contacts", sms_count);
	} else {
		ESP_LOGW(TAG, "No contacts configured for SMS");
	}
}

static esp_err_t set_date_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t is_pm){
	 // Validate data
    if(month < 1 || month > 12) {
        printf("Invalid month: %d\n", month);
        return ESP_FAIL;
    }
    if(day < 1 || day > 31) {
        printf("Invalid day: %d\n", day);
        return ESP_FAIL;
    }
    if(hour < 1 || hour > 12) {  // Assuming 12-hour format
        printf("Invalid hour: %d\n", hour);
        return ESP_FAIL;
    }
    if(minute > 59) {
        printf("Invalid minute: %d\n", minute);
        return ESP_FAIL;
    }
		
	ESP_LOGI(TAG, "Set RTC: %04d-%02d-%02d %02d:%02d %s",
                     year, month, day, hour, minute,
                     is_pm ? "PM" : "AM");
	rtc_set_time(hour, minute, 0);
	delay_ms(50);
	rtc_set_date(year, month, day);
	delay_ms(50);
	rtc_set_time_format(12);
	delay_ms(50);
	rtc_set_ampm(is_pm);
	delay_ms(50);
	display_latest_time();
	return ESP_OK;  
}

static esp_err_t get_relay_status() {
	printf("\n=== Getting Relay Status ===\n");
	
	int relays[8];
	uint8_t relay_status[8];
	const char *relay_keys[] = {
		"relay_1", "relay_2", "relay_3", "relay_4",
		"relay_5", "relay_6", "relay_7", "relay_8"
	};
	

	for(int i = 0; i < 8; i++) {
		relays[i] = config_get_number(relay_keys[i], 2);
		
		if(relays[i] == 2) {
			printf("[ERROR] Relay %d: Configuration failed or not set\n", i + 1);
			return ESP_FAIL;
		}
		
		relay_status[i] = (relays[i] == 0) ? 0x00 : 0x01;
		
		printf("Relay %d: GPIO=%d, Status=%s (0x%02X)\n", 
			i + 1, 
			relays[i], 
			relay_status[i] ? "ON" : "OFF",
			relay_status[i]);
	}
	
	
	uint8_t response[12] = {
		0x4C, 0x54,              // Header
		0x48,                    // Request type (Hardware)
		0x02,                    // Sub type (Get relay status)
		relay_status[0],         // Relay 1
		relay_status[1],         // Relay 2
		relay_status[2],         // Relay 3
		relay_status[3],         // Relay 4
		relay_status[4],         // Relay 5
		relay_status[5],         // Relay 6
		relay_status[6],         // Relay 7
		relay_status[7]          // Relay 8
	};
	
	printf("Sending relay status response (%d bytes)\n", sizeof(response));
	send_data(response, sizeof(response));
	printf("=== Relay Status Sent ===\n\n");
	
	return ESP_OK;
}

static esp_err_t toggle_relay(uint8_t relay) {
	char relay_count[13];
	snprintf(relay_count, sizeof(relay_count), "relay_%d", relay);
	int current_state = config_get_number(relay_count, 2);
	if(current_state == 2) {
		printf("[ERROR] Relay %d: Configuration not found\n", relay);
		return ESP_FAIL;
	}
	int new_state = (current_state == 0) ? 1 : 0;
	int gpio_pin;
	switch (relay) {
		case 0x01:
			gpio_pin = RELAY_1;
			break;
		case 0x02:
			gpio_pin = RELAY_2;
			break;
		case 0x03:
			gpio_pin = RELAY_3;
			break;
		case 0x04:
			gpio_pin = RELAY_4;
			break;
		case 0x05:
			gpio_pin = RELAY_5;
			break;
		case 0x06:
			gpio_pin = RELAY_6;
			break;
		case 0x07:
			gpio_pin = RELAY_7;
			break;
		case 0x08:
			gpio_pin = RELAY_8;
			break;
		default:
			printf("Invalid relay number: %d\n", relay);
			return ESP_FAIL;
	}
	printf("GPIO RELAY %d\n", gpio_pin);
	uint8_t relay_state = new_state == 1 ? 0 : 1;
	gpio_set_level(gpio_pin, relay_state);
	config_set_number(relay_count, new_state);
	
	printf("Relay %d toggled to %s\n", relay, new_state ? "ON" : "OFF");
	return ESP_OK;
}

static esp_err_t get_phone_numbers() {

	// will receive a hex data start with 9 on the phone number
    // the +63 will not send.
	// conversion will be
	// from hex 0x39 to Decimal 57 then ASCII table = 9

	const char *contacts_str = config_get_string("contacts");
	char *phone_numbers[10];
    int count = 0;
    
    char temp[256];
    strncpy(temp, contacts_str, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    
    char *token = strtok(temp, ",");
    while(token != NULL && count < 10) {
        phone_numbers[count] = strdup(token);
        printf("Phone %d: %s\n", count + 1, phone_numbers[count]);
        char *phone = phone_numbers[count];
    	int phone_len = strlen(phone);   
    	int start_index = 0;
	    if(phone_len > 3 && phone[0] == '+' && phone[1] == '6' && phone[2] == '3') {
	        start_index = 3;
	    }
	    
	    int digit_count = phone_len - start_index;
	    uint8_t response[4 + digit_count];
	    
	    response[0] = 0x4C;
	    response[1] = 0x54;
	    response[2] = 0x49;
	    response[3] = 0x02;
	    
	    for(int i = start_index; i < phone_len; i++) {
	        response[4 + (i - start_index)] = phone[i];
	    }
	    
	    printf("Response bytes: ");
	    for(int i = 0; i < 4 + digit_count; i++) {
	        printf("0x%02X ", response[i]);
	    }
	    printf("\n");
	    
	    send_data(response, sizeof(response));
		delay_ms(50);	    
	    count++;
	    token = strtok(NULL, ",");

    }
    
    printf("Total contacts: %d\n", count);
    
    for(int i = 0; i < count; i++) {
        free(phone_numbers[i]);
    }
	return ESP_OK;
}


static esp_err_t save_sell_egg(uint16_t tray_count, uint16_t price, uint8_t has_half, uint16_t total_sales) {
	
	start_data_upload_rtos();
	display_process_status("Rec.Sales");
	if(file_exists("/sdcard", "sales.csv") == false){
		int is_file_created = write_file("/sdcard", "sales.csv", "DATE,TRAY,HALF,PRICE,TOTAL SALES\n");
		if(is_file_created == ESP_FAIL) {
			return  ESP_FAIL;
		}
	}
	char date_display[125];
	char new_record[255];
	rtc_time_t t;
	if (rtc_get_time(&t) == RTC_OK) {
		if (t.format == RTC_FORMAT_12HR) {
            snprintf(date_display, sizeof(date_display), "%02d:%02d %s %02d-%02d-%02d",
                     t.hour, t.minute, t.is_pm ? "PM" : "AM", t.month, t.day, t.year);
        } else {
            snprintf(date_display, sizeof(date_display), "%02d:%02d %s %04d-%02d-%02d",
                     t.hour, t.minute, t.is_pm ? "PM" : "AM", t.year, t.month, t.day);
        }
    } else {
		return ESP_FAIL;
	}
	
	snprintf(new_record, sizeof(new_record), "%s,%u,%s,%u,%u",
				date_display,
				tray_count,
				has_half == 0x01 ? "Yes" : "No",
				price, total_sales
			);
	esp_err_t err = append_write_file("/sdcard", "sales.csv", new_record);
	if(err == ESP_FAIL) {
		return ESP_FAIL;
	}
	delay_ms(1000);
	display_process_status("Send SMS");

	const char* terminal_number = config_get_string("terminal_number");
	char message[255];
	snprintf(
		message,
		sizeof(message),
		"LoTIS Report: Sell Egg\r\n"
		"Terminal: %s\r\n"
		"Date: %s\r\n"
		"\r\n"
		"Tray: %u\r\n"
		"Half: %s\r\n"
		"Price: %u\r\n"
		"\r\n"
		"Total Sales: %u\r\n"
		"\r\n"
		"\x1A",
		terminal_number, date_display, tray_count,
		has_half == 0x01 ? "Yes" : "No", price, total_sales
	);
	send_sms(message);
	delay_ms(2000);
	stop_data_upload_rtos();
	clear_process_status();
	return ESP_OK;
}

static esp_err_t save_egg_harvest_report(uint16_t good_egg, uint16_t crack_egg){
	start_data_upload_rtos();
	display_process_status("Rec.Harvest");
	if(file_exists("/sdcard", "harvest.csv") == false){
		int is_file_created = write_file("/sdcard", "harvest.csv", "DATE,GOOD EGG,CRACK EGG\n");
		if(is_file_created == ESP_FAIL) {
			return  ESP_FAIL;
		}
	}
	char date_display[125];
	char new_record[255];
	rtc_time_t t;
	if (rtc_get_time(&t) == RTC_OK) {
		if (t.format == RTC_FORMAT_12HR) {
            snprintf(date_display, sizeof(date_display), "%02d:%02d %s %02d-%02d-%02d",
                     t.hour, t.minute, t.is_pm ? "PM" : "AM", t.month, t.day, t.year);
        } else {
            snprintf(date_display, sizeof(date_display), "%02d:%02d %s %04d-%02d-%02d",
                     t.hour, t.minute, t.is_pm ? "PM" : "AM", t.year, t.month, t.day);
        }
    } else {
		return ESP_FAIL;
	}
	
	snprintf(new_record, sizeof(new_record), "%s,%u,%u",date_display, good_egg, crack_egg);
	esp_err_t err = append_write_file("/sdcard", "harvest.csv", new_record);
	if(err == ESP_FAIL) {
		return ESP_FAIL;
	}
	delay_ms(1000);
	display_process_status("Send SMS");

	const char* terminal_number = config_get_string("terminal_number");
	char message[255];
	snprintf(
		message,
		sizeof(message),
		"LoTIS Report: Egg Harvest\r\n"
		"Terminal: %s\r\n"
		"Date: %s\r\n"
		"\r\n"
		"Good Egg: %d\r\n"
		"Crack Egg: %d\r\n"
		"\r\n"
		"\x1A",
		terminal_number, date_display, good_egg, crack_egg
	);
	send_sms(message);
	delay_ms(2000);
	stop_data_upload_rtos();
	clear_process_status();
	return ESP_OK;
}

 bool send_csv_data_via_ble(const char *line, int line_number, void *user_data) {                                                                      
      // Send line via BLE                                                                                                                         
      send_data((uint8_t *)line, strlen(line));                                                                                                    
                                                                                                                                                   
      delay_ms(50);                                                                                                         
                                                                                                                                                   
      return true;  // continue to next line                                                                                                       
}       

static esp_err_t get_egg_harvest_data_csv(){
	if(file_exists("/sdcard", "harvest.csv") == false) {
		return ESP_FAIL;
	}
	int lines_sent = read_file_lines("/sdcard", "harvest.csv", send_csv_data_via_ble, NULL);                                                          
                                                                                                                                                   
	if (lines_sent < 0) {                                                                                                                        
		return ESP_FAIL;                                                                                                                         
	}                                                                                                                                                                                      
    return ESP_OK;                                                                                                             	
}

/* ========================================================================== */
/*                         BLE Command Callback                               */
/* ========================================================================== */
/**
 * @brief Callback for when command data is received from BLE client
 *
 * Parses incoming BLE data using LoTIS protocol, validates timestamp,
 * and routes to appropriate command handler.
 */
static void on_command_received(uint8_t *data, uint16_t len) {	
    ESP_LOGI(TAG, "=== BLE Command Received ===");
    ESP_LOGI(TAG, "Length: %d bytes", len);

    // Log raw data bytes
    printf("Raw data: ");
    for (uint16_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
	printf("Data Length %d\n", len);
	if(len < 2) {
		printf("Invalid request\n");
		return;
	}

	// Command must have this header identification	
	// 4C 54 <----- Identification
	uint16_t header = (data[0] << 8) | data[1];
	if(header == 0x4C54) {
			// Request type
			// 49 <---- Info Request
		    // 48 <---- Hardware Control
	   	uint8_t request_type = data[2];
		switch (request_type) {
			case 0x43: {
				// connection device info
				uint8_t device_id = data[3]; 
				set_ble_connection_status_display(device_id);
				break;
			}
			case 0x48: {
				uint8_t sub_type = data[3];
               
				// Set time
				// 07 EA year 2026
				// 01 month january
                // 1F day 31
				// 01 hour 
				// 50 minutes
				// 01 is PM | PM = 1 or AM = 0
				if(sub_type == 0x01 && len == 11) {
					uint16_t year = (data[4] << 8) | data[5];
					uint8_t month = data[6];
					uint8_t day = data[7];
					uint8_t hour = data[8];
					uint8_t minute = data[9];
					uint8_t is_pm = data[10];
					esp_err_t result = set_date_time(year, month, day, hour, minute, is_pm);
					uint8_t status = (result == ESP_OK) ? 0x00 : 0x01;
					uint8_t response[5] = {0x4C, 0x54, 0x48, 0x01, status};
            		send_data(response, sizeof(response));
				}
				
				
				// toggle relay
				if(sub_type == 0x03 && len == 5) {
					uint8_t relay = data[4];
					esp_err_t result = toggle_relay(relay);
					uint8_t status = result == ESP_OK ? 0x00 : 0x01;
					uint8_t response[6] = {0x4C, 0x54, 0x48, 0x03, relay, status};
					send_data(response, sizeof(response));
				}
				
				break;
			}

			case 0x49: {
				uint8_t sub_type = data[3];
				// get relay info
				if(sub_type == 0x01 && len == 4) {
					esp_err_t result = get_relay_status();
					if(result == ESP_FAIL) {
						uint8_t response[5] = {0x4C, 0x54, 0x48, 0x02, 0x01};
            			send_data(response, sizeof(response));
						printf("Failed to get relay status");
					}
				}
				
				// get phone numbers
				if(sub_type == 0x02 && len == 4) {
					esp_err_t result =  get_phone_numbers();
					if(result == ESP_FAIL) {
						uint8_t response[5] = {0x4C, 0x54, 0x49, 0x02, 0x01};
            			send_data(response, sizeof(response));
					}
				}
				
				// record egg harvest
				if(sub_type == 0x03) {
					uint16_t good_egg = (data[4] << 8) | data[5];
					uint16_t crack_egg = (data[6] << 8) | data[7];
					  printf("\n--- Egg Harvest Record ---\n");
					    printf("Good Egg Count: %u\n", good_egg);
					    printf("Crack Egg Count: %u\n", crack_egg);
					esp_err_t result = save_egg_harvest_report( good_egg, crack_egg);
					uint8_t status = result == ESP_OK ? 0x00 : 0x01;
					uint8_t response[5] = {0x4C, 0x54, 0x49, 0x03, status};
					send_data(response, sizeof(response));
				}
				
				// get harvest data
				if(sub_type == 0x04) {
					esp_err_t result = get_egg_harvest_data_csv();
					uint8_t status = result == ESP_OK ? 0x00 : 0x01;
					uint8_t response[5] = {0x4C, 0x54, 0x49, 0x03, status};
            		send_data(response, sizeof(response));
				}
				
				// save sell egg
				if(sub_type == 0x05) {
					uint16_t tray_count = (data[4] << 8) | data[5];
					uint16_t price = (data[6] << 8) | data[7];
					uint8_t has_half = data[8];
					uint16_t total_price = (data[9] << 8) | data[10];
					printf("Save Sell Egg\n");
					printf("Total Tray: %u\n", tray_count);
					printf(has_half == 0x01 ? "Has Half\n" : "No Half\n");
					printf("Price: %u\n", price);
					printf("Total: %u\n", total_price);
					esp_err_t result = save_sell_egg(tray_count, price, has_half, total_price);
					uint8_t status = result == ESP_OK ? 0x00 : 0x01;
					uint8_t response[5] = {0x4C, 0x54, 0x49, 0x05, status};
					send_data(response, sizeof(response));
				}
			}
		}
		
	} else {
	   printf("Invalid Header\n");
	}
}



/* Callback for BLE connection status changes */
static void on_connection_status_changed(bool connected) {
    if (connected) {
        ESP_LOGI(TAG, "=== BLE Client Connected ===");
		set_ble_connection_status_icon(true);
    } else {
        ESP_LOGI(TAG, "=== BLE Client Disconnected ===");
        set_ble_connection_status_icon(false);
		set_ble_connection_status_display(0);

        // Save configuration to SD card on disconnect
        if (app_config != NULL) {
            ESP_LOGI(TAG, "Saving configuration to SD card...");
            if (save_config(app_config) == 0) {
                ESP_LOGI(TAG, "Configuration saved successfully");
            } else {
                ESP_LOGE(TAG, "Failed to save configuration");
            }
        }
		gpio_set_level(BUZZER, 1);
		delay_ms(50);
		gpio_set_level(BUZZER, 0);
		delay_ms(50);
    }
}





static int init_ble() {
	/* Local variables */
    int rc;
    esp_err_t ret;


    /*
     * NVS flash initialization
     * Dependency of BLE stack to store configurations
     */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nvs flash, error code: %d ", ret);
        return ESP_FAIL;
    }

    /* NimBLE stack initialization */
    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nimble stack, error code: %d ",
                 ret);
        return ESP_FAIL;
    }

    /* GAP service initialization */
    rc = gap_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to initialize GAP service, error code: %d", rc);
        return ESP_FAIL;
    }

    /* GATT server initialization */
    rc = gatt_svc_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to initialize GATT server, error code: %d", rc);
        return ESP_FAIL;
    }

    /* NimBLE host configuration initialization */
    nimble_host_config_init();

    /* Register callback for command received from client */
    set_command_callback(on_command_received);

    /* Register callback for connection status changes */
    set_connection_callback(on_connection_status_changed);

    /* Start NimBLE host task thread and return */
    xTaskCreate(nimble_host_task, "NimBLE Host", 4*1024, NULL, 5, NULL);

    /* Example: Send data to client (uncomment to test) */
    // uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    // vTaskDelay(pdMS_TO_TICKS(5000)); // Wait 5 seconds
    // send_data(test_data, sizeof(test_data));

    return ESP_OK;
}




void app_error(const char* error_message) {
	lcd_i2c_clear();
	lcd_i2c_set_cursor(0, 0);
	lcd_i2c_print("App Erro");
	lcd_i2c_set_cursor(1, 0);
	lcd_i2c_print(error_message);
	while (1) {
		delay_ms(500);
	}
}




void app_main(void) {
    /* ====================================================================== */
    /*                    Initialize I2C Bus                                  */
    /* ====================================================================== */

    ESP_LOGI(TAG, "Initializing I2C (single init)");
	
	init_relay_and_buzzer();
    // Initialize I2C once for both LCD and RTC
    esp_err_t r = i2c_helper_init(&bus);
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "i2c_helper_init failed: %d", r);
        return;
    }

    /* ====================================================================== */
    /*                    Initialize LCD Display                              */
    /* ====================================================================== */

    esp_err_t lcd_ret = lcd_i2c_init(&lcd_cfg);
    if (lcd_ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD init failed: %d", lcd_ret);
    } else {
        // Display hardware initialization status screen
        lcd_i2c_clear();
        lcd_i2c_set_cursor(0, 0);
        lcd_i2c_print("Hardware Init");
        lcd_i2c_set_cursor(1, 0);
        lcd_i2c_print("LCD: P");
        lcd_i2c_set_cursor(1, 8);
        lcd_i2c_print("GSM: *");
        lcd_i2c_set_cursor(2, 0);
        lcd_i2c_print("RTC: *");
        lcd_i2c_set_cursor(3, 0);
        lcd_i2c_print("SD: *");
    }

    // Load custom icons into LCD memory
    store_lcd_icons();

    delay_ms(1000);

    /* ====================================================================== */
    /*                    Initialize RTC (DS3231)                             */
    /* ====================================================================== */

    if (rtc_ds323_init(&rtc_cfg) != RTC_OK) {
        ESP_LOGE(TAG, "RTC init failed");
        if (lcd_ret == ESP_OK) {
            lcd_i2c_set_cursor(2, 5);
            lcd_i2c_print("F");  // Display fail status
        }
        // Continue execution to allow I2C scan results to be visible
    } else {
        ESP_LOGI(TAG, "RTC initialized");
        lcd_i2c_set_cursor(2, 5);
        lcd_i2c_print("P");  // Display pass status
    }


    /* ====================================================================== */
    /*                    Initialize SD Card                                  */
    /* ====================================================================== */

    sd_card_config_t sd_conf = {
        .spi_host = SPI2_HOST,              // SPI2_HOST or SPI3_HOST (HSPI/VSPI)
        .mosi_pin = SD_MOSI_PORT,           // GPIO11 - Master Out Slave In
        .miso_pin = SD_MISO_PORT,           // GPIO13 - Master In Slave Out
        .sclk_pin = SD_SCK_PORT,            // GPIO12 - SPI Clock
        .cs_pin = SD_CS_PORT,               // GPIO10 - Chip Select
        .max_freq_khz = 20000,              // 20MHz (safe for 74LVC125A)
        .format_if_mount_failed = false,    // Don't auto-format (set true to format on fail)
        .max_files = 5,                     // Maximum 5 files open simultaneously
        .allocation_unit_size = 16384,      // 16KB allocation units (0 = auto)
        .use_1_line_mode = true
    };

    sd_card_setup(&sd_conf);
    int sd_card_status = sd_card_init();

    // Update SD card status on LCD
    if(sd_card_status == ESP_OK) {
        lcd_i2c_set_cursor(3, 4);
        lcd_i2c_print("P");  // Display pass status
    } else {
        lcd_i2c_set_cursor(3, 4);
        lcd_i2c_print("F");  // Display fail status
    }
    delay_ms(1000);

    /* ====================================================================== */
    /*                    Initialize GSM Module (SIM900)                      */
    /* ====================================================================== */

	
    sim900_setup(&sim_conf);
    int sim_status = sim900_init();

    // Update GSM status on LCD
    if(sim_status == ESP_OK) {
        lcd_i2c_set_cursor(1, 13);
        lcd_i2c_print("P");  // Display pass status
    } else {
        lcd_i2c_set_cursor(1, 13);
        lcd_i2c_print("F");  // Display fail status
    }
	
	


    /* ====================================================================== */
    /*                    Load Configuration from SD Card                     */
    /* ====================================================================== */

    // Only proceed if both SIM900 and SD card initialized successfully
	
    if(sim_status == ESP_FAIL || sd_card_status != ESP_OK) {
        if(sim_status == ESP_FAIL && sd_card_status == ESP_FAIL){
			app_error("SIM & SD FAIL");
		} else if(sim_status == ESP_FAIL && sd_card_status == ESP_OK) {
			app_error("SIM ERROR");
		} else if(sim_status == ESP_OK && sd_card_status == ESP_FAIL) {
			app_error("SD FAIL");
		}
		return ;
    }
	
	
	lcd_i2c_set_cursor(3, 8);
    lcd_i2c_print("Passed!");
    delay_ms(1000);

    // Display memory initialization status
    lcd_i2c_clear();
    lcd_i2c_set_cursor(0, 0);
    lcd_i2c_print("Memory Init");
    lcd_i2c_set_cursor(1, 0);
    lcd_i2c_print("* Checking Config");

	if(init_memory() == ESP_FAIL) {
		app_error("Memory Fail");
		return;
	}


    lcd_i2c_set_cursor(1, 0);
    lcd_i2c_clear_row(1);
    lcd_i2c_print("Passed!");
    delay_ms(1000);
	lcd_i2c_clear();
	setup_relay_status();
	delay_ms(2000);
	

	
	
    /* ============================================================== */
    /*              Display Dashboard and Start Time Task            */
    /* ============================================================== */

    lcd_i2c_clear();
    dashboard_display();
    display_latest_time();

    // Create FreeRTOS task to update time every minute

    start_time_task();
	delay_ms(500);

	
	

	int ble_status = init_ble();
	
    if(ble_status != ESP_OK) {
		set_ble_connection_status_display(3);
	}

	while(1) {
	
		delay_ms(500);
	}
}

