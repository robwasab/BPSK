#include "RadioMsg.h"
#include "../Memory/Block.h"
#include <stdlib.h>
#include <string.h>

char RadioMsgString[][64] = 
{
    [PROCESS_DATA] = "PROCESS_DATA",
    [CMD_START] = "CMD_START",
    [CMD_STOP]  = "CMD_STOP",
    [CMD_RESET_ALL] = "CMD_RESET_ALL",
    [CMD_RESET_TRANSMITTER] = "CMD_RESET_TRANSMITTER",
    [CMD_RESET_RECEIVER] = "CMD_RESET_RECEIVER",
    [CMD_SET_TRANSMIT_CHANNEL] = "CMD_SET_TRANSMIT_CHANNEL",
    [CMD_SET_RECEIVE_CHANNEL] = "CMD_SET_RECEIVE_CHANNEL",
    [CMD_SET_NOISE_LEVEL] = "CMD_SET_NOISE_LEVEL",
    [CMD_TEST_PSK8_SIG_GEN] = "CMD_TEST_PSK8_SIG_GEN",
    [NOTIFY_PLL_RESET] = "NOTIFY_PLL_RESET",
    [NOTIFY_PACKET_HEADER_DETECTED] = "NOTIFY_PACKET_HEADER_DETECTED",
    [NOTIFY_RECEIVER_RESET_CONDITION_DETECTED] = "NOTIFY_RECEIVER_RESET_CONDITION_DETECTED",
    [NOTIFY_DATA_RECEIVED] = "NOTIFY_DATA_RECEIVED",
};

RadioMsg::RadioMsg(RadioMsgType type):
    type(type)
{
    memset(args, 0, sizeof(args));
}

void RadioData::set_id(uint8_t id)
{
    args[0] = id;
}

void RadioData::set_thread_id(uint8_t thread_id)
{
    args[1] = thread_id;
}

void RadioData::set_tid(uint8_t thread_id)
{
    args[1] = thread_id;
}

void RadioData::set_block(Block * block)
{
    memcpy(&args[2], &block, sizeof(Block *));
}

uint8_t RadioData::get_id()
{
    return args[0];
}

uint8_t RadioData::get_thread_id()
{
    return args[1];
}

uint8_t RadioData::get_tid()
{
    return args[1];
}

Block * RadioData::get_block()
{
    Block * ret;
    memcpy(&ret, &args[2], sizeof(Block *));
    return ret;
}

