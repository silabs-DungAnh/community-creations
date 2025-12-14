#include "uart_comm.h"
#include "rsi_debug.h"
#include "decode.h"
#include "crc16.h"
#include "string.h"

#define PRINT_PAYLOAD_ENABLE 0
#define FSM_TYPE_DEBUG_ENABLE 0
#define FSM_LENGTH_DEBUG_ENABLE 0
#define FSM_CRC_DEBUG_ENABLE 0


// ==================== Initialization of the decoder =======================
uart_decode_error_t fsm_decoder_init(uart_fsm_decoder_t *decoder) {

    if (!decoder) {
        DECODE_LOG ("Decoder pointer is NULL\n");
        return UART_DECODE_INIT_FAILED; // check NULL
    }

    decoder->state = WAIT_FOR_HEADER;

    memset(&decoder->packet, 0, sizeof(uart_packet_t));
    memset(decoder->packet_temp_buffer, 0, sizeof(decoder->packet_temp_buffer));
    decoder->packet_temp_buffer_index = 0;

    decoder->calculated_crc = 0;
    decoder->received_crc = 0;

    decoder->total_packets_parsed = 0;
    decoder->packets_accepted = 0;
    decoder->packets_rejected = 0;
    decoder->bad_checksum_count = 0;
    decoder->bad_length_count = 0;

    DECODE_LOG ("UART FSM Decoder initialized successfully\n");
    return UART_DECODE_INIT_SUCCESS;
}

// ============================= DEBUG FUNCTIONS =============================
void uart_fsm_print_packet(const uart_packet_t *packet){
    DECODE_LOG("\n--- PACKET DETAILS ---\n");
    DECODE_LOG("SOF: 0x%02X\n", packet->sof);
    DECODE_LOG("Type: 0x%04X\n", packet->type);
    DECODE_LOG("Length: %u\n", packet->length);
    DECODE_LOG("Payload: ");
    for (uint16_t i = 0; i < packet->length; i++) {
        DECODE_LOG("%02X ", packet->payload[i]);
    }
    DECODE_LOG("\nEndcode: 0x%02X\n", packet->endcode);
    DECODE_LOG("CRC: 0x%04X\n", packet->crc);
    DECODE_LOG("Tail: 0x%02X\n", packet->tail);
    DECODE_LOG("----------------------\n");
}
// ============================= Validation functions =======================
uart_decode_error_t packet_length_validate (uart_packet_t *pkt){
    if (pkt->length == 0 || pkt->length > UART_MAX_PACKET_LEN){
        DECODE_LOG ("Packet length is invalid: %u \n", pkt->length);
        return UART_DECODE_ERR_INVALID_LENGTH;
    }
    return UART_DECODE_LENGTH_VALID;
}

