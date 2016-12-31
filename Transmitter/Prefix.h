#ifndef __PREFIX_H__
#define __PREFIX_H__

#include "../Module/Module.h"
#include "../Memory/Memory.h"
#include "../TaskScheduler/TaskScheduler.h"

// "pound define maximum length header bits is 8"
#define ML_BITS 4
#define RAND_LEN ((1 << ML_BITS) - 1)

const char _PREFIX_NAME_[] = "Prefix";

class Prefix : public Module
{
public:
    Prefix(Memory * memory, 
            Module * next);

    Block * process(Block * block);
    using Module::name;
    const char * name() {
        return _PREFIX_NAME_;
    }
private:
    bool shift_register[ML_BITS];
    bool rand[RAND_LEN];
    void encode_helper(Block * encode, bool inv);
};

#endif
