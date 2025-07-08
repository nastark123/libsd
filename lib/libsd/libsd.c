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

#include <libsd/libsd.h>

// Constant definitions
const uint8_t LIBSD_START_BLOCK_SINGLE = 0xFE;

const uint8_t LIBSD_START_BLOCK_MULTIPLE = 0xFC;

const uint8_t LIBSD_STOP_BLOCK_MULTIPLE = 0xFD;

LibsdReturnStatus libsd_card_init(LibsdCard* card) {
    // disable the SPI interface's chip select
    libsd_spi_cs_disable(card->spi);

    // SD cards need 80 clock cycles to complete their initialization routing
    // write 10 bytes of 0 with chip select disabled to do this
    for(int i = 0; i < 10; i++) {
        libsd_spi_write_byte(card->spi, 0xFF);
    }

    // reenable chip select to begin initialization
    libsd_spi_cs_enable(card->spi);

    // status returned from commands
    LibsdReturnStatus status;

    // return from commands
    LibsdSpiCommandResponse resp;

    // struct for predefined commands to send
    LibsdSpiDefinedCommand cmd;

    // first command is GO_IDLE_STATE (CMD0)
    cmd.id = GO_IDLE_STATE;
    cmd.arg = 0;
    status = libsd_card_send_defined_command(card, &cmd, &resp);
    if(status != LIBSD_OK || resp.resp_r1.resp != 0x01) {
        return LIBSD_INIT_FAILED;
    }

    // next command asks the SD card what voltages it supports
    // according to an online source this is mandatory, despite seeming optional
    cmd.id = SEND_IF_COND;
    cmd.arg = 0x1AA;
    status = libsd_card_send_defined_command(card, &cmd, &resp);
    if(status != LIBSD_OK || resp.resp_r7.resp != 0x01) {
        return LIBSD_INIT_FAILED;
    }

    // next command is ACMD41 which sets up for high capacity - most modern cards
    LibsdSpiApplicationCommand acmd;
    acmd.id = SD_SEND_OP_COND;
    acmd.arg = 0x40000000;
    do {
        status = libsd_card_send_application_command(card, &acmd, &resp);
    } while(resp.resp_r1.resp == 0x01 && status == LIBSD_OK);

    if(status != LIBSD_OK) {
        return LIBSD_INIT_FAILED;
    }

    // multiple things can happen here
    // if the response is 0x01, the card is initializing and we need to wait
    // resend the command
    // if the response is 0x05, the card is older and uses CMD1 for initialization
    // TODO - timeout for initialization to avoid infinite loop
    if(resp.resp_r1.resp == 0x05) {
        cmd.id = SEND_OP_COND;
        cmd.arg = 0;

        do {
            status = libsd_card_send_defined_command(card, &cmd, &resp);
        } while(resp.resp_r1.resp == 0x01 && status == LIBSD_OK);

        if(status != LIBSD_OK) {
            return LIBSD_INIT_FAILED;
        }
    }

    // initialization has finished
    // TODO - add some info about the card to the struct for the user to examine
    return LIBSD_OK;
}

