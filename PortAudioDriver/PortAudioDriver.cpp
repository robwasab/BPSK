#include <portaudio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "../Module/Module.h"
#include "../Queue/Queue.h"
#include "../switches.h"

#include "PortAudioDriver.h"

static void * _arg;
static PortAudioOnReceive receive_cb = NULL;
static PaStream * stream = NULL;

static Queue<Block *> source(64);
static pthread_mutex_t mutex;
static bool quit = false;
#ifdef SIMULATE
static pthread_t thread;
static void * PortAudioSimulator_loop(void * arg);
#endif

static
int PortAudio_callback( const void *input, void *output, unsigned long frames, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void * arg );

void PortAudio_init(void * arg, PortAudioOnReceive cb)
{
    assert(cb != NULL);
    _arg = arg;
    receive_cb = cb;
    pthread_mutex_init(&mutex, NULL);
}

void PortAudio_add(Block * block)
{
    source.add(block);
}

static
PaError print_device(PaDeviceIndex index) 
{
    const PaDeviceInfo * dev_info = Pa_GetDeviceInfo(index);
    const PaHostApiInfo * api_info = Pa_GetHostApiInfo(dev_info->hostApi);
    LOG("[%d] %s\n", (int) index, dev_info->name);
    LOG("    Max input channels      : %d\n" , dev_info->maxInputChannels);
    LOG("    Max outpu channels      : %d\n" , dev_info->maxOutputChannels);
    if (dev_info->maxInputChannels != 0 ) 
    {
        LOG("    Input latency low  [Hz] : %.3lf\n", 1.0/(dev_info->defaultLowInputLatency));
        LOG("    Input latency high [Hz] : %.3lf\n", 1.0/(dev_info->defaultHighInputLatency));
        LOG("    Est. num samples low lat: %.3lf\n", dev_info->defaultLowInputLatency * dev_info->defaultSampleRate);
        LOG("    Est. num samples hig lat: %.3lf\n", dev_info->defaultHighInputLatency * dev_info->defaultSampleRate);
    }
    if (dev_info->maxOutputChannels != 0) 
    {
        LOG("    Outpu latency low  [Hz] : %.3lf\n", 1.0/(dev_info->defaultLowOutputLatency));
        LOG("    Outpu latency high [Hz] : %.3lf\n", 1.0/(dev_info->defaultHighOutputLatency));
        LOG("    Est. num samples low lat: %.3lf\n", dev_info->defaultLowOutputLatency * dev_info->defaultSampleRate);
        LOG("    Est. num samples hig lat: %.3lf\n", dev_info->defaultHighOutputLatency * dev_info->defaultSampleRate);
    }
    LOG("    Sampling rate      [Hz] : %.3lf\n", dev_info->defaultSampleRate);
    LOG("    API name                : %s\n" , api_info->name);
    LOG("    API num devices         : %d\n" , api_info->deviceCount);
    LOG("    API default input index : %d\n" , (int) (api_info->defaultInputDevice));
    LOG("    API default outpu index : %d\n" , (int) (api_info->defaultOutputDevice));

    return paNoError;
}

void PortAudio_start()
{
#ifdef SIMULATE
    pthread_create(&thread, NULL, PortAudioSimulator_loop, NULL);
    return;
#else
    PaStreamParameters input_params;
    PaStreamParameters outpu_params;
    double input_fs;
    double outpu_fs;
    PaError error;

    error = Pa_Initialize();

    LOG("Printing input device...\n");

    print_device(Pa_GetDefaultInputDevice());
    LOG("Printing output device...\n");

    print_device(Pa_GetDefaultOutputDevice());

    if (error != paNoError) 
    {
        goto fail;
    }

    // Setup input device
    input_params.device = Pa_GetDefaultInputDevice();
    if (input_params.device == paNoDevice) 
    {
        error = paNoDevice;
        goto fail;
    }

    // Setup output device
    outpu_params.device = Pa_GetDefaultOutputDevice();
    if (outpu_params.device == paNoDevice) 
    {
        error = paNoDevice;
        goto fail;
    }

    input_params.channelCount = 1;
    input_params.sampleFormat = paFloat32;
    input_params.suggestedLatency = Pa_GetDeviceInfo(input_params.device)->defaultLowInputLatency;
    input_params.hostApiSpecificStreamInfo = NULL;
    input_fs = Pa_GetDeviceInfo(input_params.device)->defaultSampleRate;

    outpu_params.channelCount = 1;
    outpu_params.sampleFormat = paFloat32;
    outpu_params.suggestedLatency = Pa_GetDeviceInfo(outpu_params.device)->defaultLowOutputLatency;
    outpu_params.hostApiSpecificStreamInfo = NULL;
    outpu_fs = Pa_GetDeviceInfo(outpu_params.device)->defaultSampleRate;

    if (input_fs != outpu_fs) 
    {
        WARNING("default input fs %.3lf != default output fs %.3lf\n", input_fs, outpu_fs);
    }

    if (input_fs != 44.1E3) 
    {
        WARNING("input fs %.3lf != 44.1kHz\n", input_fs);
        WARNING("outpu fs %.3lf != 44.1kHz\n", outpu_fs);
    }

    error = Pa_OpenStream(
            &stream,
            &input_params,
            &outpu_params,
            44.1E3,
            1024,
            0,
            PortAudio_callback,
            NULL);

    if (error != paNoError) 
    {
        goto fail;
    }

    error = Pa_StartStream(stream);

    if (error != paNoError) {
        goto fail;
    }

    return;
fail:
    ERROR("%s\n", Pa_GetErrorText(error));
    return;
#endif
}

