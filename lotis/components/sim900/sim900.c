#include "sim900.h"
/* ========================================================================== /
/                        PRIVATE DEFINITIONS                                 /
/ ========================================================================== */
#define SIM900_TAG "SIM900"
#define SIM900_UART_BUF_SIZE (2048)
#define SIM900_RESPONSE_BUF_SIZE (2048)
#define SIM900_DEFAULT_TIMEOUT_MS (5000)
#define SIM900_INIT_TIMEOUT_MS (10000)
#define SIM900_CMD_DELAY_MS (100)
/* ========================================================================== /
/                        PRIVATE VARIABLES                                   /
/ ========================================================================== */
static uart_port_t sim900_uart_port = 0;
static char response_buffer[SIM900_RESPONSE_BUF_SIZE];
static bool sim900_initialized = false;
/* ========================================================================== /
/                        PRIVATE FUNCTIONS                                   /
/ ========================================================================== */
/**

@brief Flush UART RX buffer
*/
static void sim900_flush_uart(void)
{
uart_flush_input(sim900_uart_port);
vTaskDelay(pdMS_TO_TICKS(50));
}

/**

@brief Read data from UART with timeout

@param buffer Buffer to store received data
@param buffer_size Size of buffer
@param timeout_ms Timeout in milliseconds
@return int Number of bytes read, -1 on error
*/
static int sim900_uart_read(char *buffer, size_t buffer_size, uint32_t timeout_ms)
{
    if (!buffer || buffer_size == 0) return -1;

    int total_read = 0;
    TickType_t start = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    while (1) {
        int to_read = (int)(buffer_size - total_read - 1);
        if (to_read <= 0) {
            ESP_LOGW(SIM900_TAG, "Buffer full");
            break;
        }

        int len = uart_read_bytes(sim900_uart_port,
                                  (uint8_t *)(buffer + total_read),
                                  to_read,
                                  pdMS_TO_TICKS(100)); /* short blocking read */

        if (len > 0) {
            total_read += len;
            buffer[total_read] = '\0';

            if (strstr(buffer, "\r\nOK\r\n") || strstr(buffer, "\r\nERROR\r\n") ||
                strstr(buffer, "+CME ERROR") || strstr(buffer, "+CMS ERROR") ||
                strchr(buffer, '>')) {
                break;
            }
        }

        if ((xTaskGetTickCount() - start) >= timeout_ticks) {
            uint32_t elapsed_ms = (uint32_t)((xTaskGetTickCount() - start) * portTICK_PERIOD_MS);
            ESP_LOGW(SIM900_TAG, "Read timeout after %u ms", elapsed_ms);
            break;
        }
    }
    return total_read;
}


/**

@brief Write data to UART

@param data Data to write
@param len Length of data
@return esp_err_t ESP_OK on success
*/
static esp_err_t sim900_uart_write(const char *data, size_t len)
{
int written = uart_write_bytes(sim900_uart_port, data, len);
if (written < 0) {
ESP_LOGE(SIM900_TAG, "UART write failed");
return ESP_FAIL;
}
return ESP_OK;
}

