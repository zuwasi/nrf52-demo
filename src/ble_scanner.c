#include "ble_scanner.h"

static bool addresses_are_equal(const uint8_t left[BLE_ADDRESS_LENGTH],
                                const uint8_t right[BLE_ADDRESS_LENGTH])
{
    uint8_t index = 0U;
    bool equal = true;

    for (index = 0U; index < BLE_ADDRESS_LENGTH; index++)
    {
        if (left[index] != right[index])
        {
            equal = false;
            break;
        }
    }

    return equal;
}

static void copy_address(uint8_t destination[BLE_ADDRESS_LENGTH],
                         const uint8_t source[BLE_ADDRESS_LENGTH])
{
    uint8_t index = 0U;

    for (index = 0U; index < BLE_ADDRESS_LENGTH; index++)
    {
        destination[index] = source[index];
    }
}

static void clear_name(char name[APP_DEVICE_NAME_LENGTH])
{
    uint8_t index = 0U;

    for (index = 0U; index < APP_DEVICE_NAME_LENGTH; index++)
    {
        name[index] = '\0';
    }
}

static void copy_name(char destination[APP_DEVICE_NAME_LENGTH],
                      const uint8_t * source,
                      size_t source_length)
{
    size_t index = 0U;
    size_t copy_length = source_length;

    if (copy_length >= APP_DEVICE_NAME_LENGTH)
    {
        copy_length = APP_DEVICE_NAME_LENGTH - 1U;
    }

    clear_name(destination);

    for (index = 0U; index < copy_length; index++)
    {
        destination[index] = (char)source[index];
    }
}

static uint16_t estimate_distance_cm(int8_t tx_power_dbm, int8_t rssi_dbm)
{
    int32_t loss_db = (int32_t)tx_power_dbm - (int32_t)rssi_dbm;
    uint16_t distance_cm = 100U;

    if (loss_db <= 0)
    {
        distance_cm = 100U;
    }
    else if (loss_db <= 6)
    {
        distance_cm = 201U;//test changed 200 to 201 
    }
    else if (loss_db <= 10)
    {
        distance_cm = 316U;
    }
    else if (loss_db <= 14)
    {
        distance_cm = 500U;
    }
    else if (loss_db <= 20)
    {
        distance_cm = 1000U;
    }
    else
    {
        distance_cm = 2000U;
    }

    return distance_cm;
}

static bool is_within_two_meters(int8_t tx_power_dbm,
                                 int8_t rssi_dbm,
                                 uint8_t path_loss_at_two_meters_db)
{
    int32_t threshold_dbm = (int32_t)tx_power_dbm - (int32_t)path_loss_at_two_meters_db;
    bool within_range = false;

    if ((int32_t)rssi_dbm >= threshold_dbm)
    {
        within_range = true;
    }

    return within_range;
}

static void parse_advertising_payload(const ble_scanner_t * scanner,
                                      const ble_advertisement_t * advertisement,
                                      int8_t * tx_power_dbm,
                                      char name[APP_DEVICE_NAME_LENGTH])
{
    size_t offset = 0U;
    bool name_found = false;

    *tx_power_dbm = scanner->config.default_tx_power_dbm;
    clear_name(name);

    while (offset < advertisement->payload_length)
    {
        uint8_t field_length = advertisement->payload[offset];
        size_t type_offset = offset + 1U;
        size_t value_offset = offset + 2U;
        size_t next_offset = offset + (size_t)field_length + 1U;

        if (field_length == 0U)
        {
            break;
        }

        if ((next_offset > advertisement->payload_length) || (type_offset >= advertisement->payload_length))
        {
            break;
        }

        if (advertisement->payload[type_offset] == BLE_AD_TYPE_TX_POWER)
        {
            if ((field_length >= 2U) && (value_offset < advertisement->payload_length))
            {
                *tx_power_dbm = (int8_t)advertisement->payload[value_offset];
            }
        }
        else if ((advertisement->payload[type_offset] == BLE_AD_TYPE_COMPLETE_NAME) ||
                 (advertisement->payload[type_offset] == BLE_AD_TYPE_SHORT_NAME))
        {
            if ((field_length >= 2U) && (name_found == false))
            {
                size_t name_length = (size_t)field_length - 1U;
                copy_name(name, &advertisement->payload[value_offset], name_length);
                name_found = true;
            }
        }
        else
        {
            /* Other AD structures are intentionally ignored by the range detector. */
        }

        offset = next_offset;
    }
}

