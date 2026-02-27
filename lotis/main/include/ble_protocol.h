/**
 * @file ble_protocol.h
 * @brief LoTIS BLE Communication Protocol Definitions and Parser
 *
 * This protocol ensures secure, validated communication between the LoTIS
 * terminal and trusted clients (Android app or other ESP32 devices).
 *
 * Protocol Structure (minimum 12 bytes):
 * ┌─────────────┬──────────────┬──────────────┬─────────────┬──────┬───────────┐
 * │   Header    │ Client Type  │ Request Type │ Data Length │ Data │ Timestamp │
 * │   (2 bytes) │  (2 bytes)   │   (1 byte)   │  (2 bytes)  │ (N)  │ (4 bytes) │
 * ├─────────────┼──────────────┼──────────────┼─────────────┼──────┼───────────┤
 * │   4C 54     │   0A 01 or   │  01/02/03    │   00 00 -   │ ... │  Epoch    │
 * │   ("LT")    │   0B 01      │              │   FF FF     │      │ (seconds) │
 * └─────────────┴──────────────┴──────────────┴─────────────┴──────┴───────────┘
 *
 * @author LoTIS Team
 * @date 2026
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Protocol Constants                               */
/* ========================================================================== */

/** Protocol header magic bytes "LT" (LoTIS Terminal) */
#define BLE_PROTO_HEADER_BYTE1      0x4C
#define BLE_PROTO_HEADER_BYTE2      0x54

/** Minimum protocol packet size (header + client + request + length + timestamp) */
#define BLE_PROTO_MIN_SIZE          11

/** Maximum data payload size */
#define BLE_PROTO_MAX_DATA_SIZE     256

/* Client Type Identifiers */
#define BLE_CLIENT_TYPE_ANDROID     0x0A01  /**< LoTIS Manager Android Application */
#define BLE_CLIENT_TYPE_EGG_TERM    0x0B01  /**< Egg Harvest Terminal (ESP32) */

/* Request Type Codes */
#define BLE_REQUEST_HARDWARE_CTRL   0x01    /**< Hardware control (relays, GPIO) */
#define BLE_REQUEST_SYSTEM_CONFIG   0x02    /**< System configuration (RTC, settings) */
#define BLE_REQUEST_INFO_ONLY       0x03    /**< Information/status request */

/* Offset positions in protocol buffer */
#define BLE_PROTO_OFFSET_HEADER         0   /**< Header position (2 bytes) */
#define BLE_PROTO_OFFSET_CLIENT_TYPE    2   /**< Client type position (2 bytes) */
#define BLE_PROTO_OFFSET_REQUEST_TYPE   4   /**< Request type position (1 byte) */
#define BLE_PROTO_OFFSET_DATA_LENGTH    5   /**< Data length position (2 bytes) */
#define BLE_PROTO_OFFSET_DATA           7   /**< Data payload starts here */

/* Timestamp Validation */
#define BLE_PROTO_MAX_FUTURE_OFFSET     3600    /**< Max seconds in future (default: 1 hour) */
#define BLE_PROTO_DISABLE_FUTURE_CHECK  0       /**< Set to 1 to disable future check */
#define BLE_PROTO_TIMESTAMP_BYPASS      0xFFFFFFFF  /**< Special value to bypass timestamp validation */

/* Error Codes */
#define BLE_PROTO_OK                    0   /**< Success */
#define BLE_PROTO_ERR_INVALID_HEADER   -1   /**< Invalid header magic bytes */
#define BLE_PROTO_ERR_INVALID_CLIENT   -2   /**< Unknown client type */
#define BLE_PROTO_ERR_INVALID_REQUEST  -3   /**< Unknown request type */
#define BLE_PROTO_ERR_INVALID_LENGTH   -4   /**< Data length mismatch */
#define BLE_PROTO_ERR_EXPIRED          -5   /**< Command timestamp expired */
#define BLE_PROTO_ERR_TOO_SHORT        -6   /**< Packet too short */
#define BLE_PROTO_ERR_NULL_PTR         -7   /**< Null pointer argument */

/* ========================================================================== */
/*                           Data Structures                                  */
/* ========================================================================== */

