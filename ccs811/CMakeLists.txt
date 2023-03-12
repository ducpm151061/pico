cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
set(projname "ccs811")

project(${projname} C CXX ASM)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE=Debug)
pico_sdk_init()

add_executable(${projname}
        main.cpp
        ccs811.cpp
)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(${projname} pico_stdlib hardware_i2c hardware_uart)
pico_enable_stdio_usb(${projname} 1)
pico_enable_stdio_uart(${projname} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${projname})
