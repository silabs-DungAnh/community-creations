#ifndef UART_DRIVER_H
#define UART_DRIVER_H
#include "ecode.h"
#include "uartdrv.h"
#include <stdint.h>
Ecode_t uart_transmit(const uint8_t *data, uint32_t length);
Ecode_t uart_receive(uint8_t *data, uint32_t length);
UARTDRV_Handle_t take_uart_handle(void);
uint16_t uart_make_frame(uint8_t *buf,
                         uint8_t opcode,
                         uint8_t type,
                         const uint8_t *value);
#endif // UART_DRIVER_H