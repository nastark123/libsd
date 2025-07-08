add_library(libsd STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libsd/crc7.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libsd/crc16.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libsd/libsd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/spi/${TARGET_PLATFORM}/spi.c)

# if adding your own platform, add an "if" here and do the appropriate linkage
if(${TARGET_PLATFORM} STREQUAL "rp2040")
    pico_sdk_init()

    target_link_libraries(libsd pico_stdlib hardware_spi)
endif()

target_include_directories(libsd PUBLIC ${CMAKE_SOURCE_DIR}/include)