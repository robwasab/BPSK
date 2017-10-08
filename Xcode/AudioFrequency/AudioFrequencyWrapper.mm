//
//  AudioFrequencyWrapper.m
//  AudioFrequency
//
//  Created by Robby Tong on 9/29/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AudioFrequencyWrapper.h"

#include "TestFramework/TestFramework.h"
#include "TestFramework/TestFrameworkStdin.h"
#include "Tests/simple.h"

int _argc = 0;
char ** _argv = NULL;
pthread_mutex_t log_mutex;

@interface AudioFrequencyWrapper()
// Private properties
// Private methods
@end

@implementation AudioFrequencyWrapper
{
    // Private instance variables
    TestFrameworkStdin * testbench;
    char * argv;
    int argc;
}

// Method implementation
-(id) initWithArgc:(int)passedArgc withArgv:(char **)passedArgv shouldBlockOnStart:(BOOL)blockOnStart
{
    self = [super init];
    if (self)
    {
        _block = blockOnStart;

        /* This is to make the QT library happy,
         and to not obscure code by passing these guys everywhere */
        _argc = passedArgc;
        _argv = passedArgv;
        
        pthread_mutex_init(&log_mutex, NULL);
        testbench = new TestFrameworkStdin(simple);
    }
    else
    {
        testbench = NULL;
    }
    return self;
}

-(id) initWithBlockOnStart:(BOOL)blockOnStart
{
    
    char dummy_argv[] = "main";
    
    argv = new char[sizeof(dummy_argv)];
    memcpy(argv, dummy_argv, sizeof(dummy_argv));
    argc = 1;
    
    return [self initWithArgc:argc
            withArgv:&argv shouldBlockOnStart:blockOnStart];
}

-(void) dealloc
{
    delete [] argv;
    delete testbench;
}

-(void) mainLoop
{
    if (!testbench) return;
    testbench->main_loop();
}

-(void) start
{
    if (!testbench) return;
    testbench->start(_block);
}

-(void) stop
{
    if (!testbench) return;
    testbench->stop();
}
@end

