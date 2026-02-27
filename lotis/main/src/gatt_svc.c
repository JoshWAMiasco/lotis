/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "gatt_svc.h"
#include "common.h"

/* Private function declarations */
static int data_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                           struct ble_gatt_access_ctxt *ctxt, void *arg);
static int command_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg);

/* Private variables */
/* Data service (formerly heart rate) */
static const ble_uuid128_t data_svc_uuid =
    BLE_UUID128_INIT(0x21, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15, 0xde, 0xef,
                     0x12, 0x12, 0x20, 0x00, 0x00, 0x00);

#define MAX_DATA_LEN 512
static uint8_t data_chr_val[MAX_DATA_LEN] = {0};
static uint16_t data_chr_val_len = 0;
static uint16_t data_chr_val_handle;
static const ble_uuid128_t data_chr_uuid =
    BLE_UUID128_INIT(0x21, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15, 0xde, 0xef,
                     0x12, 0x12, 0x21, 0x00, 0x00, 0x00);

static uint16_t data_chr_conn_handle = 0;
static bool data_chr_conn_handle_inited = false;
static bool data_ind_status = false;

/* Control service */
/* Callback function pointer for command received */
static void (*command_received_callback)(uint8_t *data, uint16_t len) = NULL;
/* Callback function pointer for connection status changes */
static void (*connection_status_callback)(bool connected) = NULL;
static const ble_uuid128_t control_svc_uuid =
    BLE_UUID128_INIT(0x22, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15, 0xde, 0xef,
                     0x12, 0x12, 0x30, 0x00, 0x00, 0x00);
static uint16_t command_chr_val_handle;
static const ble_uuid128_t command_chr_uuid =
    BLE_UUID128_INIT(0x24, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15, 0xde, 0xef,
                     0x12, 0x12, 0x25, 0x15, 0x00, 0x00);

/* GATT services table */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    /* Data service */
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &data_svc_uuid.u,
     .characteristics =
         (struct ble_gatt_chr_def[]){
             {/* Data characteristic */
              .uuid = &data_chr_uuid.u,
              .access_cb = data_chr_access,
              .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE,
              .val_handle = &data_chr_val_handle},
             {
                 0, /* No more characteristics in this service. */
             }}},

    /* Control service */
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &control_svc_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){/* Command characteristic */
                                        {.uuid = &command_chr_uuid.u,
                                         .access_cb = command_chr_access,
                                         .flags = BLE_GATT_CHR_F_WRITE,
                                         .val_handle = &command_chr_val_handle},
                                        {0}},
    },

    {
        0, /* No more services. */
    },
};

/* Private functions */
static int data_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                           struct ble_gatt_access_ctxt *ctxt, void *arg) {
    /* Local variables */
    int rc;

    /* Handle access events */
    /* Note: Data characteristic is read only */
    switch (ctxt->op) {

    /* Read characteristic event */
    case BLE_GATT_ACCESS_OP_READ_CHR:
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI(TAG, "characteristic read by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == data_chr_val_handle) {
            /* Send hex data buffer */
            rc = os_mbuf_append(ctxt->om, data_chr_val, data_chr_val_len);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto error;

    /* Unknown event */
    default:
        goto error;
    }

error:
    ESP_LOGE(
        TAG,
        "unexpected access operation to data characteristic, opcode: %d",
        ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

static int command_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg) {
    /* Local variables */
    int rc = 0;

    ESP_LOGI(TAG, "=== command_chr_access called! op=%d, attr_handle=%d, command_chr_val_handle=%d ===",
             ctxt->op, attr_handle, command_chr_val_handle);

    /* Handle access events */
    /* Note: Command characteristic is write only */
    switch (ctxt->op) {

    /* Write characteristic event */
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "characteristic write; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI(TAG,
                     "characteristic write by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == command_chr_val_handle) {
            /* Print received hex data to console */
            ESP_LOGI(TAG, "Received command data (%d bytes):", ctxt->om->om_len);
            printf("=== Hex data: ");
            for (int i = 0; i < ctxt->om->om_len; i++) {
                printf("%02X ", ctxt->om->om_data[i]);
            }
            printf("===\n");

            /* Call user callback if registered */
            if (command_received_callback != NULL) {
                command_received_callback(ctxt->om->om_data, ctxt->om->om_len);
            }
            return 0;
        } else {
            ESP_LOGE(TAG, "attr_handle mismatch! received=%d expected=%d",
                     attr_handle, command_chr_val_handle);
        }
        goto error;

    /* Unknown event */
    default:
        goto error;
    }

error:
    ESP_LOGE(TAG,
             "unexpected access operation to command characteristic, opcode: %d",
             ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

/* Public functions */
void set_command_callback(void (*callback)(uint8_t *data, uint16_t len)) {
    command_received_callback = callback;
    ESP_LOGI(TAG, "command callback registered");
}

void set_connection_callback(void (*callback)(bool connected)) {
    connection_status_callback = callback;
    ESP_LOGI(TAG, "connection status callback registered");
}

void notify_connection_status(bool connected) {
    if (connection_status_callback != NULL) {
        connection_status_callback(connected);
    }
}

void send_data(uint8_t *data, uint16_t len) {
    if (len > MAX_DATA_LEN) {
        ESP_LOGE(TAG, "data length %d exceeds maximum %d", len, MAX_DATA_LEN);
        return;
    }

    /* Copy data to buffer */
    memcpy(data_chr_val, data, len);
    data_chr_val_len = len;

    /* Send indication if subscribed */
    if (data_ind_status && data_chr_conn_handle_inited) {
        ble_gatts_indicate(data_chr_conn_handle, data_chr_val_handle);
        ESP_LOGI(TAG, "data indication sent! (%d bytes)", len);
    } else {
        ESP_LOGW(TAG, "data updated but no client subscribed");
    }
}

/*
 *  Handle GATT attribute register events
 *      - Service register event
 *      - Characteristic register event
 *      - Descriptor register event
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    /* Local variables */
    char buf[BLE_UUID_STR_LEN];

    /* Handle GATT attributes register events */
    switch (ctxt->op) {

    /* Service register event */
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(TAG, "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;

    /* Characteristic register event */
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(TAG,
                 "registering characteristic %s with "
                 "def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;

    /* Descriptor register event */
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(TAG, "registering descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;

    /* Unknown event */
    default:
        assert(0);
        break;
    }
}

/*
 *  GATT server subscribe event callback
 *      1. Update heart rate subscription status
 */

void gatt_svr_subscribe_cb(struct ble_gap_event *event) {
    /* Check connection handle */
    if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d",
                 event->subscribe.conn_handle, event->subscribe.attr_handle);
    } else {
        ESP_LOGI(TAG, "subscribe by nimble stack; attr_handle=%d",
                 event->subscribe.attr_handle);
    }

    /* Check attribute handle */
    if (event->subscribe.attr_handle == data_chr_val_handle) {
        /* Update data subscription status */
        data_chr_conn_handle = event->subscribe.conn_handle;
        data_chr_conn_handle_inited = true;
        data_ind_status = event->subscribe.cur_indicate;
    }
}

/*
 *  GATT server initialization
 *      1. Initialize GATT service
 *      2. Update NimBLE host GATT services counter
 *      3. Add GATT services to server
 */
int gatt_svc_init(void) {
    /* Local variables */
    int rc;

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    /* 2. Update GATT services counter */
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
