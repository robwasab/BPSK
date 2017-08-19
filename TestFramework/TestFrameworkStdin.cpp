#include <sys/select.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "../Colors/Colors.h"
#include "TestFrameworkStdin.h"

static
void * TestFramework_loop(void * args);

#define READ 0
#define WRITE 1
#define MAX(x,y) ((x > y) ? x : y)


TestFrameworkStdin::TestFrameworkStdin(StateMachine sm):
    TestFramework(sm)
{
    int flags;

    pipe(fd);
    flags = fcntl(fd[WRITE], F_GETFL, 0);
    fcntl(fd[WRITE], F_SETFL, flags | O_NONBLOCK);
}

void TestFrameworkStdin::start(bool block)
{
    pthread_create(&main, NULL, TestFramework_loop, this);
    #ifndef QT_ENABLE
    if (block)
    {
        pthread_join(main, NULL);
    }
    TestFramework::start(block);
    #else
    TestFramework::start(false);
    #endif
}

void TestFrameworkStdin::stop()
{
    static const char quit[] = "652833810";
    write(fd[WRITE], quit, sizeof(quit));

    pthread_join(main, NULL);
    LOG("stdin loop joined...");
    TestFramework::stop();
}

static
void * TestFramework_loop(void * args)
{
    char buffer[256] = {0};
    int fd_stdin;
    int result;
    int n;

    TestFrameworkStdin * self;
    fd_set read_fds;

    self = (TestFrameworkStdin *) args;

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
                //handle_command(self);
                continue;
            }

            else if ( strcmp("quit\n", buffer) == 0 ) 
            {
                LOG("quitting...\n");
                TestEvent te;
                te.type = EVENT_KILL;
                self->notify(te);
                break;
            }

            else if ( strcmp("652833810", buffer) == 0 ) 
            {
                LOG("stopping stdin loop...\n");
                break;
            }

            size_t len = strlen(buffer) + 1; // +1 to hold the size of the message +1 for the string terminator

            //self->process_msg((uint8_t *) buffer, len);
            /* Put new test event with data into test framework */
            TestEvent te;
            te.type = EVENT_USER_INPUT;
            te.data = malloc(len);
            memcpy(te.data, buffer, len);
            te.len = len;
            self->notify(te);
        }

        FD_SET(fd_stdin, &read_fds);
        FD_SET(self->fd[READ], &read_fds);
    }
    return NULL;
}

