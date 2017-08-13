#include "TestFramework/TestFramework.h"
#include "TestFramework/TestFrameworkStdin.h"
#include "Tests/simple.h"

int _argc = 0;
char ** _argv = NULL;
pthread_mutex_t log_mutex;

int main(int argc, char ** argv)
{
    /* This is to make the QT library happy,
       and to not obscure code by passing these guys everywhere */
    _argc = argc;
    _argv = argv;

    pthread_mutex_init(&log_mutex, NULL);

    TestFrameworkStdin testbench(simple); 
    testbench.main_loop();
    return 0;
}
