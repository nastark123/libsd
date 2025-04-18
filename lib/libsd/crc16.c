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

#include <libsd/crc16.h>

uint16_t crc16_table[256];

uint16_t crc16_calculate_byte(uint8_t crc_last, uint8_t data) {
    static uint8_t initialized = 0;
    if(!initialized) {
        initialized = 1;
        for (int byte = 0; byte < 256; byte++) {
            uint32_t crc = byte << 8;
            for(int bit = 0U; bit < 8U; bit ++) {
                crc <<= 1;
                if ((crc & 0x10000U) != 0U){ crc ^= 0x1021U; }
            }
            crc16_table[byte] = (crc & 0xFFFFU);
        }
    }

    return crc16_table[(data ^ (crc_last << 1)) & 0xFFFFU];
}

uint16_t crc16_calculate(uint8_t* data, int len) {
    uint8_t crc_val = 0;
    for(int i = 0; i < len; i++) {
        crc_val = crc16_calculate_byte(crc_val, data[i]);
    }

    return crc_val;
}