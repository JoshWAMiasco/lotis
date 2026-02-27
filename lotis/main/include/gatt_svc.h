/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef GATT_SVR_H
#define GATT_SVR_H

/* Includes */
/* NimBLE GATT APIs */
#include "host/ble_gatt.h"
#include "services/gatt/ble_svc_gatt.h"

/* NimBLE GAP APIs */
#include "host/ble_gap.h"

/* Public function declarations */
/* Set callback for when command data is received from client */
void set_command_callback(void (*callback)(uint8_t *data, uint16_t len));

/* Set callback for connection status changes */
void set_connection_callback(void (*callback)(bool connected));

/* Notify connection status change (called internally by GAP event handler) */
void notify_connection_status(bool connected);

/* Send hex data to client via data service */
void send_data(uint8_t *data, uint16_t len);

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
void gatt_svr_subscribe_cb(struct ble_gap_event *event);
int gatt_svc_init(void);

#endif // GATT_SVR_H
