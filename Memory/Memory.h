#ifndef __MEMORY_H__
#define __MEMORY_H__
#include <pthread.h>
#include "Block.h"
#include "../Queue/Queue.h"

#define SIZE (1 << 19) 

class Memory
{
public:
    Memory();
    ~Memory();
    Block * allocate(size_t amt);
    void lock();
    void unlock();
private:
    void free_queue();
    float * memory;
    size_t head;
    size_t tail;
    size_t size;
    Queue<MemoryBlock> blocks;
    pthread_mutex_t mutex;
};
#endif
