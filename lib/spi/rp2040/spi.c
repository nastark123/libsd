/*******************************************************************************
 * Copyright 2025 Nathan Stark
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

#include <libsd/spi.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

 void libsd_spi_init(LibsdSpi* spi) {
    // currently just support default SPI
    // TODO support multiple interfaces
    spi_init(spi_default, spi->freq);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    // Chip select needs to be disabled during initialization
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_NULL);
 }

 void libsd_spi_cs_disable(LibsdSpi* spi) {
    // Chip select needs to be disabled during initialization
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_NULL);
 }

 void libsd_spi_cs_enable(LibsdSpi* spi) {
    // Chip select needs to be disabled during initialization
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
 }

 void libsd_spi_write_byte(LibsdSpi* spi, uint8_t byte) {
    spi_write_blocking(spi_default, &byte, 1);
 }

 void libsd_spi_write_buf(LibsdSpi* spi, uint8_t* buf, uint32_t len) {
    spi_write_blocking(spi_default, buf, len);
 }

 void libsd_spi_read_write_byte(LibsdSpi* spi, uint8_t write_byte, uint8_t* read_byte) {
    spi_read_blocking(spi_default, write_byte, read_byte, 1);
 }

 void libsd_spi_read_buf(LibsdSpi* spi, uint8_t write_byte, uint8_t* buf, uint32_t len) {
    spi_read_blocking(spi_default, write_byte, buf, len);
 }