/**
 * @brief Parsed BLE protocol packet structure
 */
typedef struct {
    uint16_t client_type;       /**< Client type identifier (0x0A01 or 0x0B01) */
    uint8_t request_type;       /**< Request type (0x01, 0x02, or 0x03) */
    uint16_t data_length;       /**< Length of data payload in bytes */
    uint8_t *data;              /**< Pointer to data payload (not owned) */
    uint32_t timestamp;         /**< Unix epoch timestamp from command */
    bool is_valid;              /**< Overall validation status */
} ble_protocol_packet_t;

/* ========================================================================== */
/*                           Function Prototypes                              */
/* ========================================================================== */

/**
 * @brief Validate protocol header magic bytes
 *
 * Checks if the first two bytes match the expected protocol header (0x4C 0x54).
 *
 * @param buffer Pointer to received data buffer
 * @param len Length of buffer
 * @return true if header is valid, false otherwise
 */
bool ble_proto_validate_header(const uint8_t *buffer, uint16_t len);

/**
 * @brief Validate client type identifier
 *
 * Checks if the client type is one of the recognized types.
 *
 * @param client_type Client type value (0x0A01 or 0x0B01)
 * @return true if client type is valid, false otherwise
 */
bool ble_proto_validate_client_type(uint16_t client_type);

/**
 * @brief Validate request type code
 *
 * Checks if the request type is one of the defined types.
 *
 * @param request_type Request type value (0x01, 0x02, or 0x03)
 * @return true if request type is valid, false otherwise
 */
bool ble_proto_validate_request_type(uint8_t request_type);

/**
 * @brief Validate command timestamp against RTC
 *
 * Compares the command timestamp with the current RTC time to ensure
 * the command hasn't expired. Commands with timestamps in the past are rejected.
 *
 * @param cmd_timestamp Command timestamp (Unix epoch seconds)
 * @return true if timestamp is valid (current or future), false if expired
 *
 * @note Requires RTC to be initialized and set correctly
 */
bool ble_proto_validate_timestamp(uint32_t cmd_timestamp);

/**
 * @brief Parse BLE protocol packet
 *
 * Parses a received BLE packet according to the LoTIS protocol specification.
 * Validates header, client type, request type, data length, and extracts all fields.
 * Does NOT validate timestamp - use ble_proto_validate_timestamp() separately.
 *
 * @param buffer Pointer to received data buffer
 * @param len Length of buffer in bytes
 * @param packet Pointer to packet structure to populate
 * @return BLE_PROTO_OK on success
 * @return BLE_PROTO_ERR_NULL_PTR if buffer or packet is NULL
 * @return BLE_PROTO_ERR_TOO_SHORT if packet is smaller than minimum size
 * @return BLE_PROTO_ERR_INVALID_HEADER if header doesn't match 0x4C54
 * @return BLE_PROTO_ERR_INVALID_CLIENT if client type is unknown
 * @return BLE_PROTO_ERR_INVALID_REQUEST if request type is unknown
 * @return BLE_PROTO_ERR_INVALID_LENGTH if data length doesn't match actual packet
 *
 * Example usage:
 * @code
 * ble_protocol_packet_t packet;
 * int result = ble_proto_parse(data, len, &packet);
 * if (result == BLE_PROTO_OK) {
 *     if (ble_proto_validate_timestamp(packet.timestamp)) {
 *         // Process command
 *     } else {
 *         ESP_LOGW(TAG, "Command expired");
 *     }
 * }
 * @endcode
 */
int ble_proto_parse(const uint8_t *buffer, uint16_t len, ble_protocol_packet_t *packet);

/**
 * @brief Get client type name string
 *
 * Returns a human-readable name for the client type.
 *
 * @param client_type Client type identifier
 * @return Pointer to static string with client name
 */
const char* ble_proto_get_client_name(uint16_t client_type);

/**
 * @brief Get request type name string
 *
 * Returns a human-readable name for the request type.
 *
 * @param request_type Request type code
 * @return Pointer to static string with request name
 */
const char* ble_proto_get_request_name(uint8_t request_type);

#ifdef __cplusplus
}
#endif
