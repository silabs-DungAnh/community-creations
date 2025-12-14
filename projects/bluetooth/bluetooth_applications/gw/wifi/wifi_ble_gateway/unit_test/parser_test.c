#include "parser_test.h"
#include "parser.h"
#include "rsi_debug.h"
#include <string.h>
#include <stdint.h>

static void dump_tokens(const TLV_token_t* t, uint32_t n)
{
  for (uint32_t i = 0; i < n; i++) {
    DEBUGOUT("Token[%lu]: type=0x%04X len=%u val=",
             (unsigned long)i, t[i].type, (unsigned)t[i].length);

    for (uint32_t j = 0; j < t[i].length; j++) {
      DEBUGOUT("%02X ", t[i].value[j]);
    }
    DEBUGOUT("\r\n");
  }
}

static void run_tlv_parser_tests(void)
{
  TLV_token_t tokens[8];
  uint32_t parsed = 0;
  TLV_error_t err;

  // Case 1: OK 2 tokens
  static const uint8_t payload_ok[] = {
    0x00,0x01,  0x00,0x03,  0xAA,0xBB,0xCC,
    0x00,0x02,  0x00,0x01,  0x99
  };

  memset(tokens, 0, sizeof(tokens));
  parsed = 0;
  err = parse_tlv_payload(payload_ok, sizeof(payload_ok), tokens, 8, &parsed);
  DEBUGOUT("\r\n[TLV TEST] OK_2_TOKENS: err=%d parsed=%lu\r\n", (int)err, (unsigned long)parsed);
  if (err == TLV_ERR_NONE) dump_tokens(tokens, parsed);

  // Case 2: INVALID_LENGTH
  static const uint8_t payload_bad_len[] = {
    0x00,0x01,  0x00,0x03,  0xAA,0xBB
  };

  memset(tokens, 0, sizeof(tokens));
  parsed = 0;
  err = parse_tlv_payload(payload_bad_len, sizeof(payload_bad_len), tokens, 8, &parsed);
  DEBUGOUT("[TLV TEST] BAD_LEN: err=%d (expect %d) parsed=%lu\r\n",
           (int)err, (int)TLV_ERR_INVALID_LENGTH, (unsigned long)parsed);

  // Case 3: BUFFER_OVERFLOW
  static const uint8_t payload_3_tokens[] = {
    0x00,0x01,  0x00,0x01,  0x11,
    0x00,0x02,  0x00,0x01,  0x22,
    0x00,0x03,  0x00,0x01,  0x33
  };

  memset(tokens, 0, sizeof(tokens));
  parsed = 0;
  err = parse_tlv_payload(payload_3_tokens, sizeof(payload_3_tokens), tokens, 2, &parsed);
  DEBUGOUT("[TLV TEST] OVERFLOW: err=%d (expect %d) parsed=%lu\r\n",
           (int)err, (int)TLV_ERR_BUFFER_OVERFLOW, (unsigned long)parsed);
}

void parser_run_all_tests(void)
{
  DEBUGOUT("\r\n========== TLV PARSER TESTS ==========\r\n");
  run_tlv_parser_tests();
  DEBUGOUT("[TLV TEST] DONE\r\n");
}
