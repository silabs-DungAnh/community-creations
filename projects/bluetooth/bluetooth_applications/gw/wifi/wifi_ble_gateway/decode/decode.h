#ifndef SRC_INC_DECODE_H_
#define SRC_INC_DECODE_H_

#include <stdint.h>
#include <stdbool.h>
#include "rsi_debug.h"
#define DECODE_LOG_ENABLE 1

// Debug log macro - just use DEBUGOUT directly without prefix
#if DECODE_LOG_ENABLE == 0
    #define DECODE_LOG(format, ...)
#else   
#define DECODE_LOG(format, ...) DEBUGOUT(format, ##__VA_ARGS__)
#endif


#define UART_HEADER             0xF0
#define UART_ENDCODE            0xFF
#define UART_TAIL               0x0F
#define UART_MAX_PACKET_LEN     512
#define UART_MAX_PAYLOAD_LEN    256

// Constants for calculating CRC
#define UART_HEADER_SIZE        1
#define UART_TYPE_SIZE          2
#define UART_LENGTH_SIZE        2
#define UART_ENCODE_SIZE        1
#define UART_CRC_SIZE           2
#define UART_TAIL_SIZE          1
#define CRC16_FIXED_LENGTH      (UART_TYPE_SIZE + UART_LENGTH_SIZE  + UART_ENCODE_SIZE)    

// UART Packet structure
typedef struct {
    uint8_t            sof;                             // 1 byte
    uint16_t           type;                            // 2 bytes (uint16_t)
    uint16_t           length;                          // 2 bytes
    uint8_t          payload[UART_MAX_PAYLOAD_LEN];   // L max
    uint8_t            endcode;                         // 1 byte
    uint16_t           crc;                             // 2 bytes
    uint8_t            tail;                            // 1 byte
} uart_packet_t;

// FSM states for UART decoding
typedef enum {
    WAIT_FOR_HEADER,
    READ_TYPE,
    READ_LENGTH,
    READ_PAYLOAD,
    READ_ENDCODE,
    READ_CRC,
    READ_TAIL,
} uart_decode_state_t;

// Decoder structure for Message Queue
typedef struct {
    uart_decode_state_t state;
    uart_packet_t packet;
    uint16_t packet_temp_buffer_index;
    uint8_t packet_temp_buffer[UART_MAX_PACKET_LEN];
    uint16_t payload_rx_len;
    
    // CRC
    uint16_t calculated_crc;
    uint16_t received_crc;

    // Statistics
    uint32_t total_packets_parsed;
    uint32_t packets_accepted;
    uint32_t packets_rejected;
    uint32_t bad_checksum_count;
    uint32_t bad_length_count;
} uart_fsm_decoder_t;

// Decode error/success codes
typedef enum {
    /* ==================== SUCCESS CODES ==================== */
    UART_DECODE_SUCCESS             = 0,    /* Packet decoded successfully */
    UART_DECODE_INIT_SUCCESS        = 1,    /* Decoder initialized successfully */
    UART_DECODE_CRC_VALID           = 2,    /* CRC is valid */
    UART_DECODE_TYPE_VALID          = 3,    /* Packet type is valid */
    UART_DECODE_LENGTH_VALID        = 4,    /* Packet length is valid */
    
    /* ==================== ERROR CODES ==================== */
    UART_DECODE_ERR_INVALID_TYPE    = 10,   /* Invalid packet type */
    UART_DECODE_ERR_INVALID_LENGTH  = 11,   /* Invalid packet length */
    UART_DECODE_ERR_INVALID_PAYLOAD = 12,   /* Invalid payload */
    UART_DECODE_ERR_INVALID_CRC     = 13,   /* CRC mismatch */
    UART_DECODE_ERR_INVALID_ENDCODE = 14,   /* Invalid endcode */
    UART_DECODE_ERR_INVALID_TAIL    = 15,   /* Invalid tail */
    UART_DECODE_ERR_BUFFER_OVERFLOW = 16,   /* Buffer overflow */
    UART_DECODE_FAILED              = 17,   /* Generic decode failed */
    UART_DECODE_INIT_FAILED         = 18,   /* Decoder initialization failed */
    UART_DECODE_QUEUE_FULL          = 19,   /* Message queue is full */
    UART_DECODE_QUEUE_PUT_FAILED    = 20,   /* Failed to put packet in queue */
} uart_decode_error_t;

// Decoder initialization 
uart_decode_error_t fsm_decoder_init (uart_fsm_decoder_t *decoder);

// Validation functions
uart_decode_error_t packet_type_validate (uart_packet_t *pkt);
uart_decode_error_t packet_length_validate (uart_packet_t *pkt);   

// Debug utilities
void uart_fsm_print_packet(const uart_packet_t *packet);

// States machine for UART decoding
void decode_fsm (uart_fsm_decoder_t *decoder, uint8_t byte);

#endif /* SRC_INC_DECODE_H_ */
