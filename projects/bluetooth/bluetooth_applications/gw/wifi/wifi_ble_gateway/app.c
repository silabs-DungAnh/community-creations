#include "uart_comm.h"
#include "rsi_debug.h"
#include "string.h"
#include "decode.h"
#include "decoder_test.h"

// Macro to enable/disable mock test
#define MOCK_TEST_UART 0


#if MOCK_TEST_UART
void mock_test_uart(void)
{
  // 1. Test uart_read_byte
  uint8_t received_data;
  uart_read_byte(&received_data, osWaitForever);


  // 2. Test uart_write_bytes
  static bool sent = true;
  if (sent) {
    uint8_t msg[] = "Hello from UART!\r\n";
    uint32_t len = strlen((char*)msg);
    uart_write_bytes(msg, len, osWaitForever);
    sent = false; // Stop sending
  }
}
#endif /*MOCK_TEST_UART*/

uart_fsm_decoder_t decoder;

void app_init(void)
{
  uart_init();
  fsm_decoder_init(&decoder);
}

/***************************************************************************/ /**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  #if UART_DECODER_MOCK_TEST
    static bool tests_ran = false;
    if (!tests_ran) {
      decoder_run_all_tests(&decoder);
      tests_ran = true;
    }
  #endif /*UART_DECODER_MOCK_TEST*/

  #if MOCK_TEST_UART
    mock_test_uart();
  #endif /*MOCK_TEST_UART*/
}




