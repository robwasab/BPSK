#ifndef __TEST_FRAMEWORK_STDIN_H__
#define __TEST_FRAMEWORK_STDIN_H__

#include "TestFramework.h"
#include <pthread.h>

class TestFrameworkStdin: public TestFramework
{
public:
    TestFrameworkStdin(StateMachine sm);

    /* Overrride, stop TestFramework + join the stdin select loop */
    void stop();

    /* Override, start the stdin select loop + TestFramework */
    void start(bool block);

    /* Pipe file descriptor, so that an external thread can stop the stdin loop */
    int fd[2];
private:
    pthread_t main;
};


#endif