// ==================================== Decode FSM =====================================
void decode_fsm (uart_fsm_decoder_t *decoder, uint8_t byte){
    switch (decoder->state) {
        case WAIT_FOR_HEADER:{
            
            if (byte == UART_HEADER) {
                decoder->packet.sof = byte;
                // Print header 
                DECODE_LOG ("Received HEADER: 0x%02X\n", byte);

                // Reset CRC accumulator 
                decoder->calculated_crc = CRC16_CCITT_INIT;

                decoder->state = READ_TYPE;
                DECODE_LOG("State: WAIT_FOR_HEADER -> READ_TYPE\n");
            }
            break;
        }

        case READ_TYPE: {
            if (decoder->packet_temp_buffer_index == 0) {
                decoder->packet.type  = (uint16_t)byte;
            } else if (decoder->packet_temp_buffer_index == 1) {
                decoder->packet.type |= (uint16_t)byte << 8;
            }
            // CRC include TYPE
            decoder->calculated_crc = crc16_ccitt_update(decoder->calculated_crc, byte);
            decoder->packet_temp_buffer_index++; 


            // Print when both bytes of TYPE are read
            if (decoder->packet_temp_buffer_index == 2) {
                DECODE_LOG("Received PACKET TYPE: 0x%04X\n", decoder->packet.type);


                // Print individual bytes of TYPE
                #if FSM_TYPE_DEBUG_ENABLE == 1
                uint8_t type_lsb =  (uint8_t)(decoder->packet.type & 0xFF);
                uint8_t type_msb =  (uint8_t)((decoder->packet.type >> 8) & 0xFF);
                DECODE_LOG("Received TYPE bytes: LSB=0x%02X, MSB=0x%02X\n", type_lsb, type_msb);
                #endif

                // State transition
                decoder->state         = READ_LENGTH;
                decoder->packet_temp_buffer_index = 0;
                DECODE_LOG("State: READ_TYPE -> READ_LENGTH\n");
            }
            break;
        }

        case READ_LENGTH: {
            if (decoder->packet_temp_buffer_index == 0) {
                decoder->packet.length = (uint16_t)byte;
            } else if (decoder->packet_temp_buffer_index == 1) {
                decoder->packet.length |= ((uint16_t)byte << 8);
            }

            // CRC include TYPE
            decoder->calculated_crc = crc16_ccitt_update(decoder->calculated_crc, byte);
            decoder->packet_temp_buffer_index++;

            // 2bytes read → validate length
            if (decoder->packet_temp_buffer_index == 2) {

                #if FSM_LENGTH_DEBUG_ENABLE == 1
                uint8_t len_LSB = (uint8_t)(decoder->packet.length & 0xFF);
                uint8_t len_MSB = (uint8_t)((decoder->packet.length >> 8) & 0xFF);
                DECODE_LOG("Received LENGTH bytes: LSB=0x%02X, MSB=0x%02X\n", len_LSB, len_MSB);
                #endif
                        
                DECODE_LOG("Packet LENGTH raw = %u (0x%04X)\n",  decoder->packet.length, decoder->packet.length);

                // Validate length
                if (!packet_length_validate(&decoder->packet)) {
                    decoder->state = WAIT_FOR_HEADER;
                    decoder->bad_length_count++;
                    DECODE_LOG("Invalid length. Resetting to WAIT_FOR_HEADER\n");
                    decoder->packet_temp_buffer_index = 0;
                    break;
                }

                // Transit to READ_PAYLOAD
                decoder->state         = READ_PAYLOAD;
                decoder->packet_temp_buffer_index = 0;
                DECODE_LOG("State: READ_LENGTH -> READ_PAYLOAD\n");
            }

            break;
        }

        // TODO: Handle multiple TLV types in one payload
        case READ_PAYLOAD: {
            // Write byte to payload buffer
            if (decoder->packet_temp_buffer_index < UART_MAX_PACKET_LEN) {
                decoder->packet_temp_buffer[decoder->packet_temp_buffer_index++] = byte;

                // CRC include payload byte
                decoder->calculated_crc = crc16_ccitt_update(decoder->calculated_crc, byte);
            } else {
                DECODE_LOG("Payload buffer overflow. Resetting to WAIT_FOR_HEADER\n");
                decoder->state                       = WAIT_FOR_HEADER;
                decoder->packet_temp_buffer_index    = 0;
                decoder->packets_rejected++;
                break;
            }

            // If received full payload
            if (decoder->packet_temp_buffer_index >= decoder->packet.length) {
                // Copy from temp buffer to packet.payload
                memcpy(decoder->packet.payload,
                    decoder->packet_temp_buffer,
                    decoder->packet.length);
        
                #ifndef PRINT_PAYLOAD_ENABLE
                // Print the full payload
                for (uint16_t i = 0; i < decoder->packet.length; i++) {
                    DECODE_LOG("Payload[%u]: 0x%02X\n", i, decoder->packet.payload[i]);
                }
                #endif

                DECODE_LOG("Full payload received (%u bytes)\n", decoder->packet.length);
                

                // State transition to READ_ENDCODE
                decoder->state                    = READ_ENDCODE;
                decoder->packet_temp_buffer_index = 0;
                DECODE_LOG("State: READ_PAYLOAD -> READ_ENDCODE\n");
            }

            break;
        }

        case READ_ENDCODE: {
            
            if (byte == UART_ENDCODE) {
                decoder->packet.endcode = byte;

                // Print endcode
                DECODE_LOG ("Received ENDCODE: 0x%02X\n", byte);

                // CRC include TYPE
                decoder->calculated_crc = crc16_ccitt_update(decoder->calculated_crc, byte);

                decoder->state = READ_CRC;
                decoder->packet_temp_buffer_index = 0;
                DECODE_LOG("State: READ_ENDCODE -> READ_CRC\n");
            } else {
                decoder->state = WAIT_FOR_HEADER;
                DECODE_LOG("Invalid endcode. Resetting to WAIT_FOR_HEADER\n");
            }
            break;
        }

        case READ_CRC: {
            // Receive 2 bytes of CRC
            if (decoder->packet_temp_buffer_index == 0) {
                decoder->received_crc = (uint16_t)byte;          // LSB
            } else if (decoder->packet_temp_buffer_index == 1) {
                decoder->received_crc |= (uint16_t)byte << 8;    // MSB
            }
            decoder->packet_temp_buffer_index++;

            // When received all 2 bytes of CRC
            if (decoder->packet_temp_buffer_index == UART_CRC_SIZE) {

                #if FSM_CRC_DEBUG_ENABLE == 1
                DECODE_LOG("Received CRC:    0x%04X\n", decoder->received_crc);
                DECODE_LOG("Calculated CRC:  0x%04X\n", decoder->calculated_crc);
                #endif 

                if (decoder->calculated_crc == decoder->received_crc) {
                    decoder->packet.crc  = decoder->received_crc;
                    decoder->state       = READ_TAIL;
                    DECODE_LOG("CRC OK. State: READ_CRC -> READ_TAIL\n");
                } else {
                    decoder->state = WAIT_FOR_HEADER;
                    decoder->bad_checksum_count++;
                    decoder->packets_rejected++;
                    DECODE_LOG("CRC mismatch. Resetting to WAIT_FOR_HEADER\n");
                }

                // Reset index for next state
                decoder->packet_temp_buffer_index = 0;
            }
            break;
        }


        case READ_TAIL:{
            
            if (byte == UART_TAIL) {
                decoder->packet.tail = byte;
                decoder->total_packets_parsed++;
                decoder->packets_accepted++;
                DECODE_LOG("Packet accepted!\n");
                uart_fsm_print_packet(&decoder->packet);
                decoder->state = WAIT_FOR_HEADER; // Reset for next packet
                decoder->packet_temp_buffer_index = 0;
                DECODE_LOG("State: READ_TAIL -> WAIT_FOR_HEADER\n");
            } else {
                decoder->state = WAIT_FOR_HEADER;
                DECODE_LOG("Invalid tail. Resetting to WAIT_FOR_HEADER\n");
            }
            break;
        }

        default:{
            
            decoder->state = WAIT_FOR_HEADER;
            DECODE_LOG("Unknown state. Resetting to WAIT_FOR_HEADER\n");
            break;
        }
    }
}


