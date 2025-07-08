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

#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <libsd/libsd.h>

int main(void) {

    stdio_init_all();

    sleep_ms(10000);

    // card to be used
    LibsdCard card;
    card.block_size = 512;

    // SPI at 400 kHz on interface 0
    LibsdSpi spi;
    spi.freq = 400000;
    spi.id = 0;

    card.spi = &spi;

    LibsdReturnStatus ret;

    libsd_spi_init(&spi);

    printf("Past SPI init\n");

    ret = libsd_card_init(&card);

    printf("Past SD card init, return %d\n", ret);

    uint8_t tx_buf[512];
    for(int i = 0; i < 512; i++) {
        tx_buf[i] = 2 * (i + 1);
    }

    ret = libsd_card_write_block(&card, 0, tx_buf, 512);

    printf("Past SD card write, return %d\n", ret);

    uint8_t rx_buf[512];
    memset(rx_buf, 0, 512);

    ret = libsd_card_read_block(&card, 0, rx_buf, 512);

    printf("Past SD card read, return %d\n", ret);

    printf("First 30 elements from read:\n");
    for(int i = 0; i < 30; i++) {
        printf("%d\n", rx_buf[i]);
    }

    for(;;) {
        printf("Infinite loop\n");
        sleep_ms(1000);     
    }

    return 0;
}