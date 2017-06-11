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
    NOTIFY_PLL_RESET,
    NOTIFY_PACKET_HEADER_DETECTED,
    NOTIFY_RECEIVER_RESET_CONDITION_DETECTED,
    NOTIFY_DATA_RECEIVED,
    NOTIFY_USER_REQUEST_QUIT,
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
