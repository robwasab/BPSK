#ifndef __SUPPRESS_PRINT_H__
#define __SUPPRESS_PRINT_H__
#include "../Module/Module.h"

static char __suppress_print_name__[] = "SuppressPrint";

class SuppressPrint : public Module
{
public:
    SuppressPrint():
    Module(NULL, NULL, NULL) {}

    const char * name() {
        return __suppress_print_name__;
    }

    Block * process(Block * in) {
        in->free();
        return NULL;
    }
};

#endif
