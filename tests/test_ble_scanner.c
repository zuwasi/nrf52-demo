#include "ble_scanner.h"

#include <stdio.h>

static uint32_t g_failures = 0U;
static uint32_t g_callback_count = 0U;

static void expect_true(bool condition, const char * message)
{
    if (condition == false)
    {
        g_failures++;
        (void)printf("FAIL: %s\n", message);
    }
}

static void detection_callback(const ble_device_report_t * report, void * user_context)
{
    (void)user_context;

    if (report != (const ble_device_report_t *)0)
    {
        g_callback_count++;
    }
}

static ble_scanner_config_t make_config(void)
{
    ble_scanner_config_t config;

    config.default_tx_power_dbm = APP_DEFAULT_TX_POWER_DBM;
    config.path_loss_at_two_meters_db = APP_PATH_LOSS_AT_TWO_METERS_DB;
    config.detection_callback = detection_callback;
    config.user_context = (void *)0;

    return config;
}

static void test_detects_device_inside_two_meters(void)
{
    ble_scanner_t scanner;
    ble_scanner_config_t config = make_config();
    const uint8_t address[BLE_ADDRESS_LENGTH] = { 1U, 2U, 3U, 4U, 5U, 6U };
    const uint8_t payload[] = { 2U, BLE_AD_TYPE_TX_POWER, 0xC5U, 5U, BLE_AD_TYPE_COMPLETE_NAME, 'B', 'e', 'a', 'c' };
    ble_advertisement_t advertisement;
    const ble_device_report_t * report;

    advertisement.address_type = 0U;
    advertisement.rssi_dbm = -64;
    advertisement.payload = payload;
    advertisement.payload_length = sizeof(payload);

    for (uint8_t index = 0U; index < BLE_ADDRESS_LENGTH; index++)
    {
        advertisement.address[index] = address[index];
    }

    g_callback_count = 0U;

    expect_true(ble_scanner_init(&scanner, &config) == BLE_SCANNER_STATUS_OK,
                "scanner init inside range");
    expect_true(ble_scanner_process_advertisement(&scanner, &advertisement) == BLE_SCANNER_STATUS_OK,
                "process inside range advertisement");

    report = ble_scanner_get_device(&scanner, 0U);

    expect_true(report != (const ble_device_report_t *)0, "inside report exists");

    if (report != (const ble_device_report_t *)0)
    {
        expect_true(report->within_two_meters == true, "inside report is within two meters");
        expect_true(report->tx_power_dbm == -59, "tx power parsed from advertisement");
        expect_true(report->name[0] == 'B', "name parsed from advertisement");
        expect_true(g_callback_count == 1U, "inside range callback count");
    }
}

static void test_rejects_device_outside_two_meters(void)
{
    ble_scanner_t scanner;
    ble_scanner_config_t config = make_config();
    const uint8_t payload[] = { 2U, BLE_AD_TYPE_TX_POWER, 0xC5U };
    ble_advertisement_t advertisement;
    const ble_device_report_t * report;

    advertisement.address_type = 1U;
    advertisement.address[0] = 10U;
    advertisement.address[1] = 11U;
    advertisement.address[2] = 12U;
    advertisement.address[3] = 13U;
    advertisement.address[4] = 14U;
    advertisement.address[5] = 15U;
    advertisement.rssi_dbm = -72;
    advertisement.payload = payload;
    advertisement.payload_length = sizeof(payload);

    g_callback_count = 0U;

    expect_true(ble_scanner_init(&scanner, &config) == BLE_SCANNER_STATUS_OK,
                "scanner init outside range");
    expect_true(ble_scanner_process_advertisement(&scanner, &advertisement) == BLE_SCANNER_STATUS_OK,
                "process outside range advertisement");

    report = ble_scanner_get_device(&scanner, 0U);

    expect_true(report != (const ble_device_report_t *)0, "outside report exists");

    if (report != (const ble_device_report_t *)0)
    {
        expect_true(report->within_two_meters == false, "outside report is not within two meters");
        expect_true(g_callback_count == 0U, "outside range callback count");
    }
}

static void test_updates_existing_device(void)
{
    ble_scanner_t scanner;
    ble_scanner_config_t config = make_config();
    const uint8_t payload[] = { 0U };
    ble_advertisement_t advertisement;

    advertisement.address_type = 0U;
    advertisement.address[0] = 20U;
    advertisement.address[1] = 21U;
    advertisement.address[2] = 22U;
    advertisement.address[3] = 23U;
    advertisement.address[4] = 24U;
    advertisement.address[5] = 25U;
    advertisement.rssi_dbm = -70;
    advertisement.payload = payload;
    advertisement.payload_length = sizeof(payload);

    expect_true(ble_scanner_init(&scanner, &config) == BLE_SCANNER_STATUS_OK,
                "scanner init update test");
    expect_true(ble_scanner_process_advertisement(&scanner, &advertisement) == BLE_SCANNER_STATUS_OK,
                "process first update advertisement");

    advertisement.rssi_dbm = -63;
    expect_true(ble_scanner_process_advertisement(&scanner, &advertisement) == BLE_SCANNER_STATUS_OK,
                "process second update advertisement");
    expect_true(ble_scanner_get_device_count(&scanner) == 1U, "same device updated in place");
}

int main(void)
{
    test_detects_device_inside_two_meters();
    test_rejects_device_outside_two_meters();
    test_updates_existing_device();

    if (g_failures == 0U)
    {
        (void)printf("All BLE scanner unit tests passed.\n");
    }

    return (g_failures == 0U) ? 0 : 1;
}
