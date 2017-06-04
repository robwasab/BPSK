/* 
https://www3.nd.edu/~dthain/courses/cse20211/fall2013/wavfile/wavfile.c
 
There are endian problems, but because this is only intended to run on
my machine, and I know my machine is little endian, everything should 
be fine.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "WavSink.h"

struct wav_header {
    char riff_tag[4];
    uint32_t riff_length;
    char wave_tag[4];
    char frmt_tag[4];
    uint32_t frmt_length;
    uint16_t audi_format;
    uint16_t num_channel;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_tag[4];
    uint32_t data_length;
};

FILE * wavefile_open(const char filename[])
{
    struct wav_header header;
    int samples_per_second = 44100;
    int bits_per_sample = 16;

    strncpy(header.riff_tag, "RIFF", 4);
    strncpy(header.wave_tag, "WAVE", 4);
    strncpy(header.frmt_tag, "fmt ", 4);
    strncpy(header.data_tag, "data", 4);

    header.riff_length = 0;
    header.frmt_length = 16;
    header.audi_format = 1;
    header.num_channel = 1;
    header.sample_rate = samples_per_second;
    header.byte_rate = samples_per_second*(bits_per_sample/8);
    header.block_align = bits_per_sample/8;
    header.bits_per_sample = bits_per_sample;
    header.data_length = 0;

    FILE * file = fopen(filename, "w+");
    if (!file) return NULL;

    fwrite(&header, sizeof(header), 1, file);
    fflush(file);
    return file;
}

void wavefile_write(FILE * file, int16_t data[], size_t length)
{
    fwrite(data, sizeof(int16_t), length, file);
}

void wavefile_close(FILE * file)
{
    size_t file_length = ftell(file);

    size_t data_length = 
        file_length - sizeof(struct wav_header);
    fseek(file, sizeof(struct wav_header) - sizeof(uint32_t), SEEK_SET);
    fwrite(&data_length, sizeof(data_length), 1, file);

    uint32_t riff_length = file_length - 8;
    fseek(file, 4, SEEK_SET);
    fwrite(&riff_length, sizeof(riff_length), 1, file);

    fclose(file);
}

const char __name__[] = "WavSink";
const char * WavSink::name()
{
    return __name__;
}

WavSink::WavSink(Memory * memory,
        TransceiverCallback cb,
        void * trans,
        const char filename[]):
    Module(memory, cb, trans)
{

    file = wavefile_open(filename);
    buffer = new int16_t[1024];
}

WavSink::~WavSink()
{
    wavefile_close(file);
}

Block * WavSink::process(Block * block)
{
    size_t n = 0;

    float ** iter = block->get_iterator();

    block->reset();

    do
    {
        buffer[n++] = (int16_t)((1 << 14) * (**iter));

        if (n >= 1024) {
            wavefile_write(file, buffer, 1024);
            n = 0;
        }
    } while(block->next());

    if (n > 0) {
        wavefile_write(file, buffer, n);
    }

    return block;
}
