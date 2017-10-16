#ifndef __AUDIO_DRIVER_H__
#define __AUDIO_DRIVER_H__

#include "../Memory/Block.h"
#include "../Channel/Channel.h"

/* Initialize Port Audio with a channel. Subsequent calls will simply add the channel pointer
 * to an array that will be called when there is audio data available to rx and tx
 *
 * returns the handle so you can stop the channel later.
 */
int AudioDriver_init(Channel * channel);

/* Can be called multiple times. However, subsequent calls do nothing */ 
void AudioDriver_start();

/*
 * Stop the current channel i.e., no data will be passed to the channel. Once all the channels
 * are stopped, Port Audio gets destroyed.
 */
void AudioDriver_stop(int handle);
#endif
