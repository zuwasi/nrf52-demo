# MISRA C:2023 Notes

This project is written to be MISRA C:2023-friendly, but a formal compliance
claim requires running the selected static-analysis tool configuration and
recording project-approved deviations.

## Intended clean area

`src/ble_scanner.c` and `include/ble_scanner.h` contain the portable application
logic and avoid:

- dynamic allocation;
- recursion;
- unbounded string APIs;
- floating-point distance math;
- implicit payload lengths;
- host I/O dependencies.

## Expected implementation deviations

The following files contain common embedded-startup or test/simulation patterns
that are normally handled by project deviations or excluded from production
MISRA scope:

| File | Reason |
|---|---|
| `src/startup_nrf52.c` | Cortex-M vector table uses compiler attributes, aliases, linker symbols, and a stack-pointer vector entry. |
| `src/nrf52_ble_platform.c` | Uses GCC inline assembly for `wfi`; replace with the vendor CMSIS intrinsic in a Nordic SDK port if required. |
| `tests/test_ble_scanner.c` | Host-only unit test code uses standard I/O for test reporting. |
| `simulation/scanner_sim.c` | Host-only simulation uses standard I/O for scenario output. |

## Static-analysis recommendation

Analyze production code first:

```text
include/*.h
src/ble_scanner.c
src/main.c
src/nrf52_ble_platform.c   (after replacing the stub with the real Nordic BLE adapter)
```

Then analyze startup code with an embedded-startup deviation profile.
