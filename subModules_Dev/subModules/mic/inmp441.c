#include "inmp441.h"

#include "driver/i2s_std.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "INMP441";

static i2s_chan_handle_t rx_handle;

esp_err_t inmp441_init(void)
{

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG( I2S_NUM_0, I2S_ROLE_MASTER );

    ESP_ERROR_CHECK( i2s_new_channel( &chan_cfg,  NULL, &rx_handle );

    i2s_std_config_t std_cfg =
    {
        .clk_cfg =  I2S_STD_CLK_DEFAULT_CONFIG( INMP441_SAMPLE_RATE  ),

        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(  I2S_DATA_BIT_WIDTH_32BIT,  I2S_SLOT_MODE_MONO  ),

        .gpio_cfg =
        {
            .mclk =  I2S_GPIO_UNUSED,
            .bclk =  INMP441_BCLK,
            .ws =    INMP441_WS,
            .dout =  I2S_GPIO_UNUSED,
            .din =   INMP441_SD,
            .invert_flags =
            {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false
            }
        }
    };


    ESP_ERROR_CHECK( i2s_channel_init_std_mode( rx_handle,  &std_cfg   ) );

    ESP_ERROR_CHECK( i2s_channel_enable(rx_handle));

    ESP_LOGI(TAG,"INMP441 started");

    return ESP_OK;
}

int32_t inmp441_read_sample(void)
{
    int32_t sample = 0;

    size_t bytes_read = 0;

    i2s_channel_read( rx_handle,  &sample,  sizeof(sample),&bytes_read,  portMAX_DELAY);

    // INMP441 sends 24-bit data inside 32-bit frame
    sample >>= 14;

    return sample;
}

#define INMP441_BLOCK_SAMPLES    1024

size_t inmp441_read_block(int16_t *buffer,  size_t sample_count)
{
    static int32_t raw_buffer[INMP441_BLOCK_SAMPLES];

    if(sample_count > INMP441_BLOCK_SAMPLES)
    {
        sample_count = INMP441_BLOCK_SAMPLES;
    }

    size_t bytes_read = 0;

    esp_err_t ret =
        i2s_channel_read(
            rx_handle,
            raw_buffer,
            sample_count * sizeof(int32_t),
            &bytes_read,
            portMAX_DELAY);

    if(ret != ESP_OK)
    {
        return 0;
    }

    size_t samples = bytes_read / sizeof(int32_t);

    for(size_t i = 0; i < samples; i++)
    {
        buffer[i] = (int16_t)(raw_buffer[i] >> 14);
    }

    return samples;
}
