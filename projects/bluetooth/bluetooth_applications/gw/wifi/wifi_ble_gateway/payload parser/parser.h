#ifndef PAYLOAD_PARSER_PARSER_H_
#define PAYLOAD_PARSER_PARSER_H_
#include <stdint.h>

/*
    TLV token structure
*/
typedef struct {
    uint16_t type;
    uint16_t length;
    uint8_t* value;
} TLV_token_t;


/*
    Error codes
*/
typedef enum {
    TLV_ERR_UNKNOWN = -1,
    TLV_ERR_NONE = 0,
    TLV_ERR_INVALID_LENGTH,
    TLV_ERR_INVALID_TYPE,
    TLV_ERR_PARSING_FAILED,
    TLV_ERR_BUFFER_OVERFLOW,
} TLV_error_t;

/*
    Function to parse TLV formatted payload
    Parameters:
        payload: Pointer to the input payload buffer
        payload_length: Length of the input payload buffer
        tokens: Pointer to an array of TLV_token_t to store parsed tokens
        max_tokens: Maximum number of tokens that can be stored in the tokens array
        parsed_token_count: Pointer to store the number of successfully parsed tokens
    Returns:
        TLV_error_t indicating success or type of error encountered
*/
TLV_error_t parse_tlv_payload(const uint8_t* payload, uint32_t payload_length,
                              TLV_token_t* tokens, uint32_t max_tokens,
                              uint32_t* parsed_token_count);



#endif /* PAYLOAD_PARSER_PARSER_H_ */
