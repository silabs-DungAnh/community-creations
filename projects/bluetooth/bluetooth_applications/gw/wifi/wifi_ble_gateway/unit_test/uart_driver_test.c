#include "uart_driver_test.h"
#include "stdbool.h"
#include "stdint.h"
#include "cmsis_os2.h"
#include "uart_comm.h"
#include "rsi_debug.h"

#if MOCK_TEST_UART

// A temp message to send and receive
#define MOCK_TEST_MESSAGE "Hello, UART Mock Test!"

void mock_test_uart(void)
{
  static bool tx_sent = false;
  uint8_t rx_byte;

  /* ================= TX TEST ================= */
  if (!tx_sent) {
    const uint8_t test_msg[] = MOCK_TEST_MESSAGE;
    osStatus_t st = uart_write_bytes(test_msg, sizeof(test_msg) - 1, 0);
    if (st == osOK) {
      DEBUGOUT("[UART MOCK] TX queued\r\n");
      tx_sent = true;
    } else {
      DEBUGOUT("[UART MOCK] TX failed: %d\r\n", st);
    }
  }

  /* ================= RX TEST ================= */
  osStatus_t st = uart_read_byte(&rx_byte, 0);
  if (st == osOK) {
    DEBUGOUT("[UART MOCK] RX byte = 0x%02X ('%c')\r\n",
             rx_byte,
             (rx_byte >= 32 && rx_byte <= 126) ? rx_byte : '.');
  }
}

#endif /* MOCK_TEST_UART */

