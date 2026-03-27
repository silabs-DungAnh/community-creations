#include <stdint.h>
#include <string.h>
#include "uart_driver.h"
#include "sl_status.h"
#include "uartdrv.h"
#include "sl_uartdrv_init.h"
#define UART_HEADER   0xF0
#define UART_ENDCODE  0xFF
#define UART_TAIL     0x0F
Ecode_t uart_transmit(const uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) {
        return ECODE_EMDRV_UARTDRV_PARAM_ERROR;
    }
    Ecode_t status = UARTDRV_Transmit(sli_uartdrv_default_handle, (uint8_t *)data, length);
    return status;
}

Ecode_t uart_receive(uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) {
        return ECODE_EMDRV_UARTDRV_PARAM_ERROR;
    }
    Ecode_t status = UARTDRV_Receive(sli_uartdrv_default_handle, data, length);
    return status;
}

UARTDRV_Handle_t take_uart_handle(void) {
    sl_uartdrv_get_default();
}

uint16_t uart_make_frame(uint8_t *buf,
                         uint8_t opcode,
                         uint8_t type,
                         const uint8_t *value)
{
    if (buf == NULL || value == NULL) return 0;

    // tự tính length từ chuỗi value (kết thúc bằng '\0')
    uint16_t length = (uint16_t)strlen((const char*)value);

    uint32_t i = 0;
    uint8_t cs = 0;

    // Header
    buf[i++] = UART_HEADER;

    // Opcode
    buf[i++] = opcode; cs += opcode;

    // Type
    buf[i++] = type;   cs += type;

    // Length (LE)
    buf[i++] = (uint8_t)(length & 0xFF); cs += (length & 0xFF);
    buf[i++] = (uint8_t)((length >> 8) & 0xFF); cs += ((length >> 8) & 0xFF);

    // Value
    memcpy(&buf[i], value, length);
    for (uint16_t k = 0; k < length; ++k) cs += value[k];
    i += length;

    // Endcode
    buf[i++] = UART_ENDCODE; cs += UART_ENDCODE;

    // Checksum
    buf[i++] = cs;

    // Tail
    buf[i++] = UART_TAIL;

    return length; // chỉ trả về payload length
}