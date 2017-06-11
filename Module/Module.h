#ifndef __MODULE_H__
#define __MODULE_H__

#include <stdint.h>
#include "../RadioMsg/RadioMsg.h"
#include "../Colors/Colors.h"
#include "../Memory/Memory.h"
#include "../Memory/Block.h"
#include "../Log/Log.h"

typedef void (*TransceiverCallback)(void * obj, RadioMsg * msg);

class Module
{
public:
    Module(Memory * memory, TransceiverCallback cb, void * transceiver);

    virtual ~Module();

    virtual void handoff(Block * block, uint8_t thread_id=0);

    virtual Block * process(Block * block) = 0;

    /* To be used to process transceiver messages */
    virtual void dispatch(RadioMsg * msg);

    /* For notifying the transceiver if something happens */
    virtual void broadcast(RadioMsg * msg);

    virtual const char * name() = 0;
    Memory * memory;
    TransceiverCallback transceiver_cb;
    void * transceiver;
    uint8_t id;
private:
};

#endif
