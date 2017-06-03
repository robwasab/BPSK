#ifndef __NOTIFY_H__
#define __NOTIFY_H__

#include "../Module/Module.h"

#define NOTIFY_TYPE_BYTES   0
#define NOTIFY_TYPE_MODULE  1

#define NOTIFY_START        0
#define NOTIFY_DONE         1
#define NOTIFY_RECEIVE_DATA 2
#define NOTIFY_RESET        3
#define NOTIFY_BROADCAST    4

struct Notify
{
    uint8_t data_type;
    uint8_t notify_type;
    Module  * module;
    Block   * block;
    uint8_t * data;
    size_t len;
};

typedef struct Notify Notify;

#endif

