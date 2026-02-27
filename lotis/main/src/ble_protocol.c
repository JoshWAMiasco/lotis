/**
 * @file ble_protocol.c
 * @brief LoTIS BLE Communication Protocol Implementation
 */

#include "ble_protocol.h"
#include "rtc_ds323.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "BLE_PROTO";

/* ========================================================================== */
/*                         Validation Functions                               */
/* ========================================================================== */

bool ble_proto_validate_header(const uint8_t *buffer, uint16_t len)
{
    if (!buffer || len < 2) {
        return false;
    }

    return (buffer[0] == BLE_PROTO_HEADER_BYTE1 &&
            buffer[1] == BLE_PROTO_HEADER_BYTE2);
}

bool ble_proto_validate_client_type(uint16_t client_type)
{
    return (client_type == BLE_CLIENT_TYPE_ANDROID ||
            client_type == BLE_CLIENT_TYPE_EGG_TERM);
}

bool ble_proto_validate_request_type(uint8_t request_type)
{
    return (request_type == BLE_REQUEST_HARDWARE_CTRL ||
            request_type == BLE_REQUEST_SYSTEM_CONFIG ||
            request_type == BLE_REQUEST_INFO_ONLY);
}

bool ble_proto_validate_timestamp(uint32_t cmd_timestamp)
{
    // Check for bypass timestamp (0xFFFFFFFF) - allows commands without validation
    if (cmd_timestamp == BLE_PROTO_TIMESTAMP_BYPASS) {
        ESP_LOGW(TAG, "Timestamp bypass detected (0xFFFFFFFF) - skipping validation");
        return true;
    }

    int current_epoch = rtc_get_epoch();

    // Check if RTC read failed
    if (current_epoch < 0) {
        ESP_LOGE(TAG, "Failed to read RTC for timestamp validation");
        return false;
    }

    // Check if RTC is uninitialized (year 2000 or earlier)
    if (current_epoch < 946684800) { // Jan 1, 2000 00:00:00 UTC
        ESP_LOGE(TAG, "RTC not initialized! Current time: %d (year ~%d)",
                 current_epoch, 1970 + (current_epoch / 31557600));
        ESP_LOGE(TAG, "Please set RTC to correct time before using BLE commands");
        ESP_LOGI(TAG, "Use timestamp 0xFFFFFFFF to bypass validation for Set RTC command");
        return false;
    }

    // Command timestamp must be greater than or equal to current time
    if (cmd_timestamp < (uint32_t)current_epoch) {
        ESP_LOGW(TAG, "Command expired: cmd=%u, current=%d",
                 cmd_timestamp, current_epoch);
        return false;
    }

#if BLE_PROTO_DISABLE_FUTURE_CHECK == 0
    // Check if timestamp is too far in the future
    if ((cmd_timestamp - (uint32_t)current_epoch) > BLE_PROTO_MAX_FUTURE_OFFSET) {
        ESP_LOGW(TAG, "Command timestamp too far in future: cmd=%u, current=%d, offset=%u sec",
                 cmd_timestamp, current_epoch, (cmd_timestamp - (uint32_t)current_epoch));
        ESP_LOGI(TAG, "Max allowed future offset: %u seconds (%u minutes)",
                 BLE_PROTO_MAX_FUTURE_OFFSET, BLE_PROTO_MAX_FUTURE_OFFSET / 60);
        return false;
    }
#endif

    return true;
}

/* ========================================================================== */
/*                         Protocol Parser                                    */
/* ========================================================================== */

