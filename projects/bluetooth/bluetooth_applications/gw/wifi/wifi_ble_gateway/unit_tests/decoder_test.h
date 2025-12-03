// decoder_tests.h
#ifndef DECODER_TESTS_H
#define DECODER_TESTS_H

#include <stdint.h>
#include "decode.h"   // để dùng uart_fsm_decoder_t

#define UART_DECODER_MOCK_TEST  1

void decoder_run_all_tests(uart_fsm_decoder_t *decoder);

#endif /* DECODER_TESTS_H */
