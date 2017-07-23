#ifndef __BYTE_INTERFACE_H__
#define __BYTE_INTERFACE_H__

#include "../Module/Module.h"

const char _BYTE_INTERFACE_NAME_[] = "StdinSource";

class ByteInterface: public Module
{
public:
    ByteInterface(Memory * memory, TransceiverCallback cb, void * trans);
    Block * process(Block * block);
    const char * name() {
        return _BYTE_INTERFACE_NAME_;
    }
private:
};

#endif
