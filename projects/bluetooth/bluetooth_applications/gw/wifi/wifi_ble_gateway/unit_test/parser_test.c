#include "parser_test.h"
#include "payload parser/parser.h"
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
  TLV_parsing_error_t err;

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

// ============= TLV_GetUint8 Tests =============
static void run_tlv_get_uint8_tests(void)
{
  uint8_t result;
  TLV_conv_error_t err;
  
  DEBUGOUT("\r\n[TLV_GetUint8 TEST] Starting...\r\n");
  
  // Test 1: Valid single byte
  static const uint8_t test_data_1[] = {0x42};
  result = 0xFF;
  err = TLV_GetUint8(test_data_1, &result);
  DEBUGOUT("[TLV_GetUint8] TEST 1 - Valid byte: err=%d result=0x%02X (expect 0x42)\r\n", 
           (int)err, result);
  
  // Test 2: Test with 0x00
  static const uint8_t test_data_2[] = {0x00};
  result = 0xFF;
  err = TLV_GetUint8(test_data_2, &result);
  DEBUGOUT("[TLV_GetUint8] TEST 2 - Zero byte: err=%d result=0x%02X (expect 0x00)\r\n", 
           (int)err, result);
  
  // Test 3: Test with 0xFF
  static const uint8_t test_data_3[] = {0xFF};
  result = 0x00;
  err = TLV_GetUint8(test_data_3, &result);
  DEBUGOUT("[TLV_GetUint8] TEST 3 - Max byte: err=%d result=0x%02X (expect 0xFF)\r\n", 
           (int)err, result);
  
  // Test 4: NULL buffer - should error
  result = 0xFF;
  err = TLV_GetUint8(NULL, &result);
  DEBUGOUT("[TLV_GetUint8] TEST 4 - NULL buffer: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
  
  // Test 5: NULL result pointer - should error
  err = TLV_GetUint8(test_data_1, NULL);
  DEBUGOUT("[TLV_GetUint8] TEST 5 - NULL result: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
  
  // Test 6: Both NULL - should error
  err = TLV_GetUint8(NULL, NULL);
  DEBUGOUT("[TLV_GetUint8] TEST 6 - Both NULL: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
}

// ============= TLV_GetUint16 Tests =============
static void run_tlv_get_uint16_tests(void)
{
  uint16_t result;
  TLV_conv_error_t err;
  
  DEBUGOUT("\r\n[TLV_GetUint16 TEST] Starting...\r\n");
  
  // Test 1: Valid 16-bit value (big-endian: 0x1234)
  static const uint8_t test_data_1[] = {0x12, 0x34};
  result = 0xFFFF;
  err = TLV_GetUint16(test_data_1, &result);
  DEBUGOUT("[TLV_GetUint16] TEST 1 - Valid value: err=%d result=0x%04X (expect 0x1234)\r\n", 
           (int)err, result);
  
  // Test 2: Zero value
  static const uint8_t test_data_2[] = {0x00, 0x00};
  result = 0xFFFF;
  err = TLV_GetUint16(test_data_2, &result);
  DEBUGOUT("[TLV_GetUint16] TEST 2 - Zero value: err=%d result=0x%04X (expect 0x0000)\r\n", 
           (int)err, result);
  
  // Test 3: Max value
  static const uint8_t test_data_3[] = {0xFF, 0xFF};
  result = 0x0000;
  err = TLV_GetUint16(test_data_3, &result);
  DEBUGOUT("[TLV_GetUint16] TEST 3 - Max value: err=%d result=0x%04X (expect 0xFFFF)\r\n", 
           (int)err, result);
  
  // Test 4: High byte only
  static const uint8_t test_data_4[] = {0xAB, 0x00};
  result = 0x0000;
  err = TLV_GetUint16(test_data_4, &result);
  DEBUGOUT("[TLV_GetUint16] TEST 4 - High byte: err=%d result=0x%04X (expect 0xAB00)\r\n", 
           (int)err, result);
  
  // Test 5: NULL buffer
  result = 0xFFFF;
  err = TLV_GetUint16(NULL, &result);
  DEBUGOUT("[TLV_GetUint16] TEST 5 - NULL buffer: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
  
  // Test 6: NULL result pointer
  err = TLV_GetUint16(test_data_1, NULL);
  DEBUGOUT("[TLV_GetUint16] TEST 6 - NULL result: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
}

// ============= TLV_GetUint32 Tests =============
static void run_tlv_get_uint32_tests(void)
{
  uint32_t result;
  TLV_conv_error_t err;
  
  DEBUGOUT("\r\n[TLV_GetUint32 TEST] Starting...\r\n");
  
  // Test 1: Valid 32-bit value (big-endian: 0x12345678)
  static const uint8_t test_data_1[] = {0x12, 0x34, 0x56, 0x78};
  result = 0xFFFFFFFF;
  err = TLV_GetUint32(test_data_1, &result);
  DEBUGOUT("[TLV_GetUint32] TEST 1 - Valid value: err=%d result=0x%08lX (expect 0x12345678)\r\n", 
           (int)err, (unsigned long)result);
  
  // Test 2: Zero value
  static const uint8_t test_data_2[] = {0x00, 0x00, 0x00, 0x00};
  result = 0xFFFFFFFF;
  err = TLV_GetUint32(test_data_2, &result);
  DEBUGOUT("[TLV_GetUint32] TEST 2 - Zero value: err=%d result=0x%08lX (expect 0x00000000)\r\n", 
           (int)err, (unsigned long)result);
  
  // Test 3: Max value
  static const uint8_t test_data_3[] = {0xFF, 0xFF, 0xFF, 0xFF};
  result = 0x00000000;
  err = TLV_GetUint32(test_data_3, &result);
  DEBUGOUT("[TLV_GetUint32] TEST 3 - Max value: err=%d result=0x%08lX (expect 0xFFFFFFFF)\r\n", 
           (int)err, (unsigned long)result);
  
  // Test 4: Single byte in high position
  static const uint8_t test_data_4[] = {0xAB, 0x00, 0x00, 0x00};
  result = 0x00000000;
  err = TLV_GetUint32(test_data_4, &result);
  DEBUGOUT("[TLV_GetUint32] TEST 4 - High byte: err=%d result=0x%08lX (expect 0xAB000000)\r\n", 
           (int)err, (unsigned long)result);
  
  // Test 5: Low 16-bits set
  static const uint8_t test_data_5[] = {0x00, 0x00, 0xCD, 0xEF};
  result = 0x00000000;
  err = TLV_GetUint32(test_data_5, &result);
  DEBUGOUT("[TLV_GetUint32] TEST 5 - Low 16-bits: err=%d result=0x%08lX (expect 0x0000CDEF)\r\n", 
           (int)err, (unsigned long)result);
  
  // Test 6: NULL buffer
  result = 0xFFFFFFFF;
  err = TLV_GetUint32(NULL, &result);
  DEBUGOUT("[TLV_GetUint32] TEST 6 - NULL buffer: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
  
  // Test 7: NULL result pointer
  err = TLV_GetUint32(test_data_1, NULL);
  DEBUGOUT("[TLV_GetUint32] TEST 7 - NULL result: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
}

// ============= TLV_GetString Tests =============
static void run_tlv_get_string_tests(void)
{
  char result[128];
  TLV_conv_error_t err;
  
  DEBUGOUT("\r\n[TLV_GetString TEST] Starting...\r\n");
  
  // Test 1: Valid string
  static const uint8_t test_data_1[] = {'H', 'e', 'l', 'l', 'o'};
  memset(result, 0, sizeof(result));
  err = TLV_GetString(test_data_1, 5, result);
  DEBUGOUT("[TLV_GetString] TEST 1 - Valid string: err=%d result=\"%s\" (expect \"Hello\")\r\n", 
           (int)err, result);
  
  // Test 2: Single character
  static const uint8_t test_data_2[] = {'A'};
  memset(result, 0, sizeof(result));
  err = TLV_GetString(test_data_2, 1, result);
  DEBUGOUT("[TLV_GetString] TEST 2 - Single char: err=%d result=\"%s\" (expect \"A\")\r\n", 
           (int)err, result);
  
  // Test 3: String with spaces
  static const uint8_t test_data_3[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
  memset(result, 0, sizeof(result));
  err = TLV_GetString(test_data_3, 11, result);
  DEBUGOUT("[TLV_GetString] TEST 3 - String with spaces: err=%d result=\"%s\" (expect \"Hello World\")\r\n", 
           (int)err, result);
  
  // Test 4: String with special characters
  static const uint8_t test_data_4[] = {'P', 'a', 's', 's', '@', '1', '2', '3'};
  memset(result, 0, sizeof(result));
  err = TLV_GetString(test_data_4, 8, result);
  DEBUGOUT("[TLV_GetString] TEST 4 - Special chars: err=%d result=\"%s\" (expect \"Pass@123\")\r\n", 
           (int)err, result);
  
  // Test 5: Empty string (length 0)
  memset(result, 0xFF, sizeof(result));
  err = TLV_GetString(test_data_1, 0, result);
  DEBUGOUT("[TLV_GetString] TEST 5 - Empty string: err=%d result=\"%s\" (expect \"\")\r\n", 
           (int)err, result);
  
  // Test 6: Max length string (64 chars)
  static const uint8_t test_data_6[] = {
    'A','B','C','D','E','F','G','H','I','J',
    'K','L','M','N','O','P','Q','R','S','T',
    'U','V','W','X','Y','Z','0','1','2','3',
    '4','5','6','7','8','9','-','_','.','!',
    '#','$','%','&','*','(',')','+','=','[',
    ']','{','}','|',';',':',',','<','>','?',
    '~',' ','@','\0'
  };
  memset(result, 0, sizeof(result));
  err = TLV_GetString(test_data_6, 64, result);
  DEBUGOUT("[TLV_GetString] TEST 6 - 64 char string: err=%d len=%lu (expect 64)\r\n", 
           (int)err, (unsigned long)strlen(result));
  
  // Test 7: NULL buffer
  memset(result, 0, sizeof(result));
  err = TLV_GetString(NULL, 5, result);
  DEBUGOUT("[TLV_GetString] TEST 7 - NULL buffer: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
  
  // Test 8: NULL result pointer
  err = TLV_GetString(test_data_1, 5, NULL);
  DEBUGOUT("[TLV_GetString] TEST 8 - NULL result: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
  
  // Test 9: Both NULL
  err = TLV_GetString(NULL, 5, NULL);
  DEBUGOUT("[TLV_GetString] TEST 9 - Both NULL: err=%d (expect %d)\r\n", 
           (int)err, (int)TLV_CONV_ERR_NULL);
}

void parser_run_all_tests(void)
{
  DEBUGOUT("\r\n========== TLV PARSER TESTS ==========\r\n");
  run_tlv_parser_tests();
  DEBUGOUT("[TLV TEST] DONE\r\n");
  
  DEBUGOUT("\r\n========== TLV CONVERSION TESTS ==========\r\n");
  run_tlv_get_uint8_tests();
  run_tlv_get_uint16_tests();
  run_tlv_get_uint32_tests();
  run_tlv_get_string_tests();
  DEBUGOUT("[TLV CONVERSION TESTS] DONE\r\n");
}
