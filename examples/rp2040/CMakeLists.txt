# pico_sdk_init()

add_executable(rp2040_example main.c)

pico_enable_stdio_usb(rp2040_example 1)
pico_enable_stdio_uart(rp2040_example 0)

pico_add_extra_outputs(rp2040_example)

target_link_libraries(rp2040_example pico_stdlib hardware_spi libsd)