static ble_device_report_t * find_or_allocate_report(ble_scanner_t * scanner,
                                                     const ble_advertisement_t * advertisement)
{
    uint8_t index = 0U;
    ble_device_report_t * report = (ble_device_report_t *)0;

    for (index = 0U; index < scanner->device_count; index++)
    {
        if ((scanner->devices[index].address_type == advertisement->address_type) &&
            (addresses_are_equal(scanner->devices[index].address, advertisement->address) == true))
        {
            report = &scanner->devices[index];
            break;
        }
    }

    if ((report == (ble_device_report_t *)0) && (scanner->device_count < APP_MAX_TRACKED_DEVICES))
    {
        report = &scanner->devices[scanner->device_count];
        scanner->device_count++;
    }

    return report;
}

ble_scanner_status_t ble_scanner_init(ble_scanner_t * scanner,
                                      const ble_scanner_config_t * config)
{
    ble_scanner_status_t status = BLE_SCANNER_STATUS_OK;
    uint8_t index = 0U;

    if ((scanner == (ble_scanner_t *)0) || (config == (const ble_scanner_config_t *)0))
    {
        status = BLE_SCANNER_STATUS_INVALID_ARGUMENT;
    }
    else
    {
        scanner->config = *config;
        scanner->device_count = 0U;

        for (index = 0U; index < APP_MAX_TRACKED_DEVICES; index++)
        {
            uint8_t address_index = 0U;

            scanner->devices[index].address_type = 0U;
            scanner->devices[index].rssi_dbm = 0;
            scanner->devices[index].tx_power_dbm = scanner->config.default_tx_power_dbm;
            scanner->devices[index].estimated_distance_cm = 0U;
            scanner->devices[index].within_two_meters = false;
            clear_name(scanner->devices[index].name);

            for (address_index = 0U; address_index < BLE_ADDRESS_LENGTH; address_index++)
            {
                scanner->devices[index].address[address_index] = 0U;
            }
        }
    }

    return status;
}

ble_scanner_status_t ble_scanner_process_advertisement(ble_scanner_t * scanner,
                                                       const ble_advertisement_t * advertisement)
{
    ble_scanner_status_t status = BLE_SCANNER_STATUS_OK;

    if ((scanner == (ble_scanner_t *)0) ||
        (advertisement == (const ble_advertisement_t *)0) ||
        ((advertisement->payload == (const uint8_t *)0) && (advertisement->payload_length > 0U)))
    {
        status = BLE_SCANNER_STATUS_INVALID_ARGUMENT;
    }
    else
    {
        ble_device_report_t * report = find_or_allocate_report(scanner, advertisement);

        if (report == (ble_device_report_t *)0)
        {
            status = BLE_SCANNER_STATUS_STORAGE_FULL;
        }
        else
        {
            int8_t tx_power_dbm = scanner->config.default_tx_power_dbm;
            char parsed_name[APP_DEVICE_NAME_LENGTH];

            parse_advertising_payload(scanner, advertisement, &tx_power_dbm, parsed_name);

            copy_address(report->address, advertisement->address);
            report->address_type = advertisement->address_type;
            report->rssi_dbm = advertisement->rssi_dbm;
            report->tx_power_dbm = tx_power_dbm;
            report->estimated_distance_cm = estimate_distance_cm(tx_power_dbm, advertisement->rssi_dbm);
            report->within_two_meters = is_within_two_meters(tx_power_dbm,
                                                             advertisement->rssi_dbm,
                                                             scanner->config.path_loss_at_two_meters_db);
            copy_name(report->name, (const uint8_t *)parsed_name, APP_DEVICE_NAME_LENGTH);

            if ((report->within_two_meters == true) &&
                (scanner->config.detection_callback != (ble_scanner_detection_callback_t)0))
            {
                scanner->config.detection_callback(report, scanner->config.user_context);
            }
        }
    }

    return status;
}

uint8_t ble_scanner_get_device_count(const ble_scanner_t * scanner)
{
    uint8_t count = 0U;

    if (scanner != (const ble_scanner_t *)0)
    {
        count = scanner->device_count;
    }

    return count;
}

const ble_device_report_t * ble_scanner_get_device(const ble_scanner_t * scanner,
                                                   uint8_t index)
{
    const ble_device_report_t * report = (const ble_device_report_t *)0;

    if ((scanner != (const ble_scanner_t *)0) && (index < scanner->device_count))
    {
        report = &scanner->devices[index];
    }

    return report;
}
