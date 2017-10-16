//
//  AudioQueueDriver.cpp
//  AudioFrequency
//
//  Created by Robby Tong on 10/14/17.
//  Copyright © 2017 Robby Tong. All rights reserved.
//
//
//  Introductory Blurb:
//  This file implements AudioDriver.h using the OSX framework AudioToolbox

#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioFile.h>

#include "../Module/Module.h"
#include "../Queue/Queue.h"
#include "../switches.h"

#include "AudioDriver.h"
#define MAX_CHANNELS 10
#define BUFFER_SIZE 1024
#define FS 44.1E3

/*****************************************************************
 * MARK: Global Variables + Type Defs
 *****************************************************************/

static int occupied_channels = 0;
static Channel * channels[MAX_CHANNELS] = {NULL};

// Set the number of buffers to use
static const int kNumberBuffers = 3;

static pthread_mutex_t mutex;

struct AQPlayerState
{
    AudioStreamBasicDescription mDataFormat;
    AudioQueueRef mQueue;
    AudioQueueBufferRef mBuffers[kNumberBuffers];
    UInt32 bufferByteSize;
    UInt32 mNumPacketsToRead;
    AudioStreamPacketDescription  *mPacketDescs;
};

struct AQRecorderState
{
    AudioStreamBasicDescription mDataFormat;
    AudioQueueRef mQueue;
    AudioQueueBufferRef mBuffers[kNumberBuffers];
    UInt32 bufferByteSize;
};

static struct AQPlayerState mPlayerState = {0};
static struct AQRecorderState mRecorderState = {0};

// This is to tell the OSX RunLoop to start the audio hardware, since it is not safe to start the audio
// hardware from a different thread
bool mRequestAudioStart = false;
bool mRequestAudioStop = false;

#define LOCK pthread_mutex_lock(&mutex);
#define UNLOCK pthread_mutex_unlock(&mutex);

/*****************************************************************
 * MARK: Functions to start the Audio Queue from the main RunLoop
 *****************************************************************/

void AudioDriverOSX_callPeriodicallyFromMainRunLoop()
{
    static bool started = false;
    static bool stopped = false;

    LOCK
    if (!started && mRequestAudioStart)
    {
        started = true;
        AudioQueueStart(mPlayerState.mQueue, NULL);
        AudioQueueStart(mRecorderState.mQueue, NULL);
    }
    
    if (!stopped && mRequestAudioStop)
    {
        stopped = true;
        LOG("Calling AudioQueueStop on player\n");
        AudioQueueStop(mPlayerState.mQueue, true);
        
        LOG("Calling AudioQueueStop on recorder\n");
        AudioQueueStop(mRecorderState.mQueue, true);
        
        // Need to call in a dispose interface function
        // Since future behavior will allow start and stop to be repeatively called
        LOG("Calling AudioQueueDispose on player\n");
        AudioQueueDispose(mPlayerState.mQueue, true);
        
        LOG("Calling AudioQueueDispose on recorder\n");
        AudioQueueDispose(mRecorderState.mQueue, true);
    }
    UNLOCK
}


/*****************************************************************
 * MARK: Prototypes
 *****************************************************************/

static void AQPlayerState_Initialize(struct AQPlayerState * aq);
static void AQRecorderState_Initialize(struct AQRecorderState * aq);


/*****************************************************************
 * MARK: AudioDriver.h Implementation
 *****************************************************************/

#ifndef SIMULATE
#ifdef AUDIOTOOLBOX

int AudioDriver_init(Channel * channel)
{
    static bool initialized = false;

    if (occupied_channels < 1)
    {
        pthread_mutex_init(&mutex, NULL);
        occupied_channels = 1;
    }
    else if (occupied_channels >= MAX_CHANNELS)
    {
        return -1;
    }
    else
    {
        occupied_channels++;
    }
    channels[occupied_channels - 1] = channel;
    
    LOG("Port Audio handle: %d\n", occupied_channels-1);

    if (!initialized)
    {
        initialized = true;
        AQPlayerState_Initialize(&mPlayerState);
        AQRecorderState_Initialize(&mRecorderState);
    }
    
    return occupied_channels - 1;
}

