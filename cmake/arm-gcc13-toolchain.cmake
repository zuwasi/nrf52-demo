# ARM GCC 13 toolchain for nRF52 Cortex-M4 builds.
# Configure with:
#   cmake -S . -B build-arm -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc13-toolchain.cmake
#   cmake --build build-arm

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(ARM_GCC_PREFIX arm-none-eabi CACHE STRING "ARM GCC embedded toolchain prefix")

set(CMAKE_C_COMPILER ${ARM_GCC_PREFIX}-gcc)
set(CMAKE_ASM_COMPILER ${ARM_GCC_PREFIX}-gcc)
set(CMAKE_OBJCOPY ${ARM_GCC_PREFIX}-objcopy CACHE FILEPATH "objcopy")
set(CMAKE_SIZE ${ARM_GCC_PREFIX}-size CACHE FILEPATH "size")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

execute_process(
    COMMAND ${CMAKE_C_COMPILER} -dumpfullversion
    OUTPUT_VARIABLE ARM_GCC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(ARM_GCC_VERSION VERSION_LESS 13.0)
    message(FATAL_ERROR "ARM GCC 13 or newer is required; detected '${ARM_GCC_VERSION}'")
endif()
