add_library(libsd STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libsd/crc7.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libsd/crc16.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libsd/libsd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/spi/${TARGET_PLATFORM}/spi.c)

target_include_directories(libsd PUBLIC ${CMAKE_SOURCE_DIR}/include)