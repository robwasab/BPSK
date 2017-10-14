#ifndef __RADIO_MSG_H__
#define __RADIO_MSG_H__

#include "../Memory/Block.h"

enum RadioMsgType
{
    PROCESS_DATA,
    CMD_START,
    CMD_STOP,
    CMD_RESET_ALL,
    CMD_RESET_TRANSMITTER,
    CMD_RESET_RECEIVER,
    CMD_SET_TRANSMIT_CHANNEL,
    CMD_SET_RECEIVE_CHANNEL,
    CMD_SET_NOISE_LEVEL,
    CMD_TEST_PSK8_SIG_GEN,
    NOTIFY_MSG_EXHAUSTED,
    NOTIFY_PLL_RESET,
    NOTIFY_PLL_LOST_LOCK,
    NOTIFY_PLL_LOCK,
    NOTIFY_PACKET_HEADER_DETECTED,
    NOTIFY_RECEIVER_RESET_CONDITION_DETECTED,
    
    /* The following enumerations
     * - NOTIFY_DATA_START
     * - NOTIFY_DATA_BODY
     * Represent the mechanism in which the Transciever returns valid received data.
     * In order to avoid calls to the OS to allocate data, the received data will be broken
     * into chunks of RADIO_ARG_SIZE.
     * 
     * NOTIFY_DATA_START shall contain the length of the message in bytes, which will not exceed
     * 256 bytes. The first byte of RadioMsg.args shall contain this length.
     * 
     * NOTIFY_DATA_BODY will contain the data. The implementation receiving the data will need to keep
     * track of how much data to read using the length indicated initially by the NOTIFY_DATA_START message.
     */
    NOTIFY_DATA_START,
    NOTIFY_DATA_BODY,  
    NOTIFY_USER_REQUEST_QUIT,
    NOTIFY_CRC_CORRUPTED,
    RADIO_MSG_TYPE_LEN,
};

typedef enum RadioMsgType RadioMsgType;

#define RADIO_ARG_SIZE 16

class RadioMsg
{
    public:
        RadioMsg(RadioMsgType type=PROCESS_DATA);
        RadioMsgType type;
        uint8_t args[RADIO_ARG_SIZE];
};

class RadioData : public RadioMsg
{
    public:
        void set_id(uint8_t id);
        void set_thread_id(uint8_t thread_id);
        void set_tid(uint8_t tid);
        void set_block(Block * block);
        uint8_t get_id();
        uint8_t get_thread_id();
        uint8_t get_tid();
        Block * get_block();
};

extern char RadioMsgString[][64];

#endif
