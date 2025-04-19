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

/*******************************************************************************
 * This file contains function prototypes for various SPI functionality that the
 * library depends on to interact with the SD card.  Simple implementations are
 * already available under lib/spi/<arch> for different supported architectures.
 * If you want to use the library on a different architecture, supply your own
 * implementation of spi.c.  You may also have to modify which C compiler is
 * used for compilation in the top level CMakeLists.txt.
 ******************************************************************************/

#ifndef SPI_H
#define SPI_H

#include <stdint.h>

/**
 * @brief Struct with information about a SPI interface
 */
typedef struct {
    uint8_t id; // ID for the SPI device, for MCUs that support more than 1
    uint32_t freq; // SPI frequency
} LibsdSpi;

/**
 * @brief Initialize SPI interface that the library will be using
 * 
 * @param spi Pointer to a LibsdSpi struct for the SPI interface to initialize
 * 
 * The SPI interface should be initialized to the given frequency.  For MCUs
 * that have limits on what frequencies can be assigned, the closest one should
 * be chosen.  Note that the user of the library can choose the SPI frequency
 * that the interface will request, so you can select one that works with your
 * system.
 */
void libsd_spi_init(LibsdSpi* spi);

/**
 * @brief Write a single byte (8 bits) MSB first across the SPI interface.
 * 
 * @param spi SPI interface to write to
 * @param byte Byte to be written
 * 
 * This should simply write the byte across the SPI master.  It should block
 * until the byte transfers.
 */
void libsd_spi_write_byte(LibsdSpi* spi, uint8_t byte);

/**
 * @brief Write a buffer of bytes across the SPI interface.
 * 
 * @param spi SPI interface to write to
 * @param buf Pointer to first byte to write
 * @param len Total number of bytes to write
 * 
 * If you wish, this can simply be a repeated call to the libsd_spi_write_byte
 * function.  It should block until all data transfers.  However, for more
 * capable MCUs, this could be implemented with DMA to improve performance
 */
void libsd_spi_write_buf(LibsdSpi* spi, uint8_t* buf, uint32_t len);

/**
 * @brief Read a single byte from the SPI interface. 
 * 
 * @param spi SPI interface to read from
 * @param write_byte Byte to transfer on the write side (typically 0xFF for SD)
 * @param byte Pointer to byte to read into
 * 
 * This should read a byte from the SPI master and output a byte across the
 * SPI interface.
 */
void libsd_spi_read_write_byte(LibsdSpi* spi, uint8_t write_byte, uint8_t* read_byte);

/**
 * @brief Read a single byte from the SPI interface. 
 * 
 * @param spi SPI interface to read from
 * @param write_byte Byte to transfer on the write side (typically 0xFF for SD)
 * @param buf Pointer to first byte of read buffer
 * @param len Number of bytes to read
 * 
 * This should read len bytes from the SPI master and output a byte repeatedly
 * across the SPI interface.
 */
void libsd_spi_read_buf(LibsdSpi* spi, uint8_t write_byte, uint8_t* buf, uint32_t len);

#endif