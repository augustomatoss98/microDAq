#include "crc.h"

uint8_t crc8(const uint8_t* data, size_t len){
    uint8_t crc = 0x00; // Initial CRC value

    // Process each byte in the input data
    for (size_t i = 0; i < len; i++){
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) crc = ((crc << 1) ^ 0x07 & 0xFF);
            else crc = (crc <<= 1) & 0xFF;
        }
    }

    return crc ^ 0x55;
}