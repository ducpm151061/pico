cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
set(projname "ds1307")

project(${projname} C CXX ASM)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE=Debug)
pico_sdk_init()

add_executable(${projname}
        main.cpp
        ds1307.cpp
)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(${projname} pico_stdlib hardware_i2c)

pico_enable_stdio_usb(${projname} 0)
pico_enable_stdio_uart(${projname} 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(${projname})
