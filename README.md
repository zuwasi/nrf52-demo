# nRF52 BLE Range Detector

CMake project for a MISRA-oriented nRF52 BLE advertisement range detector.

License: MIT. Architecture flowgraph: `docs/nrf52-demo-flow.drawio`.

The portable C core processes BLE advertising reports, extracts Tx Power and
local name fields, estimates proximity from RSSI, and reports devices that are
within approximately 2 meters. The nRF52 application target uses a small platform
port layer so the same detector can be connected to Nordic SoftDevice or nRF
Connect SDK scan callbacks.

## Assumptions

- Default calibrated Tx Power at 1 meter is -59 dBm.
- The 2 meter cutoff uses a 6 dB path-loss threshold, so a device is treated as
  near when `RSSI >= tx_power_at_1m - 6 dB`.
- RSSI ranging is approximate. BLE RSSI is affected by antenna orientation,
  body loss, multipath, advertising interval, and transmit-power calibration.
- The hardware baseline is nRF52 DK/PCA10040. Use the virtual COM UART pins and
  current-measurement setup carefully: active terminals/debugger state can affect
  low-power behavior.

## Host simulation and tests

```powershell
cd C:\tmp\nrf52
python .\simulation\run_simulation.py
```

This configures a host CMake build, runs the C unit tests, and runs the simulator
scenario. The simulator must print one `NEAR` device and `tracked=2`.

## ARM GCC 13 build

Install an ARM embedded GCC 13 toolchain with `arm-none-eabi-gcc` on `PATH`, then:

```powershell
cd C:\tmp\nrf52
cmake -S . -B build-arm -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake\arm-gcc13-toolchain.cmake -DNRF52_BUILD_TESTS=OFF
cmake --build build-arm
```

Or run the simulation script with ARM build enabled:

```powershell
python .\simulation\run_simulation.py --arm
```

## Porting to real BLE scanning

Replace `src/nrf52_ble_platform.c` with a Nordic SDK implementation:

1. Initialize clocks, BLE stack/controller, and GAP scanner.
2. On each scan report, fill `ble_advertisement_t`:
   - `address[6]`
   - `address_type`
   - `rssi_dbm`
   - `payload` pointer
   - `payload_length`
3. Return `NRF52_PLATFORM_STATUS_OK` from `nrf52_platform_poll_advertisement`
   when a report is available.
4. Implement `nrf52_platform_on_near_device` to toggle an LED, log through UART,
   or publish an event.

## MISRA C:2023 orientation

The C core is written for static analysis friendliness:

- fixed-width integer types;
- no dynamic memory allocation;
- bounded arrays and explicit lengths;
- no recursion;
- no pointer arithmetic beyond indexed arrays;
- explicit null checks at public interfaces;
- no floating point in range logic;
- small functions with single responsibility.

A formal MISRA C:2023 compliance claim still requires running your qualified
static-analysis toolchain and documenting any project-specific deviations. See
`MISRA_C_2023_NOTES.md` for the production-analysis scope and expected embedded
startup/test deviations.
