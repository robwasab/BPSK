#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "../RadioMsg/RadioMsg.h"
#include "../Module/Module.h"

class Channel : public Module
{
public:
    Channel(Memory * rx_memory, TransceiverCallback cb, void * transceiver);
    void dispatch(RadioMsg * msg);
    const char * name();
private:
    Block * process(Block * block);
};

#endif
