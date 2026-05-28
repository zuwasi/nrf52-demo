#ifndef NRF52_BLE_PLATFORM_H
#define NRF52_BLE_PLATFORM_H

#include <stdbool.h>
#include "ble_scanner.h"

typedef enum
{
    NRF52_PLATFORM_STATUS_OK = 0,
    NRF52_PLATFORM_STATUS_NO_EVENT = 1,
    NRF52_PLATFORM_STATUS_ERROR = 2
} nrf52_platform_status_t;

nrf52_platform_status_t nrf52_platform_init(void);
nrf52_platform_status_t nrf52_platform_start_ble_scan(void);
nrf52_platform_status_t nrf52_platform_poll_advertisement(ble_advertisement_t * advertisement);
void nrf52_platform_on_near_device(const ble_device_report_t * report);
void nrf52_platform_idle(void);

#endif
