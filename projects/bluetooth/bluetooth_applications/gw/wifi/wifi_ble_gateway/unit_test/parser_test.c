#include "parser_test.h"
#include "payload parser/parser.h"
#include "rsi_debug.h"
#include <string.h>
#include <stdint.h>

#define TLV_TYPE_WIFI_CRED  0x1001

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


static void dump_wifi_cred(const TLV_token_t* tok)
{
  if (!tok || tok->length < 2) {
    DEBUGOUT("  [WIFI_CRED] invalid length\r\n");
    return;
  }

  const uint8_t* p = tok->value;
  uint32_t rem = tok->length;

  uint8_t ssid_len = p[0];
  p += 1; rem -= 1;

  if (rem < (uint32_t) ssid_len + 1) {
    DEBUGOUT("  [WIFI_CRED] invalid ssid_len\r\n");
    return;
  }

  char ssid[33] = {0}; // max 32
  uint32_t copy_ssid = (ssid_len > 32) ? 32 : ssid_len;
  memcpy(ssid, p, copy_ssid);
  p += ssid_len; rem -= ssid_len;

  uint8_t pass_len = p[0];
  p += 1; rem -= 1;

  if (rem < pass_len) {
    DEBUGOUT("  [WIFI_CRED] invalid pass_len\r\n");
    return;
  }

  char pass[65] = {0}; // max 64
  uint32_t copy_pass = (pass_len > 64) ? 64 : pass_len;
  memcpy(pass, p, copy_pass);

  DEBUGOUT("  [WIFI_CRED] SSID=\"%s\" PASS=\"%s\"\r\n", ssid, pass);
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

   // Case 4: MULTIPLE WIFI CREDENTIALS in one payload
  // TLV(type=0x1001, len=1+ssid +1+pass)
  // Cred1: SSID="HomeWiFi" (8), PASS="12345678" (8) => len=1+8+1+8 = 18 (0x0012)
  // Cred2: SSID="Cafe" (4), PASS="coffee123" (9)  => len=1+4+1+9 = 15 (0x000F)

  static const uint8_t payload_multi_wifi[] = {
    // --- Cred 1 ---
    0x10,0x01,  0x00,0x12,
    0x08,'H','o','m','e','W','i','F','i',
    0x08,'1','2','3','4','5','6','7','8',

    // --- Cred 2 ---
    0x10,0x01,  0x00,0x0F,
    0x04,'C','a','f','e',
    0x09,'c','o','f','f','e','e','1','2','3'
  };

  memset(tokens, 0, sizeof(tokens));
  parsed = 0;
  err = parse_tlv_payload(payload_multi_wifi, sizeof(payload_multi_wifi), tokens, 8, &parsed);
  DEBUGOUT("[TLV TEST] MULTI_WIFI: err=%d parsed=%lu\r\n", (int)err, (unsigned long)parsed);

  if (err == TLV_ERR_NONE) {
    for (uint32_t i = 0; i < parsed; i++) {
      DEBUGOUT("Token[%lu]: type=0x%04X len=%u\r\n",
              (unsigned long)i, tokens[i].type, (unsigned)tokens[i].length);

      if (tokens[i].type == TLV_TYPE_WIFI_CRED) {
        dump_wifi_cred(&tokens[i]);
      } else {
        dump_tokens(&tokens[i], 1);
      }
    }
  }

}

void parser_run_all_tests(void)
{
  DEBUGOUT("\r\n========== TLV PARSER TESTS ==========\r\n");
  run_tlv_parser_tests();
  DEBUGOUT("[TLV TEST] DONE\r\n");
}