void AudioDriver_start()
{
    static bool started = false;
    
    if (started)
    {
        LOG("Audio Queue already started\n");
        return;
    }
    else
    {
        started = true;
        LOCK
        mRequestAudioStart = true;
        UNLOCK
    }
}

void AudioDriver_stop(int handle)
{
    if (handle >= occupied_channels || handle < 0)
    {
        LOG("Invalid Handle %d...\n", handle);
        assert(true);
        return;
    }
    
    if (channels[handle] != NULL)
    {
        channels[handle] = NULL;
        occupied_channels--;
    }
    
    if (occupied_channels > 0)
    {
        return;
    }
    else
    {
        LOG("Shutting down Audio Queue!\n");
    }
    
    LOCK
    mRequestAudioStop = true;
    UNLOCK
}

#endif
#endif 


/*****************************************************************
 * MARK: Data Transmission Section
 *****************************************************************/


// Audio Queue callback
static
void HandleOutputBuffer(void * aqData, AudioQueueRef aq, AudioQueueBufferRef buf)
{
    bool stopped = false;
    LOCK
    stopped = mRequestAudioStop;
    UNLOCK
    
    if (stopped) { return; }
    
    size_t totalSamples = buf->mAudioDataBytesCapacity / sizeof(Float32);
    
    Float32 * ref = (Float32 *) buf->mAudioData;
    size_t size = totalSamples * sizeof(Float32);
    
    memset(ref, 0, buf->mAudioDataBytesCapacity);
    for (int k = 0; k < MAX_CHANNELS; ++k)
    {
        if (channels[k] != NULL)
        {
            channels[k]->transmit_callback(ref, totalSamples);
        }
    }
    
    buf->mAudioDataByteSize = (UInt32) size;
    
    AudioQueueEnqueueBuffer(aq, buf, 0, NULL);
}

static
void PrintResultCodes(OSStatus code)
{
    switch (code)
    {
        case kAudioFileUnspecifiedError:
            printf("File unspecified\n");
            break;
        case kAudioFileUnsupportedFileTypeError:
            printf("Unsupported file type\n");
            break;
        case kAudioFileUnsupportedDataFormatError:
            printf("Unsupported data format\n");
            break;
        case kAudioFileUnsupportedPropertyError:
            printf("Unsupported property\n");
            break;
        case kAudioFileBadPropertySizeError:
            printf("Bad property size\n");
            break;
        case kAudioFileNotOptimizedError:
            printf("File not optimized\n");
            break;
        case kAudioFileInvalidChunkError:
            printf("Invalid chunk\n");
            break;
        case kAudioFileDoesNotAllow64BitDataSizeError:
            printf("File does not allow 64 bit data size\n");
            break;
        case kAudioFileInvalidPacketOffsetError:
            printf("Invalid packet offset\n");
            break;
        case kAudioFileInvalidFileError:
            printf("Invalid file\n");
            break;
        case kAudioFileOperationNotSupportedError:
            printf("Operation not supported\n");
            break;
        case kAudioFileNotOpenError:
            printf("File not open\n");
            break;
        case kAudioFileEndOfFileError:
            printf("End of file\n");
            break;
        case kAudioFilePositionError:
            printf("Position\n");
            break;
        case kAudio_FileNotFoundError:
            printf("File not found\n");
            break;
        default:
            return;
    }
    assert(false);
}

static
void PrintCFString(CFStringRef cf_string)
{
    const char * string = CFStringGetCStringPtr(cf_string, kCFStringEncodingMacRoman);
    
    printf("%s\n", string);
}


