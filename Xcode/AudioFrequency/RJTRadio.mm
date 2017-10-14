//
//  RJTRadio.m
//  AudioFrequency
//
//  Created by Robby Tong on 10/7/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RJTRadio.h"
#include "../Transceivers/TransceiverBPSK.h"
#include "RJTPlotController.h"
#include "../Queue/Queue.h"

typedef enum
{
    RJT_STOP_RADIO,
    RJT_DATA_RECEIVED,
    RJT_CRC_ERROR,
    RJT_PLL_LOST_LOCK,
} RJTRadioTask;

@interface RJTRadio()
// private method declaration
-(void) checkTaskQueue:(NSTimer *) timer;
@end

@implementation RJTRadio
{
    // private instance variables
    TransceiverBPSK * mTransceiver;
    RJTPlotController * mPlotController;
    NSTimer * mTaskCheckTimer;
    Queue<RJTRadioTask> * mTaskQueue;
}

static
bool isChar(char c, const char check[])
{
    int k;
    for (k = 0; check[k] != '\0'; k++)
    {
        if (c == check[k])
        {
            return true;
        }
    }
    return false;
}

void print_msg(const uint8_t msg[], uint8_t size)
{
    char c;
    MAGENTA;
    for (int k = 0; k < size; k++)
    {
        c = (char) msg[k];
        
        if (c == '\n')
        {
            printf("\\n");
        }
        else if ((c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z'))
        {
            printf("%c", c);
        }
        else if (isChar(c, "1234567890 !@#$%^&*()_-+={}[]|:;<>,.?/""'"))
        {
            printf("%c", c);
        }
        else if (c == '\0')
        {
            printf("█");
        }
        else
        {
            printf("?");
        }
    }
    printf("\n");
    ENDC;
}

void RJTRadio_callBack(void * obj, RadioMsg * msg)
{
    RJTRadio * radio = (__bridge RJTRadio *) obj;
    
    switch (msg->type)
    {
        case NOTIFY_USER_REQUEST_QUIT:
            LOG("User has requested to quit\n");
            //te.type = EVENT_KILL;
            //self->notify(te);
            break;
            
        case NOTIFY_DATA_START:
            LOG("Incomming data: %hhu\n", msg->args[0]);
            
            radio->mReceiveDataCount = 0;
            radio->mReceiveDataLen= msg->args[0];
            break;
            
        case NOTIFY_DATA_BODY:
            LOG("Received data body!\n");
            
            if ((radio->mReceiveDataLen - radio->mReceiveDataCount) > RADIO_ARG_SIZE)
            {
            
                memcpy(&radio->mReceiveData[radio->mReceiveDataCount], msg->args, RADIO_ARG_SIZE);
                radio->mReceiveDataCount += RADIO_ARG_SIZE;
            }
            else
            {
                memcpy(&radio->mReceiveData[radio->mReceiveDataCount], msg->args,radio->mReceiveDataLen - radio->mReceiveDataCount);
                
                radio->mReceiveDataCount += radio->mReceiveDataLen - radio->mReceiveDataCount;
                
                LOG("Assembled message: ");
                print_msg(radio->mReceiveData, radio->mReceiveDataLen);
                
                RJTRadioTask task = RJT_DATA_RECEIVED;
                radio->mTaskQueue->add(task);
             }
            break;
        
        case NOTIFY_CRC_CORRUPTED:
        {
            RJTRadioTask task = RJT_CRC_ERROR;
            radio->mTaskQueue->add(task);
        } break;
            
        case NOTIFY_PLL_LOST_LOCK:
        {
            RJTRadioTask task = RJT_PLL_LOST_LOCK;
            radio->mTaskQueue->add(task);
        } break;
            
        default:
            break;
    }
}

// methods
-(id) initWithTxFreq:(double)txFreq rxFreq:(double)rxFreq ifBandWidth:(double)bw ifFreq:(double)ifFreq cyclesPerBit:(int)cycles
{
    self = [super init];
    if (self)
    {
        mPlotController = new RJTPlotController();
        mDataSourcesRef = &mPlotController->mDataSources;
        mTransceiver =
        new TransceiverBPSK(RJTRadio_callBack, (__bridge void *)(self), 44.1E3, txFreq, rxFreq, ifFreq, bw, cycles, mPlotController);
        mTaskCheckTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(checkTaskQueue:) userInfo:nil repeats:YES];
        mTaskQueue = new Queue<RJTRadioTask>(256);
    }
    return self;
}

-(id) initWithTxFreq:(double)txFreq rxFreq:(double)rxFreq
{
    return [self initWithTxFreq:txFreq rxFreq:rxFreq ifBandWidth:4E3 ifFreq:3E3 cyclesPerBit:20];
}

-(void) dealloc
{
    [mTaskCheckTimer invalidate];
    mTaskCheckTimer = nil;
    delete mTransceiver;
    delete mPlotController;
    delete mTaskQueue;
}

-(void) sendWithByteArray:(const uint8_t *)inMsg length:(uint8_t)len
{
    mTransceiver->send(inMsg, len);
}

-(void) sendWithObject:(const NSString *)inMsg
{
    uint8_t len = [inMsg length];
    NSData * data = [inMsg dataUsingEncoding:NSASCIIStringEncoding];
    [self sendWithByteArray:(uint8_t *) data.bytes length:len];
}

-(void) start
{
    mTransceiver->start(false);
}

-(void) stop
{
    RJTRadioTask task = RJT_STOP_RADIO;
    mTaskQueue->add(task);
}

-(void) checkTaskQueue:(NSTimer *) timer
{
    RJTRadioTask task;
    
    if (mTaskQueue->size() > 0)
    {
        mTaskQueue->get(&task);
    
        switch(task)
        {
            case RJT_STOP_RADIO:
                mTransceiver->stop();
                [_mReceiveDelegate finishedStopping];
                break;
            
            case RJT_DATA_RECEIVED:
                if (_mReceiveDelegate != NULL)
                {
                    [_mReceiveDelegate receivedData:mReceiveData dataLen:mReceiveDataLen];
                }
                break;
            
            case RJT_CRC_ERROR:
                if (_mReceiveDelegate != NULL)
                {
                    char msg[] = "CRC ERROR";
                    [_mReceiveDelegate receivedData:(uint8_t *)msg dataLen:sizeof(msg)];
                }
                break;
            
            case RJT_PLL_LOST_LOCK:
                if (_mReceiveDelegate != NULL)
                {
                    char msg[] = "PLL LOST LOCK";
                    [_mReceiveDelegate receivedData:(uint8_t *)msg dataLen:sizeof(msg)];
                }
                break;
        }
    }
}

@end
