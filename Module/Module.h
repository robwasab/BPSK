#ifndef __MODULE_H__
#define __MODULE_H__

#include <typeinfo>
#include "../Colors/Colors.h"
#include "../Memory/Block.h"
#include "../Memory/Memory.h"
#include <stdint.h>

#define LOG(...) BLUE; printf("%s %s(): ", __FILE__, __func__); ENDC; printf(__VA_ARGS__)
#define WARNING(...) BLINK; YELLOW; printf("%s %s(): ", __FILE__, __func__); ENDC; printf(__VA_ARGS__)
#define ERROR(...) BLINK; UNDERLINE; BOLD; RED; printf("%s %s(): ", __FILE__, __func__); ENDC; printf(__VA_ARGS__)

class Module
{
public:
    Module(Memory * memory, Module * next) :
        memory(memory),
        next(next)
    {
    }
    virtual ~Module() {
    }

    virtual const char * name() = 0;
    virtual Block * process(Block * in) = 0;
    Memory * memory;
    Module * next;
private:
};

#endif
