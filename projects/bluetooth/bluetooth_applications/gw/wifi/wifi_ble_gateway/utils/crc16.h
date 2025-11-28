#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#define CRC16_CCITT_INIT  (0xFFFFU)
uint16_t crc16_ccitt_update(uint16_t crc, uint8_t data);
uint16_t crc16_ccitt(const uint8_t *data, uint16_t length);

#endif 
