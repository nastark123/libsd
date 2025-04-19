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
 * This file contains the functions provided to the user by the library for
 * interacting with the SD card (reading, writing, etc)
 ******************************************************************************/

#ifndef LIBSD_H
#define LIBSD_H

#include <stdint.h>

#include <libsd/spi.h>
#include <libsd/crc7.h>
#include <libsd/crc16.h>

/**
 * @brief Enumeration of all SD card commands predefined by standard
 * 
 * Enumeration of both numerical and actual names for SD card SPI commands.
 * Gaps in the numbering are either because the command is reserved or not
 * available for SPI mode (but may work in SDIO mode).  Taken from the SD Card
 * Physical Layer Specification.
 */
typedef enum {
    // Numerical names for commands
    CMD0 = 0,
    CMD1 = 1,
    CMD6 = 6,
    CMD8 = 8,
    CMD9 = 9,
    CMD10 = 10,
    CMD12 = 12,
    CMD13 = 13,
    CMD16 = 16,
    CMD17 = 17,
    CMD18 = 18,
    CMD24 = 24,
    CMD25 = 25,
    CMD27 = 27,
    CMD28 = 28,
    CMD29 = 29,
    CMD30 = 30,
    CMD32 = 32,
    CMD33 = 33,
    CMD38 = 38,
    CMD42 = 42,
    CMD55 = 55,
    CMD56 = 56,
    CMD58 = 58,
    CMD59 = 59,
    
    // Actual names for commands
    GO_IDLE_STATE           = CMD0,
    SEND_OP_COND            = CMD1,
    SWITCH_FUNC             = CMD6,
    SEND_IF_COND            = CMD8,
    SEND_CSD                = CMD9,
    SEND_CID                = CMD10,
    STOP_TRANSMISSION       = CMD12,
    SEND_STATUS             = CMD13,
    SET_BLOCKLEN            = CMD16,
    READ_SINGLE_BLOCK       = CMD17,
    READ_MULTIPLE_BLOCK     = CMD18,
    WRITE_BLOCK             = CMD24,
    WRITE_MULTIPLE_BLOCK    = CMD25,
    PROGRAM_CSD             = CMD27,
    SET_WRITE_PROT          = CMD28,
    CLR_WRITE_PROT          = CMD29,
    SEND_WRITE_PROT         = CMD30,
    ERASE_WR_BLK_START_ADDR = CMD32,
    ERASE_WR_BLK_END_ADDR   = CMD33,
    ERASE                   = CMD38,
    LOCK_UNLOCK             = CMD42,
    APP_CMD                 = CMD55,
    GEN_CMD                 = CMD56,
    READ_OCR                = CMD58,
    CRC_ON_OFF              = CMD59
} LibsdSpiDefinedCommandId;

/**
 * @brief Enumeration of all application specific SD card commands
 * 
 * Enumeration of both numerical and actual names for SD card application
 * specific SPI commands.  Gaps in the numbering are either because the command
 * is reserved or not available for SPI mode (but may work in SDIO mode).  Taken
 * from the SD Card Physical Layer Specification.
 */
typedef enum {
    // Numerical names for commands
    ACMD13 = 13,
    ACMD18 = 18,
    ACMD22 = 22,
    ACMD23 = 23,
    ACMD25 = 25,
    ACMD26 = 26,
    ACMD38 = 38,
    ACMD41 = 41,
    ACMD42 = 42,
    ACMD43 = 43,
    ACMD44 = 44,
    ACMD45 = 45,
    ACMD46 = 46,
    ACMD47 = 47,
    ACMD48 = 48,
    ACMD49 = 49,
    ACMD51 = 51,

    // Actual names for commands
    SD_STATUS              = ACMD13,
    SEND_NUM_WR_BLOCKS     = ACMD22,
    SET_WR_BLK_ERASE_COUNT = ACMD23,
    SD_SEND_OP_COND        = ACMD41,
    SET_CLR_CARD_DETECT    = ACMD42,
    SEND_SCR               = ACMD51
} LibsdSpiApplicationCommandId;

/**
 * @brief Struct with information for SD card
 */
typedef struct {
    LibsdSpi* spi; // SPI interface connected to SD card
    uint16_t block_size; // size of the block on the SD card, usually 512 bytes
} LibsdCard;

/**
 * @brief Struct for a predefined SPI command
 */
typedef struct {
    LibsdSpiDefinedCommandId id; // ID for command, see above enum
    uint32_t arg; // Some commands take a 32 bit argument, if yours doesn't, set to 0
} LibsdSpiDefinedCommand;

/**
 * @brief Struct for an application specific SPI command
 */
typedef struct {
    LibsdSpiApplicationCommandId id; // ID for command, see above enum
    uint32_t arg; // Some commands take a 32 bit argument, if yours doesn't set to 0
} LibsdSpiApplicationCommand;

/**
 * @brief Struct for R1 type response
 */
typedef struct {
    uint8_t resp;
} LibsdSpiCommandResponseR1;

/**
 * @brief Struct for R2 type response
 */
typedef struct {
    uint16_t resp;
} LibsdSpiCommandResponseR2;

/**
 * @brief Struct for R3 type response
 */
typedef struct {
    uint8_t resp;
    uint32_t extra;
} LibsdSpiCommandResponseR3;

/**
 * @brief Struct for R7 type response
 */
typedef struct {
    uint8_t resp;
    uint32_t extra;
} LibsdSpiCommandResponseR7;

/**
 * @brief Union for all response types
 */
typedef union {
    LibsdSpiCommandResponseR1 resp_r1;
    LibsdSpiCommandResponseR2 resp_r2;
    LibsdSpiCommandResponseR3 resp_r3;
    LibsdSpiCommandResponseR7 resp_r7;
} LibsdSpiCommandResponse;

/**
 * @brief Initialize an SD card using the values in a struct
 * 
 * @param card Pointer to LibsdCard struct for the card
 * @return Response code from SD card on failure, or 0 on success
 */
uint8_t libsd_init_card(LibsdCard* card);

/**
 * @brief Send a predefined command to the card
 * 
 * @param card Struct for the SD card to send the command to
 * @param cmd Struct for the command to be sent
 * @param resp Union to place response into
 */
void libsd_send_defined_command(LibsdCard* card, LibsdSpiDefinedCommand* cmd, LibsdSpiCommandResponse* resp);

/**
 * @brief Send an application specific command to the card
 * 
 * @param card Struct for the SD card to send the command to
 * @param cmd Struct for the command to be sent
 * @param resp Union to place the response into
 */
void libsd_send_application_command(LibsdCard* card, LibsdSpiApplicationCommand* cmd, LibsdSpiCommandResponse* resp);

/**
 * @brief Write a block (usually 512 bytes) to the SD card at the given block address
 * 
 * @param card Struct for the SD card to write to
 * @param write_buf Buffer containing data to be written
 * @param len Length of the buffer in bytes
 */
void libsd_write_block(LibsdCard* card, uint8_t* write_buf, uint16_t len);

/**
 * @brief Read a block (usually 512 bytes) from the SD card at the given block address
 * 
 * @param card Struct for the SD card to read from
 * @param read_buf Buffer to place the read data in
 * @param len Length of the buffer in bytes
 */
void libsd_read_block(LibsdCard* card, uint8_t* read_buf, uint16_t len);

#endif