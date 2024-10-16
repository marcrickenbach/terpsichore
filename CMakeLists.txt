# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Uncomment if you must specify the board here
set(BOARD fkmg_stm32f405)

# Set BOARD_ROOT to the parent directory of the 'boards' folder
list(APPEND BOARD_ROOT "/Users/marcrickenbach/fkmg-dev/Terpsichore")


# For debugging, print the BOARD_ROOT path
message("BOARD_ROOT is: ${BOARD_ROOT}")
message("BOARD is: ${BOARD}")
message("ZEPHYR BASE is: $ENV{ZEPHYR_BASE}")

set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/terpsichore.overlay")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})


project(TERPSICHORE)



# Explicitly list source files
set(app_sources
    # src/leds.c
    src/main.c
    # src/pot.c 
    # src/io_ctrl/adc/adc.c
    # src/io_ctrl/dac/dac.c
    # src/io_ctrl/i2c/i2c.c
    # src/io_ctrl/gpio/gpio.c
    # src/io_ctrl/led/led.c
    # Add other source files here
)

target_sources(app PRIVATE ${app_sources})

# Main Include Directory
target_include_directories(app PRIVATE include)


