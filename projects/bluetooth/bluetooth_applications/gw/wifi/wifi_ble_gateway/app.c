#include "uart_comm.h"
#include "rsi_debug.h"
#include "cmsis_os2.h"
#include "string.h"
void app_init(void)
{
   uart_init();
}


void app_process_action(void)
{
  // 1. Test uart_read_byte
//  uint8_t data;
//  uart_read_byte(&data, osWaitForever);
//  DEBUGOUT ("%c  \r\n", data);

  // 2. Test uart_write_bytes
//   static bool sent = true;
//
//     if (sent) {
//       uint8_t msg[] = "Hello from UART!\r\n";
//       uint32_t len = strlen((char*)msg);
//       uart_write_bytes(msg, len, osWaitForever);
//       sent = false; // Stop sending
//     }
}
