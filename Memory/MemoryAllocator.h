#ifndef __MEMORY_ALLOCATOR_H__
#define __MEMORY_ALLOCATOR_H__

#include <stdint.h>
#include "../Colors/Colors.h"

#define LOG(...) BLUE; printf("%s %s(): ", __FILE__, __func__); ENDC; printf(__VA_ARGS__)
#define WARNING(...) BLINK; YELLOW; printf("%s %s(): ", __FILE__, __func__); ENDC; printf(__VA_ARGS__)
#define ERROR(...) BLINK; UNDERLINE; BOLD; RED; printf("%s %s(): ", __FILE__, __func__); ENDC; printf(__VA_ARGS__)

template<class Block>
class MemoryAllocator
{
public:
    MemoryAllocator(size_t amt) :
        blocks(amt)
    {
    }

    ~MemoryAllocator()
    {
    }

    Block * allocate(Block& b) 
    {
        free_queue();
        Block * ref = blocks.add(b); 

        if (!ref)
        {
            ERROR("Queue is full!\n");
            return NULL;
        }
        return ref;
    }

    void free_queue()
    {
        Block temp;
        LOG("blocks.peek(): %p\n", blocks.peek());
        while ( blocks.size() > 0 && blocks.peek()->is_free() )
        {
            blocks.get(&temp);
        }
    }

private:
    Queue<Block> blocks;
};

#endif
