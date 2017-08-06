#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "StdinSource.h"
#include "../Colors/Colors.h"
#include "../TaskScheduler/TaskScheduler.h"

void * StdinSource_loop(void * args);

#define READ 0
#define WRITE 1
#define MAX(x,y) ((x > y) ? x : y)

StdinSource::StdinSource(Memory * memory, 
        TransceiverCallback cb,
        void * trans,
        const uint16_t crc_table[]):
    ByteInputInterface(memory, cb, trans, crc_table)
{
    int flags;

    pipe(fd);
    flags = fcntl(fd[WRITE], F_GETFL, 0);
    fcntl(fd[WRITE], F_SETFL, flags | O_NONBLOCK);
}

void StdinSource::start(bool block)
{
    pthread_create(&main, NULL, StdinSource_loop, this);
    if (block) {
        pthread_join(main, NULL);
    }
}

void StdinSource::stop()
{
    pthread_join(main, NULL);
}

void StdinSource::dispatch(RadioMsg * msg)
{
    static char buf[256] = {0};
    static const char quit[] = "652833810";
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
        {
            LOG("starting %s thread...\n", name());
            start(false);

            LOG("adding dummy data...\n");
            uint8_t dummy[] = {0};

            process_msg(dummy, 1);
            break;
        }

        case CMD_STOP:
            LOG("stopping %s thread...\n", name());

            LOG("Writing secret random number ""652833810"" to pipe...\n");
            memset(buf, 0, sizeof(buf));
            memcpy(buf, quit, sizeof(quit));
            write(fd[WRITE], buf, sizeof(buf));
            stop();
            break;

        default:
            break;
    }
}

static
void cmd_set_noise_level(StdinSource * self)
{
    double noise_level_db = 0.0;
    LOG("Enter noise level in dB: ");
    scanf("%lf", &noise_level_db);
    RadioMsg msg(CMD_SET_NOISE_LEVEL);
    memcpy(msg.args, &noise_level_db, sizeof(double)); 
    self->transceiver_cb(self->transceiver, &msg);
}

static
void cmd_test(StdinSource * self, RadioMsgType msg_type)
{
    RadioMsg msg(msg_type);
    self->transceiver_cb(self->transceiver, &msg);
}

static
void handle_command(StdinSource * self)
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

void * StdinSource_loop(void * args)
{
    char buffer[256] = {0};
    int fd_stdin;
    int result;
    int n;

    StdinSource * self;
    fd_set read_fds;

    self = (StdinSource *) args;

    FD_ZERO(&read_fds);
    fd_stdin = fileno(stdin);

    FD_SET(fd_stdin, &read_fds);
    FD_SET(self->fd[READ], &read_fds);

    n = MAX(fd_stdin, self->fd[READ]) + 1;

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
            if (FD_ISSET(fd_stdin, &read_fds)) 
            {
                read(fd_stdin, buffer, sizeof(buffer));
            }
            else if (FD_ISSET(self->fd[READ], &read_fds))
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

            else if ( strcmp("652833810", buffer) == 0 ) 
            {
                LOG("qt quit...\n");
                break;
            }

            size_t len = strlen(buffer) + 1; // +1 to hold the size of the message +1 for the string terminator

            self->process_msg((uint8_t *) buffer, len);

        }
        FD_SET(fd_stdin, &read_fds);
        FD_SET(self->fd[READ], &read_fds);
    }
    return NULL;
}