LibsdReturnStatus libsd_card_send_defined_command(LibsdCard* card, LibsdSpiDefinedCommand* cmd, LibsdSpiCommandResponse* resp) {
    // first, we need to create a cmd_bufer with the command to send across the SPI interface
    // each command is 48 bits (6 bytes)
    uint8_t cmd_buf[6];

    cmd_buf[0] = 0x40 | (cmd->id & 0x3F);
    cmd_buf[1] = (cmd->arg >> 24) & 0xFF;
    cmd_buf[2] = (cmd->arg >> 16) & 0xFF;
    cmd_buf[3] = (cmd->arg >> 8) & 0xFF;
    cmd_buf[4] = cmd->arg & 0xFF;

    // find CRC7 for the first 5 bytes
    uint8_t crc = libsd_crc7_calculate(cmd_buf, 5);

    cmd_buf[5] = (crc << 1) | 0x01;

    // send to command buffer across the SPI interface
    libsd_spi_write_buf(card->spi, cmd_buf, 6);

    uint8_t read_byte = 0xFF;
    // it can take anywhere from 0 to 8 clock cycles for a response according to the specification
    for(int i = 0; i <= 8; i++) {
        libsd_spi_read_write_byte(card->spi, 0xFF, &read_byte);
        // the SD card will hold the line high for busy, and the MSB of any response is 0, so check for that
        if((read_byte & 0x80) == 0x00) {
            break;
        }
    }

    if(read_byte == 0xFF) {
        return LIBSD_TIMEOUT; // didn't get a valid response in time from the card
    }

    if(cmd->id == CMD8) {
        // CMD8 is the only predefined command that responds in an R7 format
        resp->resp_r7.resp = read_byte;
        resp->resp_r7.extra = 0;
        // R7 format has an additional 4 bytes of status
        for(int i = 0; i < 4; i++) {
            libsd_spi_read_write_byte(card->spi, 0xFF, &read_byte);
            resp->resp_r7.extra <<= 8;
            resp->resp_r7.extra |= read_byte;
        }

        return LIBSD_OK;
    } else if(cmd->id == CMD58) {
        // CMD58 is the only predefined command that responds in an R3 format
        resp->resp_r3.resp = read_byte;
        resp->resp_r3.extra = 0;
        // R3 format has an additional 4 bytes of status
        for(int i = 0; i < 4; i++) {
            libsd_spi_read_write_byte(card->spi, 0xFF, &read_byte);
            resp->resp_r3.extra <<= 8;
            resp->resp_r3.extra |= read_byte;
        }
        
        return LIBSD_OK;
    } else if(cmd->id == CMD13) {
        // CMD13 is the only predefined command that responds in an R2 format
        // need to read an extra 2 bytes
        uint8_t read_byte_lower;
        libsd_spi_read_write_byte(card->spi, 0xFF, &read_byte_lower);
        resp->resp_r2.resp = read_byte;
        resp->resp_r2.resp <<= 8;
        resp->resp_r2.resp |= read_byte_lower;

        return LIBSD_OK;
    } else {
        // we have a R1 response
        // some of these can have an optional busy signal, which this checks for
        // could probably be optimized, but this is a quick way to get this working
        resp->resp_r1.resp = read_byte;
        do {
            libsd_spi_read_write_byte(card->spi, 0xFF, &read_byte);
        } while(read_byte == 0x00);

        return LIBSD_OK;
    }
}

LibsdReturnStatus libsd_card_send_application_command(LibsdCard* card, LibsdSpiApplicationCommand* cmd, LibsdSpiCommandResponse* resp) {
    // every application specific command needs a APP_CMD (CMD55) sent first
    LibsdSpiDefinedCommand app_cmd;
    app_cmd.arg = 0;
    app_cmd.id = APP_CMD;
    LibsdReturnStatus status = libsd_card_send_defined_command(card, &app_cmd, resp);
    if(status != LIBSD_OK) {
        return status;
    }

    // after we have sent the APP_CMD, send the application command
    // same general procedure as the other
    uint8_t cmd_buf[6];

    cmd_buf[0] = 0x40 | (cmd->id & 0x3F);
    cmd_buf[1] = (cmd->arg >> 24) & 0xFF;
    cmd_buf[2] = (cmd->arg >> 16) & 0xFF;
    cmd_buf[3] = (cmd->arg >> 8) & 0xFF;
    cmd_buf[4] = cmd->arg & 0xFF;

    // find CRC7 for the first 5 bytes
    uint8_t crc = libsd_crc7_calculate(cmd_buf, 5);

    cmd_buf[5] = (crc << 1) | 0x01;

    // send to command buffer across the SPI interface
    libsd_spi_write_buf(card->spi, cmd_buf, 6);

    uint8_t read_byte = 0xFF;
    // it can take anywhere from 0 to 8 clock cycles for a response according to the specification
    for(int i = 0; i <= 8; i++) {
        libsd_spi_read_write_byte(card->spi, 0xFF, &read_byte);
        // the SD card will hold the line high for busy, and the MSB of any response is 0, so check for that
        if((read_byte & 0x80) == 0x00) {
            break;
        }
    }

    if(read_byte == 0xFF) {
        return LIBSD_TIMEOUT; // didn't get a valid response in time from the card
    }

    if(cmd->id == ACMD13) {
        // ACMD13 is the only application command with an R2 response
        // need to read an extra 2 bytes
        uint8_t read_byte_lower;
        libsd_spi_read_write_byte(card->spi, 0xFF, &read_byte_lower);
        resp->resp_r2.resp = read_byte;
        resp->resp_r2.resp <<= 8;
        resp->resp_r2.resp |= read_byte_lower;

        return LIBSD_OK;
    } else {
        // we have a R1 response
        // no busy signals for these
        resp->resp_r1.resp = read_byte;

        return LIBSD_OK;
    }
}

