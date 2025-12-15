#include "uart_comm.h"
#include "rsi_debug.h"
#include "string.h"
#include "decode.h"
#include "decoder_test.h"
#include "parser_test.h"
#include "uart_driver_test.h"


uart_fsm_decoder_t decoder;

void app_init(void)
{
  uart_init();
  fsm_decoder_init(&decoder);
}

void app_process_action(void)
{
#if UART_DECODER_MOCK_TEST
  static bool tests_ran = false;
  if (!tests_ran) {
    decoder_run_all_tests(&decoder);
    tests_ran = true;
  }
#endif

#if MOCK_TEST_UART
  mock_test_uart();
#endif

#if TLV_PARSER_TEST
  static bool ran = false;
  if (!ran) {
    parser_run_all_tests();
    ran = true;
  }
#endif
}