static
void PrintBasicDescription(AudioStreamBasicDescription * mDataFormat)
{
    printf("Bits per channel  : %d\n", mDataFormat->mBitsPerChannel);
    printf("Sample rate       : %lf.3\n", mDataFormat->mSampleRate);
    printf("Channels per frame: %d\n", mDataFormat->mChannelsPerFrame);
    printf("Frames per packet : %d\n", mDataFormat->mFramesPerPacket);
    printf("Bytes per packet  : %d\n", mDataFormat->mBytesPerPacket);
}

static
void CheckError(OSStatus error, const char *operation)
{
    if (error == noErr) return;
    
    char str[20];
    // see if it appears to be a 4-char-code
    *(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
    if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
        str[0] = str[5] = '\'';
        str[6] = '\0';
    } else
        // no, format it as an integer
        sprintf(str, "%d", (int)error);
    
    fprintf(stderr, "Error: %s (%s)\n", operation, str);
    
    exit(1);
}

static
void AQPlayerState_InitOutputQueue(struct AQPlayerState * aq)
{
    OSStatus code =
    AudioQueueNewOutput(&aq->mDataFormat, HandleOutputBuffer, aq, CFRunLoopGetMain(), kCFRunLoopCommonModes, 0, &aq->mQueue);
    
    CheckError(code, "AudioQueueNewOutput ");
}


static
void AQPlayerState_InitSizes(struct AQPlayerState * aq)
{
    aq->mNumPacketsToRead = BUFFER_SIZE;
    aq->bufferByteSize = sizeof(Float32) * aq->mNumPacketsToRead;
}

static
void AQPlayerState_AllocateBuffersAndPrime(struct AQPlayerState * aq)
{
    int k;
    
    for (k = 0; k < kNumberBuffers; k++)
    {
        AudioQueueAllocateBuffer(aq->mQueue, aq->bufferByteSize, &aq->mBuffers[k]);
        HandleOutputBuffer(aq, aq->mQueue, aq->mBuffers[k]);
    }
}

static
void AQPlayerState_Configure(struct AQPlayerState * aq)
{
    AudioQueueSetParameter(aq->mQueue, kAudioQueueParam_Volume, 1.0);
    AudioQueueSetParameter(aq->mQueue, kAudioQueueParam_PlayRate, 1.0);
    AudioQueueSetParameter(aq->mQueue, kAudioQueueParam_Pitch, 0);
    AudioQueueSetParameter(aq->mQueue, kAudioQueueParam_VolumeRampTime, 1.0);
    AudioQueueSetParameter(aq->mQueue, kAudioQueueParam_Pan, 0);
}

static
void AQPlayerState_InitBasicDescription(struct AQPlayerState * aq)
{
    aq->mDataFormat = {0};
    aq->mDataFormat.mBitsPerChannel = 8 * sizeof(Float32);
    aq->mDataFormat.mBytesPerFrame = sizeof(Float32);
    aq->mDataFormat.mBytesPerPacket = aq->mDataFormat.mBytesPerFrame;
    aq->mDataFormat.mChannelsPerFrame = 1;
    aq->mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsFloat;
    aq->mDataFormat.mFormatID = kAudioFormatLinearPCM;
    aq->mDataFormat.mFramesPerPacket = 1;
    aq->mDataFormat.mSampleRate = FS;
}


static
void AQPlayerState_Initialize(struct AQPlayerState * aq)
{
    // Init basic description property
    AQPlayerState_InitBasicDescription(aq);
    
    // Init audio queue
    AQPlayerState_InitOutputQueue(aq);
    
    // Init buffer & packet size numbers
    AQPlayerState_InitSizes(aq);
    
    // Allocate audio queue buffers and prime them
    AQPlayerState_AllocateBuffersAndPrime(aq);
    
    // Set the gain
    AQPlayerState_Configure(aq);
}

/*****************************************************************
 * MARK: Data Reception Section
 *****************************************************************/

