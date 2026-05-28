#include "ble_scanner.h"
#include "nrf52_ble_platform.h"

static volatile uint32_t g_detection_count = 0U;

static void on_detection(const ble_device_report_t * report, void * user_context)
{
    (void)user_context;

    if (report != (const ble_device_report_t *)0)
    {
        g_detection_count++;
        nrf52_platform_on_near_device(report);
    }
}

int main(void)
{
    ble_scanner_t scanner;
    ble_scanner_config_t config;
    nrf52_platform_status_t platform_status;
    ble_scanner_status_t scanner_status;
    uint32_t idle_count = 0U;

    config.default_tx_power_dbm = APP_DEFAULT_TX_POWER_DBM;
    config.path_loss_at_two_meters_db = APP_PATH_LOSS_AT_TWO_METERS_DB;
    config.detection_callback = on_detection;
    config.user_context = (void *)0;

    platform_status = nrf52_platform_init();
    scanner_status = ble_scanner_init(&scanner, &config);

    if ((platform_status == NRF52_PLATFORM_STATUS_OK) &&
        (scanner_status == BLE_SCANNER_STATUS_OK))
    {
        platform_status = nrf52_platform_start_ble_scan();
    }

    while (platform_status == NRF52_PLATFORM_STATUS_OK)
    {
        ble_advertisement_t advertisement;

        platform_status = nrf52_platform_poll_advertisement(&advertisement);

        if (platform_status == NRF52_PLATFORM_STATUS_OK)
        {
            (void)ble_scanner_process_advertisement(&scanner, &advertisement);
            idle_count = 0U;
        }
        else if (platform_status == NRF52_PLATFORM_STATUS_NO_EVENT)
        {
            nrf52_platform_idle();
            idle_count++;

            if (idle_count > APP_SCAN_POLL_IDLE_LIMIT)
            {
                idle_count = 0U;
            }

            platform_status = NRF52_PLATFORM_STATUS_OK;
        }
        else
        {
            /* Stay in the error state and leave the loop. */
        }
    }

    for (;;)
    {
        nrf52_platform_idle();
    }
}
