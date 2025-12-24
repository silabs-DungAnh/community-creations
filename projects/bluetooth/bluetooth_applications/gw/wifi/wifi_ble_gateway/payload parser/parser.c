#include "parser.h"
#include "stdint.h"
#include "string.h"

tlv_parsing_error_t parse_tlv_payload
                                    (
                                    const uint8_t*  input_payload,
                                    uint32_t        input_full_payload_length,
                                    tlv_token_t*    output_parsed_tokens,
                                    uint32_t        max_tokens_arr_capacity,
                                    uint32_t*       parsed_token_count
                                    )
{

    if (input_payload == NULL || output_parsed_tokens == NULL || parsed_token_count == NULL) {
        return  TLV_ERR_NULL;
    }

    uint32_t offset = 0;
    uint32_t token_index = 0;

    while (offset + 4 <= input_full_payload_length) { // Ensure enough bytes for type and length
        if (token_index >= max_tokens_arr_capacity) {
            return TLV_ERR_BUFFER_OVERFLOW;
        }

        // Combine 2 bytes of type and length into a 2byte-value by big-edian order
        uint16_t type = (input_payload[offset] << 8) | input_payload[offset + 1];
        offset += 2;

        uint16_t length = (input_payload[offset] << 8) | input_payload[offset + 1];
        offset += 2;

        // Check if the remaining output_payload is sufficient for the value
        if (offset + length > input_full_payload_length) {
            return TLV_ERR_INVALID_LENGTH;
        }

        // Assign values to the token
        output_parsed_tokens[token_index].type = type;
        output_parsed_tokens[token_index].length = length;
        output_parsed_tokens[token_index].value = (uint8_t*)&input_payload[offset];

        // Move offset forward by the length of the value to read the next token
        offset += length;
        token_index++;
    }

    // Set the number of parsed tokens
    *parsed_token_count = token_index;
    return TLV_ERR_NONE;
}

tlv_parsing_error_t tlv_getUint8   (const uint8_t* buf, uint8_t* result){
    if (buf == NULL || result == NULL) {
        return TLV_CONV_ERR_NULL;
    }

    *result = buf[0]; 
    return TLV_CONV_ERR_NONE;
}

tlv_parsing_error_t tlv_getUint16  (const uint8_t* buf, uint16_t* result){
    if (buf == NULL || result == NULL) {
        return TLV_CONV_ERR_NULL;
    }

    *result = (buf[0] << 8) | buf[1]; 
    return TLV_CONV_ERR_NONE;
}

tlv_parsing_error_t tlv_getUint32  (const uint8_t* buf, uint32_t* result){
    if (buf == NULL || result == NULL) {
        return TLV_CONV_ERR_NULL;
    }

    *result = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]; 
    return TLV_CONV_ERR_NONE;
}

tlv_parsing_error_t tlv_getString  (const uint8_t* buf, uint32_t buf_len, char* out, uint32_t out_len){
    if (buf == NULL || out == NULL) {
        return TLV_CONV_ERR_NULL;
    }

    if (buf_len + 1 > out_len) { // +1 for null-terminator
        return TLV_CONV_ERR_BUFFER_TOO_SMALL;
    }
    
    // Copy bytes to output string 
    memcpy(out, buf, buf_len);

    // Null-termination: End of a string (C-style)
    out[buf_len] = '\0'; 
    return TLV_CONV_ERR_NONE;
}


