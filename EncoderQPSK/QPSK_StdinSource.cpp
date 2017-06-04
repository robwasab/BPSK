#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "QPSK_StdinSource.h"
#include "../Colors/Colors.h"
#include "../TaskScheduler/TaskScheduler.h"

void * QPSK_StdinSource_loop(void * args);

QPSK_StdinSource::QPSK_StdinSource(Memory * memory, 
        TransceiverCallback cb,
        void * trans):
    Module(memory, cb, trans)
{
}

void QPSK_StdinSource::start(bool block)
{
    pthread_create(&main, NULL, QPSK_StdinSource_loop, this);
    if (block) {
        pthread_join(main, NULL);
    }
}

void QPSK_StdinSource::stop()
{
    pthread_join(main, NULL);
}

void QPSK_StdinSource::dispatch(RadioMsg * msg)
{
    RadioData * data;
    Block * block;

    data = (RadioData *) msg;

    switch(msg->type)
    {
        case PROCESS_DATA:
            block = data->get_block();

            block = process(block);

            if (block != NULL)
            {
                handoff(block, data->get_tid());
            }
            break;

        case CMD_START:
            start(false);
            break;
        case CMD_STOP:
            stop();
            break;
        case NOTIFY_USER_REQUEST_QUIT:
        case CMD_RESET_ALL:
        case CMD_RESET_TRANSMITTER:
        case CMD_RESET_RECEIVER:
        case CMD_SET_TRANSMIT_CHANNEL:
        case CMD_SET_RECEIVE_CHANNEL:
        case NOTIFY_PLL_LOST_LOCK:
        case NOTIFY_PACKET_HEADER_DETECTED:
        case NOTIFY_RECEIVER_RESET_CONDITION_DETECTED:
        case NOTIFY_DATA_RECEIVED:
            break;
    }
}

void * QPSK_StdinSource_loop(void * args)
{
    char buffer[256] = {0};
    fd_set read_fds;
    QPSK_StdinSource * self = (QPSK_StdinSource *) args;

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
                    LOG("quitting...\n");
                    RadioMsg msg(NOTIFY_USER_REQUEST_QUIT);
                    self->transceiver_cb(self->transceiver, &msg);
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
                    block = self->process(block);
                    self->handoff(block, 0);
                }
                else 
                {
                    ERROR("Could not allocate enough space!\n");
                }
            }
        }
    }
    return NULL;
}

Block * QPSK_StdinSource::process(Block * msg)
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

    size_t len = msg->get_size() * 4;

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
        for (n = 0; n < 4; ++n)
        {
            **bit_iter = (byte & (0x03 << (n * 2))) >> (n * 2) ;
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

static
const char _STDIN_SOURCE_NAME_[] = "QPSK_StdinSource";

const char * QPSK_StdinSource::name() {
    return _STDIN_SOURCE_NAME_; 
}
