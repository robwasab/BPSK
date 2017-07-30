#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "QPSK_StdinSource.h"
#include "../Colors/Colors.h"
#include "../TaskScheduler/TaskScheduler.h"

#define MAX(x,y) ((x > y) ? x : y)
#define READ 0
#define WRITE 1

void * QPSK_StdinSource_loop(void * args);

QPSK_StdinSource::QPSK_StdinSource(Memory * memory, 
        TransceiverCallback cb,
        void * trans):
    Module(memory, cb, trans)
{
    int flags;

    pipe(fd);
    flags = fcntl(fd[WRITE], F_GETFL, 0);
    fcntl(fd[WRITE], F_SETFL, flags | O_NONBLOCK);
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
    static char quit[] = "quit\n";
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
            LOG("WRITING TO PIPE!\n");
            write(fd[WRITE], quit, sizeof(quit));
            stop();
            break;

        default:
            break;
    }
}

static
void cmd_set_noise_level(QPSK_StdinSource * self)
{
    double noise_level_db = 0.0;
    LOG("Enter noise level in dB: ");
    scanf("%lf", &noise_level_db);
    RadioMsg msg(CMD_SET_NOISE_LEVEL);
    memcpy(msg.args, &noise_level_db, sizeof(double)); 
    self->transceiver_cb(self->transceiver, &msg);
}

static
void cmd_test(QPSK_StdinSource * self, RadioMsgType msg_type)
{
    RadioMsg msg(msg_type);
    self->transceiver_cb(self->transceiver, &msg);
}

static
void handle_command(QPSK_StdinSource * self)
{
    int k;
    LOG("Enter in the number of the command to execute...\n");
    for (k = 0; k < RADIO_MSG_TYPE_LEN; k++)
    {
        LOG("[%2d]: %s\n", k, RadioMsgString[k]);
    }

    LOG("Command Number: ");
    scanf("%d", &k);

    switch (k)
    {
        case CMD_SET_NOISE_LEVEL:
            cmd_set_noise_level(self);
            break;

        case CMD_TEST_PSK8_SIG_GEN:
            cmd_test(self, (RadioMsgType) k);
            break;

        default:
            WARNING("Not Implemented...\n");
            break;
    }
}

void * QPSK_StdinSource_loop(void * args)
{
    char buffer[256] = {0};
    int fd_stdin;
    int result;
    int n;

    QPSK_StdinSource * self;
    fd_set read_fds;

    self = (QPSK_StdinSource *) args;

    FD_ZERO(&read_fds);
    fd_stdin = fileno(stdin);

    FD_SET(fd_stdin, &read_fds);
    FD_SET(self->fd[READ], &read_fds);

    n = MAX(fd_stdin, self->fd[READ]) + 1;

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        result = select(FD_SETSIZE, &read_fds, NULL, NULL, NULL);
        if (result == -1) 
        {
            perror("select");
        }
        else 
        {
            if (FD_ISSET(fd_stdin, &read_fds)) 
            {
                read(fd_stdin, buffer, sizeof(buffer));
            }
            else if (FD_ISSET(self->fd[READ], & read_fds))
            {
                LOG("Reading from pipe!\n");
                read(self->fd[READ], buffer, sizeof(buffer));
            }
            else
            {
                continue;
            }

            BLUE;
            printf("Read: ");
            ENDC;
            printf("%s", buffer);

            if ( strcmp("cmd\n", buffer) == 0 )
            {
                handle_command(self);
                continue;
            }

            else if ( strcmp("quit\n", buffer) == 0 ) 
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
