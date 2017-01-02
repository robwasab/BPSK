#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "Memory.h"
#include "Block.h"
#include "../Queue/Queue.h"
#include "../Colors/Colors.h"

Memory::Memory():
    head(0),
    tail(0),
    size(0),
    blocks(256)
{
    memory = new float[SIZE];
    pthread_mutex_init(&mutex, NULL);
}

Memory::~Memory()
{
    printf("Memory is freeing %d elements\n", SIZE);
    delete [] memory;
}

Block * Memory::allocate(size_t amt)
{
    lock();
    free_queue();
    if (size + amt <= SIZE) 
    {
        MemoryBlock b(tail, amt, memory, SIZE);

        Block * ref = blocks.add(b);

        if (!ref)
        {
            RED;
            printf("\'blocks\' Queue is full. Recompile with bigger size!\n");
            ENDC;
            unlock();
            return NULL;
        }

        tail = (tail + amt) % SIZE;
        size =  size + amt;

        printf("Allocat Block: ");
        MAGENTA;
        printf("[++%4zu/%4zu]\n", amt, size);
        ENDC;
        //ref->print();
        //ENDC;
        unlock();
        return ref;
    }
    else 
    {
        RED;
        printf("Not enough memory!\n");
        ENDC;
        unlock();
        return NULL;
    }
}

void Memory::free_queue()
{
    size_t unused;
    MemoryBlock temp;
    
    //printf("%s(): blocks.size() = %zu\n", __func__, blocks.size());

    while ( blocks.size() > 0 && blocks.peek()->is_free() ) 
    {
        printf("Freeing Block: ");
        BLUE;
        printf("[--%4zu/%4zu]\n", blocks.peek()->get_size(), size);
        ENDC;
        //blocks.peek()->print();
        unused = blocks.peek()->get_size();
        head = (head + unused) % SIZE;
        size =  size - unused;
        blocks.get(&temp);
    }
}

void Memory::lock()
{
    pthread_mutex_lock(&mutex);
}

void Memory::unlock()
{
    pthread_mutex_unlock(&mutex);
}