/* ========================================================================== /
/                        PUBLIC FUNCTIONS                                    /
/ ========================================================================== */
esp_err_t sim900_setup(const sim900_config_t *config)
{
if (!config) {
ESP_LOGE(SIM900_TAG, "Configuration is NULL");
return ESP_ERR_INVALID_ARG;
}
ESP_LOGI(SIM900_TAG, "Setting up SIM900A on UART%d", config->uart_port);

sim900_uart_port = config->uart_port;

// Configure UART parameters
uart_config_t uart_config = {
    .baud_rate = config->baud_rate,
    .data_bits = config->data_bits,
    .parity = config->parity,
    .stop_bits = config->stop_bits,
    .flow_ctrl = config->flow_ctrl,
    .rx_flow_ctrl_thresh = config->rx_flow_ctrl_thresh,
    .source_clk = UART_SCLK_DEFAULT,
};

// Install UART driver
esp_err_t ret = uart_driver_install(config->uart_port, 
                                    SIM900_UART_BUF_SIZE * 2, 
                                    SIM900_UART_BUF_SIZE * 2, 
                                    0, 
                                    NULL, 
                                    0);
if (ret != ESP_OK) {
    ESP_LOGE(SIM900_TAG, "Failed to install UART driver: %s", esp_err_to_name(ret));
    return ret;
}

// Configure UART parameters
ret = uart_param_config(config->uart_port, &uart_config);
if (ret != ESP_OK) {
    ESP_LOGE(SIM900_TAG, "Failed to configure UART: %s", esp_err_to_name(ret));
    uart_driver_delete(config->uart_port);
    return ret;
}

// Set UART pins
ret = uart_set_pin(config->uart_port, 
                  config->tx_pin, 
                  config->rx_pin,
                  UART_PIN_NO_CHANGE, 
                  UART_PIN_NO_CHANGE);
if (ret != ESP_OK) {
    ESP_LOGE(SIM900_TAG, "Failed to set UART pins: %s", esp_err_to_name(ret));
    uart_driver_delete(config->uart_port);
    return ret;
}

ESP_LOGI(SIM900_TAG, "SIM900A UART setup complete (TX:%d, RX:%d, Baud:%lu)",
         config->tx_pin, config->rx_pin, config->baud_rate);

return ESP_OK;
}
esp_err_t sim900_init(void)
{
	ESP_LOGI(SIM900_TAG, "Initializing SIM900A module...");
	// Wait for module to boot
	vTaskDelay(pdMS_TO_TICKS(3000));
	
	sim900_flush_uart();
	
	// Test basic communication with multiple attempts
	int max_attempts = 5;
	bool at_ok = false;

	for (int i = 0; i < max_attempts; i++) {
	    ESP_LOGI(SIM900_TAG, "Testing communication (attempt %d/%d)", i + 1, max_attempts);
	    
	    char *response = send_at_command("AT\r");
	    if (response && strstr(response, "OK")) {
	        ESP_LOGI(SIM900_TAG, "Module responded to AT command");
	        at_ok = true;
			break;
		}
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (!at_ok) {
        ESP_LOGE(SIM900_TAG, "Module not responding to AT commands");
        return ESP_FAIL;
    }

    // Disable echo for cleaner responses
    char *echo_response = send_at_command(AT_ECHO_OFF);
    if (!echo_response || !strstr(echo_response, "OK")) {
        ESP_LOGW(SIM900_TAG, "Failed to disable echo");
    }

    // Set verbose error messages
    char *error_response = send_at_command("AT+CMEE=2\r");
    if (error_response && strstr(error_response, "OK")) {
        ESP_LOGI(SIM900_TAG, "Verbose error reporting enabled");
    }

    // Check SIM card status
    vTaskDelay(pdMS_TO_TICKS(500));
    char *sim_response = send_at_command("AT+CPIN?\r");
    if (sim_response) {
        if (strstr(sim_response, "READY")) {
            ESP_LOGI(SIM900_TAG, "SIM card ready");
        } else if (strstr(sim_response, "SIM PIN")) {
            ESP_LOGW(SIM900_TAG, "SIM card requires PIN");
        } else {
            ESP_LOGW(SIM900_TAG, "SIM card status: %s", sim_response);
        }
    }

    // Get module information
    vTaskDelay(pdMS_TO_TICKS(500));
    char *info_response = send_at_command(AT_GET_INFO);
    if (info_response) {
        ESP_LOGI(SIM900_TAG, "Module info: %s", info_response);
    }

    // Wait for network registration
    ESP_LOGI(SIM900_TAG, "Waiting for network registration...");
    int reg_attempts = 30; // 30 seconds timeout
    bool registered = false;

    for (int i = 0; i < reg_attempts; i++) {
        char *reg_response = send_at_command("AT+CREG?\r");
        if (reg_response && (strstr(reg_response, ",1") || strstr(reg_response, ",5"))) {
            ESP_LOGI(SIM900_TAG, "Registered on network");
            registered = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (!registered) {
        ESP_LOGW(SIM900_TAG, "Network registration timeout - module may still work");
    }

    // Get signal quality
    vTaskDelay(pdMS_TO_TICKS(500));
    char *csq_response = send_at_command(AT_SIGNAL_QUALITY);
    if (csq_response) {
        ESP_LOGI(SIM900_TAG, "Signal quality: %s", csq_response);
    }

    sim900_initialized = true;
    ESP_LOGI(SIM900_TAG, "SIM900A initialization complete");

    return ESP_OK;
}

char* send_at_command(const char *command)
{
    if (!command) {
        ESP_LOGE(SIM900_TAG, "Command is NULL");
        return NULL;
    }

    // Clear response buffer
    memset(response_buffer, 0, SIM900_RESPONSE_BUF_SIZE);

    // Flush any pending data
    sim900_flush_uart();

    // Send command
    ESP_LOGD(SIM900_TAG, "Sending: %s", command);
    
    if (sim900_uart_write(command, strlen(command)) != ESP_OK) {
        ESP_LOGE(SIM900_TAG, "Failed to send command");
        return NULL;
    }

    // Small delay after sending
    vTaskDelay(pdMS_TO_TICKS(SIM900_CMD_DELAY_MS));

    // Read response
    int bytes_read = sim900_uart_read(response_buffer, 
                                      SIM900_RESPONSE_BUF_SIZE, 
                                      SIM900_DEFAULT_TIMEOUT_MS);

    if (bytes_read <= 0) {
        ESP_LOGW(SIM900_TAG, "No response received");
        return NULL;
    }

    ESP_LOGD(SIM900_TAG, "Response (%d bytes): %s", bytes_read, response_buffer);

    return response_buffer;
}

esp_err_t sim900_send_command_timeout(const char *command, char *response, size_t response_size, uint32_t timeout_ms)
{
    if (!command || !response || response_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Clear response buffer
    memset(response, 0, response_size);

    // Flush any pending data
    sim900_flush_uart();

    // Send command
    ESP_LOGD(SIM900_TAG, "Sending: %s", command);
    
    if (sim900_uart_write(command, strlen(command)) != ESP_OK) {
        ESP_LOGE(SIM900_TAG, "Failed to send command");
        return ESP_FAIL;
    }

    // Small delay after sending
    vTaskDelay(pdMS_TO_TICKS(SIM900_CMD_DELAY_MS));

    // Read response
    int bytes_read = sim900_uart_read(response, response_size, timeout_ms);

    if (bytes_read <= 0) {
        ESP_LOGW(SIM900_TAG, "No response received");
        return ESP_ERR_TIMEOUT;
    }

    ESP_LOGD(SIM900_TAG, "Response (%d bytes): %s", bytes_read, response);

    return ESP_OK;
}

bool sim900_is_ready(void)
{
    if (!sim900_initialized) {
        return false;
    }

    char *response = send_at_command("AT\r");
    return (response && strstr(response, "OK"));
}

esp_err_t sim900_deinit(void)
{
    ESP_LOGI(SIM900_TAG, "Deinitializing SIM900A module");

    if (sim900_initialized) {
        // Optionally send power down command
        send_at_command("AT+CPOWD=1\r");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    // Delete UART driver
    esp_err_t ret = uart_driver_delete(sim900_uart_port);
    if (ret != ESP_OK) {
        ESP_LOGE(SIM900_TAG, "Failed to delete UART driver: %s", esp_err_to_name(ret));
        return ret;
    }

    sim900_initialized = false;
    ESP_LOGI(SIM900_TAG, "SIM900A deinitialized");

    return ESP_OK;
}