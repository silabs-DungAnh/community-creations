#ifndef FLASH_STORE_H
#define FLASH_STORE_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int pad4_write(uint32_t slot, uint32_t off, const uint8_t* data, size_t len);
int ota_store_begin(uint32_t total);
int ota_store_write(const uint8_t* data, size_t len);
int ota_store_finalize(void);
uint32_t ota_store_get_payload_size(void);
size_t ota_store_read(uint32_t off, uint8_t* out, size_t max);
#endif // FLASH_STORE_H