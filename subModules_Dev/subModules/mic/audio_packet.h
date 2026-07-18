#ifndef AUDIO_PACKET_H
#define AUDIO_PACKET_H

#include <stdint.h>

#define AUDIO_PACKET_MAGIC       0x41554430U
#define AUDIO_PACKET_VERSION     1U

#define AUDIO_SAMPLE_RATE        16000U
#define AUDIO_CHANNELS           1U
#define AUDIO_BITS_PER_SAMPLE    16U

#define AUDIO_SAMPLES_PER_PACKET 512U

typedef struct 
{
    uint32_t magic;
    uint8_t  version;
    uint8_t  channels;
    uint16_t sample_count;

    uint32_t sample_rate;
    uint32_t sequence_number;

    int16_t samples[AUDIO_SAMPLES_PER_PACKET];

} audio_packet_t;

#endif
