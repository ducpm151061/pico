cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
set(projname "template")

project(${projname} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE=Debug)
pico_sdk_init()

add_executable(${projname}
        main.c
        )

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(${projname} pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(${projname})
