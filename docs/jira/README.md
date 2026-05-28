# Jira Manual Testing Setup

This folder contains sample Jira-importable manual tests for the nRF52 BLE range detector.

## Files

- `manual-tests.csv` — two sample manual test issues in CSV format.

## Import into Jira

1. Open Jira as a project/admin user.
2. Go to **Settings** -> **System** -> **External System Import** -> **CSV**.
3. Upload `docs/jira/manual-tests.csv`.
4. Map fields:
   - `Issue Type` -> Jira issue type. Use `Test` if Xray/Zephyr/test-management issue types are installed; otherwise map to `Task`.
   - `Summary` -> Summary.
   - `Description` -> Description.
   - `Priority` -> Priority.
   - `Labels` -> Labels.
   - `Components` -> Components.
   - `Manual Test Steps` -> a multiline text/custom field, or paste into the issue body if no manual-test plugin exists.
   - `Expected Results` -> a multiline text/custom field, or paste into the issue body if no manual-test plugin exists.

## Suggested Jira project settings

- Component: `BLE Scanner`
- Labels: `nrf52`, `manual-test`, `ble-ranging`, `simulation`
- Test environment: Windows host with CMake, Ninja or Make, Python 3, and a C compiler on `PATH`.

## Notes for real hardware testing

The current repository contains a host simulation and a platform seam for Nordic SDK integration. For nRF52 DK hardware validation, replace `src/nrf52_ble_platform.c` with the SoftDevice or nRF Connect SDK scan adapter, then add hardware-specific Jira tests for flashing, serial logging, and real RSSI calibration.
