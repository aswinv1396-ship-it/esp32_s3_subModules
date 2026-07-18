#ifndef INMP441_H
#define INMP441_H

#include <stdint.h>
#include "esp_err.h"

#define INMP441_BCLK   14
#define INMP441_WS     15
#define INMP441_SD     16

#define INMP441_SAMPLE_RATE 16000
#define AUDIO_BLOCK_SIZE    512

typedef enum
{
    INMP441_CHANNEL_LEFT = 0,
    INMP441_CHANNEL_RIGHT,
    INMP441_CHANNEL_STEREO

} inmp441_channel_t;


#define INMP441_CHANNEL_MODE INMP441_CHANNEL_LEFT			// L/R -> GND


esp_err_t inmp441_init(void);

int32_t inmp441_read_sample(void);

/*
 * Read multiple samples.
 *
 * buffer      : destination PCM buffer
 * sample_count: number of 16-bit samples requested
 *
 * Returns number of samples actually read.
 */
size_t inmp441_read_block(int16_t *buffer,  size_t sample_count);

void mic_task(void *arg);

#endif
