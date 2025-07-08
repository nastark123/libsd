cmake_minimum_required(VERSION 3.12)

message("TARGET_PLATFORM = ${TARGET_PLATFORM}")

# The pico sdk import needs to happen before the project call
if(${TARGET_PLATFORM} STREQUAL "rp2040")
    include(pico_sdk_import.cmake)
endif()

project(libsd C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

add_subdirectory(examples)