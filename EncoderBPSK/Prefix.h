#ifndef __PREFIX_H__
#define __PREFIX_H__

#include "../Module/Module.h"
#include "../Memory/Memory.h"
#include "../TaskScheduler/TaskScheduler.h"

// "pound define maximum length header bits is 8"

const char _PREFIX_NAME_[] = "Prefix";

class Prefix : public Module
{
public:
    Prefix(Memory * memory, 
            TransceiverCallback cb,
            void * trans,
            bool * prefix,
            size_t prefix_len);

    ~Prefix() {
        delete [] prefix;
    }

    Block * process(Block * block);
    using Module::name;
    const char * name() {
        return _PREFIX_NAME_;
    }
private:
    size_t prefix_len;
    bool * prefix;
    void encode_helper(Block * encode, bool inv);
};

#endif
