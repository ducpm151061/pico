cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
set(PICO_BOARD pico_w)
set(projname "iperf")

project(${projname} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE=Debug)
pico_sdk_init()

add_executable(picow_iperf_server_background
        picow_iperf.c
)
target_compile_definitions(picow_iperf_server_background PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)
target_include_directories(picow_iperf_server_background PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)
target_link_libraries(picow_iperf_server_background
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        pico_lwip_iperf
)
pico_enable_stdio_usb(picow_iperf_server_background 1)
pico_enable_stdio_uart(picow_iperf_server_background 0)
pico_add_extra_outputs(picow_iperf_server_background)

add_executable(picow_iperf_server_poll
        picow_iperf.c
)
target_compile_definitions(picow_iperf_server_poll PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)
target_include_directories(picow_iperf_server_poll PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)
target_link_libraries(picow_iperf_server_poll
        pico_cyw43_arch_lwip_poll
        pico_stdlib
        pico_lwip_iperf
)

pico_enable_stdio_usb(picow_iperf_server_poll 1)
pico_enable_stdio_uart(picow_iperf_server_poll 0)
pico_add_extra_outputs(picow_iperf_server_poll)