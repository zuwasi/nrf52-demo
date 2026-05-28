#include "ble_scanner.h"

#include <stdio.h>

static void print_address(const uint8_t address[BLE_ADDRESS_LENGTH])
{
    int32_t index = (int32_t)BLE_ADDRESS_LENGTH - 1;

    while (index >= 0)
    {
        (void)printf("%02X", address[(uint8_t)index]);

        if (index > 0)
        {
            (void)printf(":");
        }

        index--;
    }
}

static void detection_callback(const ble_device_report_t * report, void * user_context)
{
    (void)user_context;

    if (report != (const ble_device_report_t *)0)
    {
        (void)printf("NEAR ");
        print_address(report->address);
        (void)printf(" rssi=%d tx=%d distance_cm=%u name=%s\n",
                     (int)report->rssi_dbm,
                     (int)report->tx_power_dbm,
                     (unsigned int)report->estimated_distance_cm,
                     report->name);
    }
}

static ble_advertisement_t make_advertisement(uint8_t base,
                                              int8_t rssi_dbm,
                                              const uint8_t * payload,
                                              size_t payload_length)
{
    ble_advertisement_t advertisement;
    uint8_t index = 0U;

    for (index = 0U; index < BLE_ADDRESS_LENGTH; index++)
    {
        advertisement.address[index] = (uint8_t)(base + index);
    }

    advertisement.address_type = 0U;
    advertisement.rssi_dbm = rssi_dbm;
    advertisement.payload = payload;
    advertisement.payload_length = payload_length;

    return advertisement;
}

int main(void)
{
    ble_scanner_t scanner;
    ble_scanner_config_t config;
    const uint8_t near_payload[] = { 2U, BLE_AD_TYPE_TX_POWER, 0xC5U, 6U, BLE_AD_TYPE_COMPLETE_NAME, 'N', 'e', 'a', 'r', '1' };
    const uint8_t far_payload[] = { 2U, BLE_AD_TYPE_TX_POWER, 0xC5U, 5U, BLE_AD_TYPE_COMPLETE_NAME, 'F', 'a', 'r', '1' };
    ble_advertisement_t near_advertisement = make_advertisement(1U, -64, near_payload, sizeof(near_payload));
    ble_advertisement_t far_advertisement = make_advertisement(40U, -74, far_payload, sizeof(far_payload));

    config.default_tx_power_dbm = APP_DEFAULT_TX_POWER_DBM;
    config.path_loss_at_two_meters_db = APP_PATH_LOSS_AT_TWO_METERS_DB;
    config.detection_callback = detection_callback;
    config.user_context = (void *)0;

    if (ble_scanner_init(&scanner, &config) != BLE_SCANNER_STATUS_OK)
    {
        return 1;
    }

    (void)ble_scanner_process_advertisement(&scanner, &near_advertisement);
    (void)ble_scanner_process_advertisement(&scanner, &far_advertisement);

    (void)printf("tracked=%u\n", (unsigned int)ble_scanner_get_device_count(&scanner));

    return 0;
}