LibsdReturnStatus libsd_card_write_block(LibsdCard* card, uint32_t addr, uint8_t* write_buf, uint16_t len) {
    if(len < card->block_size) {
        return LIBSD_BUF_TOO_SMALL;
    } else if(len > card->block_size) {
        return LIBSD_BUF_TOO_LARGE;
    }

    LibsdSpiCommandResponse resp;
    LibsdSpiDefinedCommand cmd;
    cmd.arg = addr;
    cmd.id = WRITE_BLOCK;
    LibsdReturnStatus status = libsd_card_send_defined_command(card, &cmd, &resp);

    if(status != LIBSD_OK /* || resp.resp_r1.resp != 0x01*/) { // TODO check about the response from a write command
        return status;
    }

    // write start of block symbol
    libsd_spi_write_byte(card->spi, LIBSD_START_BLOCK_SINGLE);

    // write the actual data
    libsd_spi_write_buf(card->spi, write_buf, len);

    // calculate CRC and send
    // uint16_t crc = libsd_crc16_calculate_byte(0, LIBSD_START_BLOCK_SINGLE);
    uint16_t crc = libsd_crc16_calculate(0, write_buf, len);

    libsd_spi_write_byte(card->spi, crc >> 8);
    libsd_spi_write_byte(card->spi, crc & 0xFF);

    uint8_t read_byte = 0xFF;
    // it can take anywhere from 0 to 8 clock cycles for a response according to the specification
    for(int i = 0; i <= 8; i++) {
        libsd_spi_read_write_byte(card->spi, 0xFF, &read_byte);
        // the SD card will hold the line high for busy, and the MSB of any response is 0, so check for that
        if((read_byte & 0x01) && (~read_byte & 0x10)) {
            break;
        }
    }

    // wait for the write to be processed by the card
    uint8_t busy = 0x00;
    while(busy == 0) {
        libsd_spi_read_write_byte(card->spi, 0xFF, &busy);
    }

    // TODO - maybe make some constants for these???
    uint8_t write_status = (read_byte >> 1) & 0x07;
    switch(write_status) {
        case 0x02:
            return LIBSD_OK;

        case 0x05:
            return LIBSD_CRC_ERROR;

        case 0x06:
            return LIBSD_WRITE_FAILED;

        default:
            return LIBSD_WRITE_FAILED;
    }
}

LibsdReturnStatus libsd_card_read_block(LibsdCard* card, uint32_t addr, uint8_t* read_buf, uint16_t len) {
    if(len < card->block_size) {
        return LIBSD_BUF_TOO_SMALL;
    }

    LibsdSpiCommandResponse resp;
    LibsdSpiDefinedCommand cmd;
    cmd.arg = addr;
    cmd.id = READ_SINGLE_BLOCK;
    LibsdReturnStatus status = libsd_card_send_defined_command(card, &cmd, &resp);

    if(status != LIBSD_OK /* || resp.resp_r1.resp != 0x01*/) { // TODO check about the response from a read command
        return status;
    }

    // look for start of block symbol
    uint8_t read_byte = 0xFF;
    while(read_byte != LIBSD_START_BLOCK_SINGLE) {
        libsd_spi_read_write_byte(card->spi, 0xFF, &read_byte);
    }

    // read a block from the SD card
    libsd_spi_read_buf(card->spi, 0xFF, read_buf, card->block_size);

    // read the CRC16 for the data to verify correct
    uint8_t crc_buf[2];
    libsd_spi_read_buf(card->spi, 0xFF, crc_buf, 2);

    uint16_t received_crc = crc_buf[0];
    received_crc <<= 8;
    received_crc |= crc_buf[1];

    uint16_t calc_crc = libsd_crc16_calculate(0, read_buf, card->block_size);

    if(received_crc != calc_crc) {
        return LIBSD_CRC_ERROR;
    }

    return LIBSD_OK;
}