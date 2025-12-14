#include "parser.h"
#include "stdint.h"
#include "string.h"

TLV_error_t parse_tlv_payload(const uint8_t* payload, uint32_t payload_length,
                              TLV_token_t* tokens, uint32_t max_tokens,
                              uint32_t* parsed_token_count) {
    if (payload == NULL || tokens == NULL || parsed_token_count == NULL) {
        return TLV_ERR_PARSING_FAILED;
    }

    uint32_t offset = 0;
    uint32_t token_index = 0;

    while (offset + 4 <= payload_length) { // Ensure enough bytes for type and length
        if (token_index >= max_tokens) {
            return TLV_ERR_BUFFER_OVERFLOW;
        }

        // Read type (2 bytes)
        uint16_t type = (payload[offset] << 8) | payload[offset + 1];
        offset += 2;

        // Read length (2 bytes)
        uint16_t length = (payload[offset] << 8) | payload[offset + 1];
        offset += 2;

        // Check if the remaining payload is sufficient for the value
        if (offset + length > payload_length) {
            return TLV_ERR_INVALID_LENGTH;
        }

        // Assign values to the token
        tokens[token_index].type = type;
        tokens[token_index].length = length;
        tokens[token_index].value = (uint8_t*)&payload[offset];

        // Move offset forward by the length of the value
        offset += length;
        token_index++;
    }

    *parsed_token_count = token_index;
    return TLV_ERR_NONE;
}
