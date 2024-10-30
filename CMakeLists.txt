# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Uncomment if you must specify the board here
set(BOARD fkmg_stm32f405)

# Set BOARD_ROOT to the parent directory of the 'boards' folder
list(APPEND BOARD_ROOT "/workspaces/terpsichore")

set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/terpsichore.overlay")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Include custom bindings in your board directory
set(ZEPHYR_EXTRA_MODULES "${BOARD_ROOT}/boards/arm/fkmg_terpsichore")

project(TERPSICHORE)

# Optimization flags based on build type
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build.")
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g" CACHE STRING "" FORCE)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3" CACHE STRING "" FORCE)
elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os" CACHE STRING "" FORCE)
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g" CACHE STRING "" FORCE)
endif()

# Explicitly list source files
set(app_sources
    src/main.c
)

target_sources(app PRIVATE ${app_sources})

# Main Include Directory
target_include_directories(app PRIVATE include)
