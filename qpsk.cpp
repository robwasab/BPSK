#include "TestFramework/TestFramework.h"
#include "Tests/simple.h"

int _argc = 0;
char ** _argv = NULL;

int main(int argc, char ** argv)
{
    /* This is to make the QT library happy,
       and to not obscure code by passing these guys everywhere */
    _argc = argc;
    _argv = argv;

    TestFramework testbench(PSK4, simple); 
    testbench.start(false);
    return 0;
}
