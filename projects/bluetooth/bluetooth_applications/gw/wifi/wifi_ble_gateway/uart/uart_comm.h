#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif


// Message queue lengths
#ifndef UART_RX_MQ_LEN
#define UART_RX_MQ_LEN            1024u
#endif
#ifndef UART_TX_MQ_LEN
#define UART_TX_MQ_LEN            1024u
#endif

// RX chunk size
#ifndef UART_RX_CHUNK
#define UART_RX_CHUNK             1u
#endif


void uart_init(void);

// Byte by byte API
osStatus_t uart_write_bytes(const uint8_t *data, uint32_t length, uint32_t timeout_ms);
osStatus_t uart_read_byte(uint8_t *data, uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif
