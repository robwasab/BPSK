#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "StdinSource.h"
#include "../Colors/Colors.h"
#include "../TaskScheduler/TaskScheduler.h"

void * StdinSource_loop(void * args);

StdinSource::StdinSource(Memory * memory, 
        Module * next,
        TaskScheduler * scheduler):
    Module(memory, next),
    scheduler(scheduler)
{
}

void StdinSource::start(bool block)
{
    pthread_create(&main, NULL, StdinSource_loop, this);
    if (block) {
        pthread_join(main, NULL);
    }
}

void * StdinSource_loop(void * args)
{
    char buffer[256] = {0};
    fd_set read_fds;
    StdinSource * self = (StdinSource *) args;

    FD_ZERO(&read_fds);
    FD_SET(fileno(stdin), &read_fds);
    int n = fileno(stdin) + 1;

    int result;
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        result = select(n, &read_fds, NULL, NULL, NULL);
        if (result == -1) 
        {
            perror("select");
        }
        else 
        {
            if (FD_ISSET(fileno(stdin), &read_fds)) 
            {
                read(fileno(stdin), buffer, sizeof(buffer));
                BLUE;
                printf("Read: ");
                ENDC;
                printf("%s", buffer);
                if ( strcmp("quit\n", buffer) == 0 ) 
                {
                    printf("quitting...\n");
                    break;
                }
                size_t len = strlen(buffer) + 1;
                Block * block = self->memory->allocate(len);

                if (block)
                {
                    float ** iter = block->get_iterator();

                    for (size_t n = 0; n < len; ++n)
                    {
                        **iter = (float) buffer[n];
                        block->next();
                    }
                    self->scheduler->add_module(self, block);
                }
                else 
                {
                    RED;
                    printf("Could not allocate enough space!\n");
                    ENDC;
                }
            }
        }
    }
    return NULL;
}

float mask(float value, int shift)
{
    uint8_t byte = (uint8_t) value;
    byte &= ( ( 1 << ( 1 + shift * 2 ) ) + ( 1 << ( shift * 2 ) ) );
    byte >>= 2*shift;
    return (float) byte;
}

Block * StdinSource::process(Block * msg)
{
    static char errors[][100] = {
        {"No error"},
        {"Memory allocate error"},
        {"Iterator error"} };
    int error = 0;
    size_t line = 0;

    int n = 0;
    uint8_t byte = 0;
    Block * bit = NULL;
    float ** msg_iter = NULL;
    float ** bit_iter = NULL;

    size_t len = msg->get_size() * 8;

    bit = memory->allocate(len);

    if (!bit) {
        line = __LINE__;
        error = 1;
        goto fail;
    }

    msg_iter = msg->get_iterator();
    bit_iter = bit->get_iterator();

    msg->reset();

    do
    {
        byte = (uint8_t) **msg_iter;
        for (n = 0; n < 8; ++n)
        {
            **bit_iter = byte & (1 << n) ? 1.0 : 0.0;
            bit->next();
        }
    } while(msg->next());

    if (msg->next() || bit->next()) {
        line = __LINE__;
        error = 2;
        goto fail;
    }

    msg->free();
    return bit;


fail:
    RED;
    fprintf(stderr, "[%zu]: %s\n", line, errors[error]);
    ENDC;
    msg->free();
    if (bit) {
        bit->free();
    }
    return NULL;
}

