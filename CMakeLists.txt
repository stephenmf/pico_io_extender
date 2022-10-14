# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/pi/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(greenhouse C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(greenhouse
)

pico_set_program_name(greenhouse "greenhouse")
pico_set_program_version(greenhouse "0.1")

pico_enable_stdio_uart(greenhouse 1)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(greenhouse PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Add the standard library to the build
target_link_libraries(greenhouse
    pico_stdlib
)

target_sources(greenhouse
  PUBLIC
    io/conversion.h
    io/console.h
    io/app_api.h
    io/framework.h
    io/led.h
    app/app.h
    app_config.h
  PRIVATE
    src/io/conversion.cpp
    src/io/console.cpp
    src/io/usb_descriptors.cpp
    src/io/framework.cpp
    src/io/led.cpp
    src/app/app.cpp
    src/main.cpp
)

target_link_libraries(greenhouse
    hardware_spi
    hardware_i2c
    hardware_dma
    hardware_pio
    hardware_timer
    hardware_watchdog
    hardware_clocks
    tinyusb_device
)

pico_add_extra_outputs(greenhouse)
