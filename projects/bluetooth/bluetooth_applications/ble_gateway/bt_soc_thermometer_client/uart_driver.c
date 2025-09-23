#include "uart_driver.h"
#include "sl_status.h"
#include "uartdrv.h"
#include "sl_uartdrv_init.h"
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