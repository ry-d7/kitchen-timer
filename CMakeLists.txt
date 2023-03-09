# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(kitchen-timer C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(kitchen-timer kitchen-timer.cpp oled.cpp Graphics.cpp dots-mono.cpp fortimer_small.cpp fortimer_large.cpp fortimer_large_long.cpp alarm-counter.cpp alarm-sound.cpp display.cpp switches.cpp)

pico_set_program_name(kitchen-timer "kitchen-timer")
pico_set_program_version(kitchen-timer "0.1")

pico_enable_stdio_uart(kitchen-timer 0)
pico_enable_stdio_usb(kitchen-timer 1)

# Add the standard library to the build
target_link_libraries(kitchen-timer pico_stdlib)

# Add any user requested libraries
target_link_libraries(kitchen-timer
        hardware_timer
        hardware_clocks
        hardware_pwm
        hardware_i2c
        hardware_dma
        )

pico_add_extra_outputs(kitchen-timer)

