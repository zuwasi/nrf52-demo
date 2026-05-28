cmake_minimum_required(VERSION 3.20)

project(nrf52_ble_range_detector C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(NRF52_BUILD_TESTS "Build host unit tests and simulation" ON)
option(NRF52_BUILD_ARM_APP "Build the nRF52 bare-metal ARM application target" ON)

add_library(nrf52_ble_core STATIC
    src/ble_scanner.c
)

target_include_directories(nrf52_ble_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(nrf52_ble_core PRIVATE /W4 /WX)
else()
    target_compile_options(nrf52_ble_core PRIVATE
        -Wall
        -Wextra
        -Wconversion
        -Wsign-conversion
        -Werror
        -pedantic
    )
endif()

if(NRF52_BUILD_ARM_APP)
    add_executable(nrf52_ble_range_app
        src/main.c
        src/nrf52_ble_platform.c
        src/startup_nrf52.c
    )

    target_link_libraries(nrf52_ble_range_app PRIVATE nrf52_ble_core)
    target_include_directories(nrf52_ble_range_app PRIVATE include)
    target_compile_definitions(nrf52_ble_range_app PRIVATE NRF52_TARGET=1)

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(nrf52_ble_range_app PRIVATE
            -Wall
            -Wextra
            -Wconversion
            -Wsign-conversion
            -Werror
            -pedantic
            -ffunction-sections
            -fdata-sections
            -mcpu=cortex-m4
            -mthumb
        )
        target_link_options(nrf52_ble_range_app PRIVATE
            -T${CMAKE_CURRENT_SOURCE_DIR}/linker/nrf52832.ld
            -Wl,--gc-sections
            -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/nrf52_ble_range_app.map
            -nostartfiles
            -mcpu=cortex-m4
            -mthumb
        )
    endif()
endif()

if(NRF52_BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
    enable_testing()

    add_executable(ble_scanner_unit_tests
        tests/test_ble_scanner.c
    )
    target_link_libraries(ble_scanner_unit_tests PRIVATE nrf52_ble_core)
    target_include_directories(ble_scanner_unit_tests PRIVATE include)

    add_executable(scanner_sim
        simulation/scanner_sim.c
    )
    target_link_libraries(scanner_sim PRIVATE nrf52_ble_core)
    target_include_directories(scanner_sim PRIVATE include)

    add_test(NAME ble_scanner_unit_tests COMMAND ble_scanner_unit_tests)
    add_test(NAME scanner_sim_scenario COMMAND scanner_sim)
endif()
