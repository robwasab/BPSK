#ifndef __STDIN_SOURCE_H__
#define __STDIN_SOURCE_H__

#include <pthread.h>
#include "../Module/Module.h"
#include "ByteInputInterface.h"

const char _STDIN_SOURCE_NAME_[] = "StdinSource";

class StdinSource: public ByteInputInterface
{
public:
    StdinSource(Memory * memory, 
            TransceiverCallback cb, 
            void * trans, 
            const uint16_t crc_table[]);
    void start(bool block = true);
    void stop();
    void dispatch(RadioMsg * msg);
    const char * name() {
        return _STDIN_SOURCE_NAME_; 
    }
    int fd[2];
private:
    pthread_t main;
};

#endif
