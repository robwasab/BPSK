#include "TestFramework/TestFramework.h"
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

    double ftx = 19E3;
    double frx = 19E3;
    double fif = 3E3;
    double bw = 1E3;
    int cycles_per_bit = 10;
    TestFramework testbench(simple, ftx, frx, fif, bw, cycles_per_bit); 
    testbench.main_loop();
    return 0;
}