static
void HandleInputBuffer(
                       void * userData,
                       AudioQueueRef inAudioQueue,
                       AudioQueueBufferRef inBuffer,
                       const AudioTimeStamp * inStartTime,
                       UInt32 inNumPackets,
                       const AudioStreamPacketDescription * inPacketDesc)
{
    struct AQRecorderState * aq = (struct AQRecorderState *) userData;
    
    bool stopped = false;
    LOCK
    stopped = mRequestAudioStop;
    UNLOCK
    
    if (stopped) { return; }
    
    size_t totalSamples = inBuffer->mAudioDataByteSize / sizeof(Float32);
    
    Float32 * ref = (Float32 *) inBuffer->mAudioData;
    
    int k;
    
    for (k = 0; k < MAX_CHANNELS; ++k)
    {
        if (channels[k] != NULL)
        {
            channels[k]->receive_callback(ref, totalSamples);
        }
    }
    
    // Enqueue the buffer
    // The last two parameters inNumPacketDesc, which equals zero, and
    // inPacketDescs, which equals NULL, are unused when recording.
    AudioQueueEnqueueBuffer(inAudioQueue, inBuffer, 0, NULL);
}


static
void PrintFormatID(AudioFormatID id)
{
    UInt8 * data = (UInt8 *) &id;
    printf("AudioFormatID: %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3]);
    printf("AudioFormatID: %c   %c   %c   %c\n", data[0], data[1], data[2], data[3]);
}

static
void PrintResultCode(OSStatus code)
{
    UInt8 * data = (UInt8 *) &code;
    printf("%02X %02X %02X %02X\n", data[0], data[1], data[2], data[3]);
    printf("%c  %c  %c  %c\n", data[3], data[2], data[1], data[0]);
}


static
void AQRecorderState_InitAsLinearPCM(struct AQRecorderState * aq)
{
    aq->mDataFormat = {0};
    aq->mDataFormat.mBitsPerChannel = 8 * sizeof(Float32);
    aq->mDataFormat.mBytesPerFrame = sizeof(Float32);
    aq->mDataFormat.mBytesPerPacket = aq->mDataFormat.mBytesPerFrame;
    aq->mDataFormat.mChannelsPerFrame = 1;
    aq->mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsFloat;
    aq->mDataFormat.mFormatID = kAudioFormatLinearPCM;
    aq->mDataFormat.mFramesPerPacket = 1;
    aq->mDataFormat.mSampleRate = FS;
}


static
bool AQRecorderState_InitAudioQueue(struct AQRecorderState * aq)
{
    OSStatus code;

    code = AudioQueueNewInput(&aq->mDataFormat, HandleInputBuffer, aq, CFRunLoopGetMain(), kCFRunLoopCommonModes, 0, &aq->mQueue);
    
    if (code != noErr)
    {
        printf("AudioQueueNewInput Error %d\n", (int) code);
        return false;
    }
    
    // Copy the newly created audio queue's description into your description since
    // it might have more information
    UInt32 dataFormatSize = sizeof(AudioStreamBasicDescription);
    AudioQueueGetProperty(aq->mQueue, kAudioQueueProperty_StreamDescription, &aq->mDataFormat, &dataFormatSize);
    return true;
}

static
void AQRecorderState_InitSizes(struct AQRecorderState * aq)
{
    aq->bufferByteSize = sizeof(Float32) * BUFFER_SIZE;
}

static
void AQRecorderState_AllocateBuffers(struct AQRecorderState * aq)
{
    for (int k = 0; k < kNumberBuffers; k++)
    {
        AudioQueueAllocateBuffer(aq->mQueue, aq->bufferByteSize, &aq->mBuffers[k]);
        
        // Last two parameters are unused when enqueuing a buffer for recording
        AudioQueueEnqueueBuffer(aq->mQueue, aq->mBuffers[k], 0, NULL);
    }
}

static
void AQRecorderState_Initialize(struct AQRecorderState * aq)
{
    // Init basic description
    AQRecorderState_InitAsLinearPCM(aq);
    
    // Init audio queue
    AQRecorderState_InitAudioQueue(aq);
    
    // Init buffer sizes
    AQRecorderState_InitSizes(aq);
    
    // Init audio queue buffers
    AQRecorderState_AllocateBuffers(aq);
}
