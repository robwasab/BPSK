#ifndef __QPSK_PREFIX_H__
#define __QPSK_PREFIX_H__

#include "../Module/Module.h"
#include "../Memory/Memory.h"
#include "../TaskScheduler/TaskScheduler.h"

// "pound define maximum length header bits is 8"

const char _QPSK_PREFIX_NAME_[] = "QPSK Prefix";

class QPSK_Prefix : public Module
{
public:
    QPSK_Prefix(Memory * memory, 
            Module * next);

    ~QPSK_Prefix() {
        delete [] qpsk_prefix;
    }

    Block * process(Block * block);
    using Module::name;
    const char * name() {
        return _QPSK_PREFIX_NAME_;
    }
private:
    int qpsk_prefix_len;
    int * qpsk_prefix;
    void encode_helper(Block * encode, bool inv);
};

#endif
