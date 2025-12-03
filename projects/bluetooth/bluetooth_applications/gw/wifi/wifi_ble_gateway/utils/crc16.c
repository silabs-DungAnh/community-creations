#include "crc16.h"
#include <stdint.h>


uint16_t crc16_ccitt_update(uint16_t crc, uint8_t data)
{
    crc ^= (uint16_t)data << 8;
    for (uint8_t j = 0; j < 8; j++) {
        if (crc & 0x8000U) {
            crc = (crc << 1) ^ 0x1021U;
        } else {
            crc <<= 1;
        }
        crc &= 0xFFFFU;
    }
    return crc;
}

// Calculate crc16
uint16_t crc16_ccitt(const uint8_t *data, uint16_t length){
    uint16_t crc = CRC16_CCITT_INIT;
    for (uint16_t i = 0; i < length; i++) {
        crc = crc16_ccitt_update(crc, data[i]);
    }
    return crc;
}