int ble_proto_parse(const uint8_t *buffer, uint16_t len, ble_protocol_packet_t *packet)
{
    // Validate input pointers
    if (!buffer || !packet) {
        ESP_LOGE(TAG, "Null pointer in ble_proto_parse");
        return BLE_PROTO_ERR_NULL_PTR;
    }

    // Initialize packet structure
    memset(packet, 0, sizeof(ble_protocol_packet_t));
    packet->is_valid = false;

    // Check minimum packet size
    if (len < BLE_PROTO_MIN_SIZE) {
        ESP_LOGE(TAG, "Packet too short: %d bytes (min %d)", len, BLE_PROTO_MIN_SIZE);
        return BLE_PROTO_ERR_TOO_SHORT;
    }

    // Validate header (bytes 0-1: 0x4C 0x54)
    if (!ble_proto_validate_header(buffer, len)) {
        ESP_LOGE(TAG, "Invalid header: 0x%02X 0x%02X (expected 0x4C 0x54)",
                 buffer[0], buffer[1]);
        return BLE_PROTO_ERR_INVALID_HEADER;
    }

    // Extract client type (bytes 2-3, big-endian)
    packet->client_type = ((uint16_t)buffer[2] << 8) | buffer[3];
    if (!ble_proto_validate_client_type(packet->client_type)) {
        ESP_LOGE(TAG, "Invalid client type: 0x%04X", packet->client_type);
        return BLE_PROTO_ERR_INVALID_CLIENT;
    }

    // Extract request type (byte 4)
    packet->request_type = buffer[4];
    if (!ble_proto_validate_request_type(packet->request_type)) {
        ESP_LOGE(TAG, "Invalid request type: 0x%02X", packet->request_type);
        return BLE_PROTO_ERR_INVALID_REQUEST;
    }

    // Extract data length (bytes 5-6, big-endian)
    packet->data_length = ((uint16_t)buffer[5] << 8) | buffer[6];

    // Validate data length
    // Total packet = 7 (header+client+request+length) + data_length + 4 (timestamp)
    uint16_t expected_len = 7 + packet->data_length + 4;
    if (len != expected_len) {
        ESP_LOGE(TAG, "Length mismatch: received %d, expected %d (data_len=%d)",
                 len, expected_len, packet->data_length);
        return BLE_PROTO_ERR_INVALID_LENGTH;
    }

    // Check data length doesn't exceed maximum
    if (packet->data_length > BLE_PROTO_MAX_DATA_SIZE) {
        ESP_LOGE(TAG, "Data length %d exceeds maximum %d",
                 packet->data_length, BLE_PROTO_MAX_DATA_SIZE);
        return BLE_PROTO_ERR_INVALID_LENGTH;
    }

    // Extract data pointer (starts at byte 7)
    if (packet->data_length > 0) {
        packet->data = (uint8_t *)&buffer[7];
    } else {
        packet->data = NULL;
    }

    // Extract timestamp (last 4 bytes, big-endian)
    uint16_t timestamp_offset = 7 + packet->data_length;
    packet->timestamp = ((uint32_t)buffer[timestamp_offset] << 24) |
                       ((uint32_t)buffer[timestamp_offset + 1] << 16) |
                       ((uint32_t)buffer[timestamp_offset + 2] << 8) |
                       ((uint32_t)buffer[timestamp_offset + 3]);

    // Mark as successfully parsed (but not timestamp validated)
    packet->is_valid = true;

    ESP_LOGI(TAG, "Parsed packet: client=%s, request=%s, data_len=%d, timestamp=%u",
             ble_proto_get_client_name(packet->client_type),
             ble_proto_get_request_name(packet->request_type),
             packet->data_length,
             packet->timestamp);

    return BLE_PROTO_OK;
}

/* ========================================================================== */
/*                         Utility Functions                                  */
/* ========================================================================== */

const char* ble_proto_get_client_name(uint16_t client_type)
{
    switch (client_type) {
        case BLE_CLIENT_TYPE_ANDROID:
            return "LoTIS Manager (Android)";
        case BLE_CLIENT_TYPE_EGG_TERM:
            return "Egg Harvest Terminal (ESP32)";
        default:
            return "Unknown Client";
    }
}

const char* ble_proto_get_request_name(uint8_t request_type)
{
    switch (request_type) {
        case BLE_REQUEST_HARDWARE_CTRL:
            return "Hardware Control";
        case BLE_REQUEST_SYSTEM_CONFIG:
            return "System Configuration";
        case BLE_REQUEST_INFO_ONLY:
            return "Information Only";
        default:
            return "Unknown Request";
    }
}
