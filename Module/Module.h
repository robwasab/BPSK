#ifndef __MODULE_H__
#define __MODULE_H__
#include <typeinfo>
#include "../Memory/Block.h"
#include "../Memory/Memory.h"
#include <stdint.h>

class Module
{
public:
    Module(Memory * memory, Module * next) :
        memory(memory),
        next(next)
    {
    }

    virtual const char * name() = 0;
    virtual Block * process(Block * in) = 0;
    Memory * memory;
    Module * next;
private:
};


#endif
