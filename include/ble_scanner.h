#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "app_config.h"

#define BLE_ADDRESS_LENGTH            (6U)
#define BLE_AD_TYPE_COMPLETE_NAME     (0x09U)
#define BLE_AD_TYPE_SHORT_NAME        (0x08U)
#define BLE_AD_TYPE_TX_POWER          (0x0AU)

typedef enum
{
    BLE_SCANNER_STATUS_OK = 0,
    BLE_SCANNER_STATUS_INVALID_ARGUMENT = 1,
    BLE_SCANNER_STATUS_STORAGE_FULL = 2
} ble_scanner_status_t;

typedef struct
{
    uint8_t address[BLE_ADDRESS_LENGTH];
    uint8_t address_type;
    int8_t rssi_dbm;
    int8_t tx_power_dbm;
    uint16_t estimated_distance_cm;
    bool within_two_meters;
    char name[APP_DEVICE_NAME_LENGTH];
} ble_device_report_t;

typedef struct
{
    uint8_t address[BLE_ADDRESS_LENGTH];
    uint8_t address_type;
    int8_t rssi_dbm;
    const uint8_t * payload;
    size_t payload_length;
} ble_advertisement_t;

typedef void (*ble_scanner_detection_callback_t)(const ble_device_report_t * report,
                                                 void * user_context);

typedef struct
{
    int8_t default_tx_power_dbm;
    uint8_t path_loss_at_two_meters_db;
    ble_scanner_detection_callback_t detection_callback;
    void * user_context;
} ble_scanner_config_t;

typedef struct
{
    ble_scanner_config_t config;
    ble_device_report_t devices[APP_MAX_TRACKED_DEVICES];
    uint8_t device_count;
} ble_scanner_t;

ble_scanner_status_t ble_scanner_init(ble_scanner_t * scanner,
                                      const ble_scanner_config_t * config);

ble_scanner_status_t ble_scanner_process_advertisement(ble_scanner_t * scanner,
                                                       const ble_advertisement_t * advertisement);

uint8_t ble_scanner_get_device_count(const ble_scanner_t * scanner);

const ble_device_report_t * ble_scanner_get_device(const ble_scanner_t * scanner,
                                                   uint8_t index);

#endif
