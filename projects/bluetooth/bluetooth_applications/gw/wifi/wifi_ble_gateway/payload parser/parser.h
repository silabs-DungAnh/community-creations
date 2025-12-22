#ifndef PAYLOAD_PARSER_PARSER_H_
#define PAYLOAD_PARSER_PARSER_H_
#include <stdint.h>

/*
    Enum for TLV types
*/
typedef enum {
    TLV_TYPE_WIFI_CRED   = 0x1001,
    TLV_TYPE_MAC_ADDRESS = 0x01,
    TLV_TYPE_UUID        = 0x02,
    TLV_TYPE_STATUS      = 0x03,
    TLV_TYPE_UUID_VALUE  = 0x04,
    TLV_TYPE_OTA_VALUE   = 0x05,
    TLV_TYPE_WIFI_CONFIG = 0x06,
} TLV_types_t;


/*
    TLV token structure
*/
typedef struct {
    TLV_types_t type;
    uint16_t    length;
    uint8_t*    value;
} TLV_token_t;


/*
    Error codes for parsing process
*/
typedef enum {
    TLV_ERR_UNKNOWN = -1,
    TLV_ERR_NONE,
    TLV_ERR_INVALID_LENGTH,
    TLV_ERR_INVALID_TYPE,
    TLV_ERR_PARSING_FAILED,
    TLV_ERR_BUFFER_OVERFLOW,
    TLV_ERR_NULL,
} TLV_parsing_error_t;

/*
    Error codes for data conversion functions
*/
typedef enum {
    TLV_CONV_ERR_NONE = 0,          // No error occurred during conversion
    TLV_CONV_ERR_INVALID_LENGTH,    // Value length does not match the expected size for this data type
    TLV_CONV_ERR_NULL,              // Null pointer passed as input or output buffer
    TLV_CONV_ERR_TYPE_UNSUPPORTED,  // TLV type is not supported by the conversion function
    TLV_CONV_ERR_OUT_OF_RANGE,      // Converted value is outside the valid range for the target type
    TLV_CONV_ERR_ENDIANNESS,        // Endianness mismatch or invalid byte order detected
    TLV_CONV_ERR_ENCODING,          // String or text encoding error (e.g., invalid UTF-8 sequence)
    TLV_CONV_ERR_BUFFER_TOO_SMALL,  // Output buffer is too small to hold the converted result
    TLV_CONV_ERR_FORMAT,            // Value format is invalid or does not follow TLV specification
} TLV_conv_error_t;

/*
    Function to parse TLV formatted payload
    @params:
        input_payload:              Pointer to the input payload buffer
        input_full_payload_length:  Length of the input payload buffer
        output_parsed_tokens:       Pointer to an array of TLV_token_t to store parsed tokens
        max_tokens_arr_capacity:    Maximum number of tokens that can be stored in the tokens array
        parsed_token_count:         Pointer to store the number of successfully parsed tokens
    @return:
        TLV_error_t indicating success or type of error encountered
*/
TLV_parsing_error_t parse_tlv_payload(
                            const uint8_t*  input_payload, 
                            uint32_t        input_full_payload_length,
                            TLV_token_t*    output_parsed_tokens, 
                            uint32_t        max_tokens,
                            uint32_t*       parsed_token_count
                            );


/*
    Functions to convert parsed-tokens byte array to other data types based on token types
    @params:
        buf:        Pointer to the byte array representing the TLV value
        result:     Pointer to store the converted result
    @return:
        TLV_conv_error_t indicating success or type of error encountered
*/
TLV_conv_error_t TLV_GetUint8   (const uint8_t* buf, uint8_t* result);
TLV_conv_error_t TLV_GetUint16  (const uint8_t* buf, uint16_t* result);
TLV_conv_error_t TLV_GetUint32  (const uint8_t* buf, uint32_t* result);

/*
    Function to convert TLV value to string
    @params:
        buf:    Pointer to the byte array representing the TLV value
        len:    Length of the TLV value
        out:    Pointer to the output string buffer
    @return:
        TLV_conv_error_t indicating success or type of error encountered
*/
TLV_conv_error_t TLV_GetString  (const uint8_t* buf, uint32_t len, char* out);


#endif /* PAYLOAD_PARSER_PARSER_H_ */
