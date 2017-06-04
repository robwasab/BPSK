#ifndef __PORT_AUDIO_DRIVER_H__
#define __PORT_AUDIO_DRIVER_H__

#include "../Memory/Block.h"

typedef void (*PortAudioOnReceive)(void * obj, const float rx_buffer[], unsigned long frames);

void PortAudio_add(Block * block);
void PortAudio_init(void * arg, PortAudioOnReceive cb);
void PortAudio_start();
void PortAudio_stop();

#endif
