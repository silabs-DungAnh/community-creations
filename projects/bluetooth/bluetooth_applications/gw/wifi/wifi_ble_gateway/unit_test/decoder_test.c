#include "decoder_test.h"
#include "rsi_debug.h"
#include "string.h"



// Packet hợp lệ
#if UART_DECODER_MOCK_TEST
static uint8_t valid_packet[] = {
    0xF0,
    0x01, 0x00,
    0x16, 0x00,
    0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20,
    0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73,
    0x20, 0x64, 0x65, 0x63, 0x6F, 0x64, 0x65, 0x72,
    0xFF,
    0x72, 0x5E,
    0x0F
};
#endif /* UART_DECODER_MOCK_TEST */

#if UART_DECODER_MOCK_TEST
static void run_decoder_test(const char *name,
                             uart_fsm_decoder_t *decoder,
                             uint8_t *packet,
                             size_t len)
{
  DEBUGOUT("\n========== %s ==========\n", name);

  fsm_decoder_init(decoder);

  for (size_t i = 0; i < len; i++) {
    decode_fsm(decoder, packet[i]);
  }
}
#endif /* UART_DECODER_MOCK_TEST */

#if UART_DECODER_MOCK_TEST
void decoder_run_all_tests(uart_fsm_decoder_t *decoder)
{
  // TC1: VALID
  run_decoder_test("TC1: VALID packet", decoder,
                   valid_packet, sizeof(valid_packet));

  // TC2: INVALID CRC
  uint8_t pkt_invalid_crc[sizeof(valid_packet)];
  memcpy(pkt_invalid_crc, valid_packet, sizeof(valid_packet));
  pkt_invalid_crc[sizeof(valid_packet) - 3] ^= 0x01;
  run_decoder_test("TC2: INVALID CRC", decoder,
                   pkt_invalid_crc, sizeof(pkt_invalid_crc));

// TC3: INVALID LENGTH
  uint8_t pkt_invalid_length[sizeof(valid_packet)];
  memcpy(pkt_invalid_length, valid_packet, sizeof(valid_packet));
  pkt_invalid_length[3] ^= 0x01; // Thay đổi byte độ dài
  run_decoder_test("TC3: INVALID LENGTH", decoder,
                   pkt_invalid_length, sizeof(pkt_invalid_length));


}
#endif /* UART_DECODER_MOCK_TEST */
