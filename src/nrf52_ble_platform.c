#include "nrf52_ble_platform.h"

static volatile uint32_t g_near_device_events = 0U;

nrf52_platform_status_t nrf52_platform_init(void)
{
    return NRF52_PLATFORM_STATUS_OK;
}

nrf52_platform_status_t nrf52_platform_start_ble_scan(void)
{
    return NRF52_PLATFORM_STATUS_OK;
}

nrf52_platform_status_t nrf52_platform_poll_advertisement(ble_advertisement_t * advertisement)
{
    (void)advertisement;

    /*
     * Porting seam for Nordic SoftDevice or nRF Connect SDK/Zephyr:
     * convert BLE scan callback data into ble_advertisement_t and return OK.
     * The portable detector is fully tested by host simulation.
     */
    return NRF52_PLATFORM_STATUS_NO_EVENT;
}

void nrf52_platform_on_near_device(const ble_device_report_t * report)
{
    if (report != (const ble_device_report_t *)0)
    {
        g_near_device_events++;
    }
}

void nrf52_platform_idle(void)
{
#if defined(__GNUC__)
    __asm volatile ("wfi");
#endif
}
