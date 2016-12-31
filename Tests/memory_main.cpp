#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "TaskScheduler/TaskScheduler.h"
#include "Transmitter/Transmitter.h"
#include "Memory/Memory.h"

MemoryManager memory;

int run(void * args) {
    static char error_names[][100] = { 
        "no error\n",
        "allocate() returned NULL\n",
        "block->next() returned true when no room left in allocated memory\n",
        "using the iterator, data read back was false\n",
        "block->next() returned false when room still left after reset\n" };

    static size_t size_index = 0;
    static size_t sizes[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    size_t size = sizes[size_index];
    size_index = (size_index + 1) % (sizeof(sizes)/sizeof(sizes[0]));

    Block * block2 = memory.allocate(16);
    Block * block = memory.allocate(size);
    printf("block2: %p\n", block2);
    printf("block : %p\n", block);
    float ** iter = block->get_iterator();
    int error = 0;
    float val = 0.0;

    if (!block) 
    {
        error = 1;
        printf("Failed Test #1\n");
        goto fail;
    }

    do 
    {
        **iter = val;
        val += 1.0;
    } while( block->next() );

    if (block->next()) 
    {
        RED;
        printf("Failed Test #2\n");
        error = 2;
        goto fail;
    }

    block->reset();

    for (float n = 0; n < (float)size; n += 1.0)
    {
        if (n != **iter)
        {
            RED;
            printf("[n: %.3f] != [**iter: %.3f]\n", n, **iter);
            printf("Failed Test #3\n");
            error = 3;
            goto fail;
        }
        /*
        else {
            DIM;
            BLUE;
            printf("[n: %.3f] == [**iter: %.3f]\n", n, **iter);
            ENDC;
        }
        */
        if (!block->next() && n != size - 1.0) {
            RED;
            printf("Failed Test #4\n");
            error = 4;
            goto fail;
        }
    }

    block->free();
    if (block2) {
        block2->free();
    }

    BLUE;
    printf("finished test...\n");
    ENDC;
    return 0;

fail:
    RED;
    printf("%s", error_names[error]);
    ENDC;
    if (block) {
        block->free();
    }
    if (block2) {
        block2->free();
    }
    return -1;
}

int main(int argc, char ** argv)
{
    TaskScheduler scheduler(128);
    
    scheduler.start();

    int num = 0;

    while (1) 
    {
        printf("Enter how many times to load Scheduler. [-1 to exit program]\n");
        scanf("%d", &num);

        if (num < 0 )
            break;

        for (int n = 0; n < num; n++)
        {
            printf("add_event(): %d\n", scheduler.add_event(run, NULL));
        }
    }
    scheduler.stop();

    printf("main returning...\n");
    return 0;
}
