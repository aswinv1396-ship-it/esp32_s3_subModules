#ifndef INMP441_H
#define INMP441_H

#include <stdint.h>
#include "esp_err.h"

#define INMP441_BCLK   14
#define INMP441_WS     15
#define INMP441_SD     16

#define INMP441_SAMPLE_RATE 16000


esp_err_t inmp441_init(void);

int32_t inmp441_read_sample(void);

#endif
