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
*******************************************************************************/

#include <libsd/crc7.h>

uint8_t crc7_table[256];

uint8_t crc7_calculate_byte(uint8_t crc_last, uint8_t data) {
    static uint8_t initialized = 0;
    if(!initialized) {
        initialized = 1;
        for (int byte = 0; byte < 256; byte++) {
            uint8_t crc = byte;
            if((crc & 0x80U) != 0U) {
                crc ^= 0x89U;
            }
            for(int bit = 1U; bit < 8U; bit ++) {
                crc <<= 1;
                if ((crc & 0x80U) != 0U){ crc ^= 0x89U; }
            }
            crc7_table[byte] = (crc & 0x7FU);
        }
    }

    return crc7_table[(data ^ (crc_last << 1)) & 0xFFU];
}

uint8_t crc7_calculate(uint8_t* data, int len) {
    uint8_t crc_val = 0;
    for(int i = 0; i < len; i++) {
        crc_val = crc7_calculate_byte(crc_val, data[i]);
    }

    return crc_val;
}