add_executable(busytest test.c unity_config.c ../src/busy_threads.c)

target_link_libraries(busytest PRIVATE
  pico_stdlib
  pico_async_context_freertos
  FreeRTOS-Kernel-Heap4
  unity
)

# The CYW43 is the Wifi/Bluetooth module. If the board is set to pico_w, this
# variable will be set and the wireless libraries added.
if(PICO_CYW43_SUPPORTED)
    target_link_libraries(busytest PRIVATE
        pico_cyw43_arch_none
    )
endif()

# Modify the below lines to enable/disable output over UART/USB.
# These are mutually exclusive, only set one.
pico_enable_stdio_uart(busytest 0)
pico_enable_stdio_usb(busytest 1)

add_custom_target(flash_test picotool load -f $<TARGET_FILE:busytest> DEPENDS busytest)
