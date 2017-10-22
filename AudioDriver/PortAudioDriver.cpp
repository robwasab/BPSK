
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "../Module/Module.h"
#include "../Queue/Queue.h"
#include "../switches.h"

#include "AudioDriver.h"

#if defined(PORTAUDIO) && !defined(SIMULATE)

#define MAX_CHANNELS 10

#include <portaudio.h>

static PaStream * stream = NULL;
static int PortAudio_callback( const void *input, void *output, unsigned long frames, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void * arg );

static int occupied_channels = 0;

static Channel * channels[MAX_CHANNELS] = {NULL};

static pthread_mutex_t mutex;
static bool quit = false;

/*
 * Initialize Port Audio with a channel.
 * returns the handle number.
 * returns -1 if no more available channles.
 */
int AudioDriver_init(Channel * channel)
{
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

    return occupied_channels - 1;
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

void AudioDriver_start()
{
    static bool started = false;

    if (started)
    {
        LOG("Port Audio already started\n");
        return;
    }
    else
    {
        LOG("Starting Port Audio!\n");
        started = true;
    }
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

    LOG("Opening stream!\n");

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
}

void AudioDriver_stop(int handle)
{
    if (handle >= occupied_channels || handle < 0)
    {
        LOG("Invalid Handle %d...\n", handle);
        return;
    }

    pthread_mutex_lock(&mutex);

    if (channels[handle] != NULL)
    {
        channels[handle] = NULL;
        occupied_channels--;
    }

    pthread_mutex_unlock(&mutex);

    if (occupied_channels > 0)
    {
        return;
    }
    else
    {
        LOG("Shutting down Port Audio!\n");
    }
        
    pthread_mutex_lock(&mutex);
    quit = true;
    pthread_mutex_unlock(&mutex);

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
}

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
    float * tx_buffer = (float *) output;
    const float * rx_buffer = (const float *) input;
    size_t k;
    
    /* Parse Error Messages TODO */
    /*
    PaTime input_time = timeInfo->inputBufferAdcTime;
    PaTime invok_time = timeInfo->currentTime;
    PaTime outpu_time = timeInfo->outputBufferDacTime;

    printf("input  time: %lf\n", input_time);
    printf("invoke time: %lf\n", invok_time);
    printf("output time: %lf\n", outpu_time);
    */

    if (statusFlags & paInputUnderflow) 
    {
        LOG("Input Underflow, not enough input data.\n");
    }
    else if (statusFlags & paInputOverflow) 
    {
        LOG("Input Overflow, callback taking too much time.\n");
    }
    else if (statusFlags & paOutputUnderflowed) 
    {
        LOG("Output Underflow, callback taking too much time.\n");
    }
    else if (statusFlags & paOutputOverflow) 
    {
        LOG("Output Overflow, too much output data.\n");
    }
    else if (statusFlags & paPrimingOutput) 
    {
        LOG("Priming the stream.\n");
    }

    pthread_mutex_lock(&mutex);

    memset(tx_buffer, 0, sizeof(float) * frames);

    double scale = 0.0;

    for (k = 0; k < MAX_CHANNELS; k++)
    {
        if (channels[k] != NULL)
        {
            channels[k]->transmit_callback(tx_buffer, frames);
            channels[k]->receive_callback(rx_buffer, frames);
            scale += 1.0;
        }
    }

    for (k = 0; k < frames; k++)
    {
        tx_buffer[k] /= scale;
    }

    if (quit) 
    {
        pthread_mutex_unlock(&mutex);
        return paComplete;
    }
    pthread_mutex_unlock(&mutex);
	
	#ifdef PRINT_CPU_LOAD
    double load;
    if (stream)
    {
        load = Pa_GetStreamCpuLoad(stream);
        LOG("CPU load: %.3lf\r", 100.0 * load);
    }
	#endif
	
    return paContinue;
}

#endif