void PortAudio_stop()
{
    pthread_mutex_lock(&mutex);
    quit = true;
    pthread_mutex_unlock(&mutex);
#ifdef SIMULATE
    pthread_join(thread, NULL);
#else
    PaError err;

    if (Pa_IsStreamActive(stream) == 1)
    {
        err = Pa_StopStream(stream);
        
        if (err != paNoError) 
        {
            goto fail;
        }

        while(1) 
        {
            err = Pa_IsStreamStopped(stream); 
            Pa_Sleep(100);
            if (err < 0) 
            {
                goto fail;
            }
            else if (err == 1) 
            {
                break;
            }
            Pa_Sleep(100);
        }
    }

    err = Pa_CloseStream(stream);

    if (err != paNoError) {
        ERROR("%s\n", Pa_GetErrorText(err));
    }

    Pa_Terminate();

    return;
fail:
    ERROR("%s\n", Pa_GetErrorText(err));
    return;
#endif
}

#ifdef SIMULATE
void * PortAudioSimulator_loop(void * arg)
{
    size_t size = 1024;
    unsigned long sleep_time = (unsigned long) round(size/44.1E3*1E6);
    //sleep_time = 1E5;

    //int count = 0;
    float * tx_buffer = (float *) malloc(sizeof(float) * size);
    float * rx_buffer = tx_buffer;
    unsigned long frames = size;

    PaStreamCallbackTimeInfo time_info;
    PaStreamCallbackFlags status_flags = 0;
    bool quit = false;
    int result;

    while(!quit)
    {
        time_info.inputBufferAdcTime = 0.0;
        time_info.currentTime = 0.0;
        time_info.outputBufferDacTime = 0.0;
        usleep(sleep_time);
        result = PortAudio_callback(
                rx_buffer, tx_buffer, frames, &time_info, status_flags, arg);

        switch (result)
        {
            case paContinue:
                continue;

            case paAbort:
                ERROR("PortAudio returned Error!\n");
                break;

            case paComplete:
                LOG("PortAudio completed!\n");
                quit = true;
                break;
        }
    }
    LOG("PortAudio quitting...\n");
    return NULL;
}
#endif

static
int PortAudio_callback(
    const void *input, 
    void *output,
    unsigned long frames,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void * arg )
{
    /* assume mono playback */
    static Block * tx_block = NULL;
    static float scale = 1.0;
    float * tx_buffer = (float *) output;
    const float * rx_buffer = (const float *) input;
    float ** tx_iter;
    size_t start;

    /* Parse Error Messages TODO */
    /*
    PaTime input_time = timeInfo->inputBufferAdcTime;
    PaTime invok_time = timeInfo->currentTime;
    PaTime outpu_time = timeInfo->outputBufferDacTime;

    printf("input  time: %lf\n", input_time);
    printf("invoke time: %lf\n", invok_time);
    printf("output time: %lf\n", outpu_time);
    */

    if (statusFlags & paInputUnderflow) {
        LOG("Input Underflow, not enough input data.\n");
    }
    else if (statusFlags & paInputOverflow) {
        LOG("Input Overflow, callback taking too much time.\n");
    }
    else if (statusFlags & paOutputUnderflowed) {
        LOG("Output Underflow, callback taking too much time.\n");
    }
    else if (statusFlags & paOutputOverflow) {
        LOG("Output Overflow, too much output data.\n");
    }
    else if (statusFlags & paPrimingOutput) {
        LOG("Priming the stream.\n");
    }

    start = 0;
    if (source.size() > 0)
    {
        Block * block;
        source.get(&block);
        if (tx_block) 
        {
            tx_iter = tx_block->get_iterator();
            tx_buffer[0] = **tx_iter * scale;
            start = 1;
            tx_block->free();
        }
        tx_block = block;
        tx_block->reset();
        LOG("tx_block size: %zu\n", tx_block->get_size());
    }

    if (tx_block) 
    {
        tx_iter = tx_block->get_iterator();

        for (size_t n = start; n < frames; ++n)
        {
            tx_buffer[n] = **tx_iter * scale; 
            tx_block->next();
        }
        receive_cb(_arg, rx_buffer, frames);
    }
    else 
    {
        memset(tx_buffer, 0, sizeof(float) * frames);
    }

    pthread_mutex_lock(&mutex);
    if (quit) 
    {
        pthread_mutex_unlock(&mutex);
        return paComplete;
    }
    pthread_mutex_unlock(&mutex);

    if (stream) {
        double load = Pa_GetStreamCpuLoad(stream);
        LOG("CPU load: %.3lf\r", 100.0 * load);
    }
    return paContinue;
}
