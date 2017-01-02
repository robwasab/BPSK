#ifndef __WAVSINK_H__
#define __WAVSINK_H__

#include <stdio.h>
#include "../Module/Module.h"

class WavSink : public Module
{
public:
    WavSink(Memory * memory, Module * next, const char filename[] = "output.wav");
    ~WavSink();
    Block * process(Block * block);
    const char * name();
private:
    FILE * file;
    int16_t * buffer;
};
#endif
