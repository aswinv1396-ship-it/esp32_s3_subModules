#include "inmp441.h"
#include "audio_packet.h"
#include "websocket_client.h"

#include "driver/i2s_std.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "INMP441";
static i2s_chan_handle_t rx_handle;

esp_err_t inmp441_init(void)
{
    i2s_chan_config_t chan_cfg =   I2S_CHANNEL_DEFAULT_CONFIG( I2S_NUM_0, I2S_ROLE_MASTER  );
    /*
     * Reduce DMA latency
     */
    chan_cfg.dma_desc_num = 8;
    chan_cfg.dma_frame_num = 512;

    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg,  NULL,  &rx_handle));

    i2s_std_config_t std_cfg =
    {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG( INMP441_SAMPLE_RATE   ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(  I2S_DATA_BIT_WIDTH_32BIT, (INMP441_CHANNEL_MODE ==  INMP441_CHANNEL_STEREO )   ?  I2S_SLOT_MODE_STEREO  :    I2S_SLOT_MODE_MONO
            ),


        .gpio_cfg =
        {
            .mclk = I2S_GPIO_UNUSED,

            .bclk = INMP441_BCLK,

            .ws = INMP441_WS,

            .dout = I2S_GPIO_UNUSED,

            .din = INMP441_SD
        }
    };

#if INMP441_CHANNEL_MODE == INMP441_CHANNEL_LEFT

    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;


#elif INMP441_CHANNEL_MODE == INMP441_CHANNEL_RIGHT

    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT;


#elif INMP441_CHANNEL_MODE == INMP441_CHANNEL_STEREO

    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;

#endif

    ESP_ERROR_CHECK( i2s_channel_init_std_mode( rx_handle,  &std_cfg  ) );

    ESP_ERROR_CHECK( i2s_channel_enable(rx_handle ) );

    ESP_LOGI(TAG, "INMP441 started");

    return ESP_OK;
}

static inline int16_t inmp441_convert_sample(int32_t raw)
{
    int32_t sample =  raw >> 8;
    return (int16_t)(sample >> 8);
}

size_t inmp441_read_block(int16_t *buffer, size_t samples)
{
    static int32_t raw[1024];

    if(samples > 1024)
        samples = 1024;

    size_t bytes = 0;
    esp_err_t ret =  i2s_channel_read( rx_handle, raw,   samples * sizeof(int32_t), &bytes,  portMAX_DELAY );

    if(ret != ESP_OK)
        return 0;

    size_t count =  bytes / sizeof(int32_t);

    for(size_t i=0;i<count;i++)
    {
        buffer[i] = inmp441_convert_sample(raw[i] );
    }

    return count;
}
/*
void mic_task(void *arg)
{
    int16_t pcm[AUDIO_BLOCK_SIZE];
    uint32_t print_count = 0;
    while(1)
    {
        size_t samples = inmp441_read_block(  pcm, AUDIO_BLOCK_SIZE );
        if(samples)
        {
            if(print_count++ % 100 == 0)
            {
                printf( "MIC: %d %d %d %d %d\n",   pcm[0],  pcm[1],  pcm[2],  pcm[3], pcm[4]  );
            }
        }

        vTaskDelay( pdMS_TO_TICKS(1)
        );
    }
}
*/

void mic_task(void *arg)
{

    ESP_LOGI(TAG, "Mic started Listening..");

    static audio_packet_t packet;

    uint32_t sequence = 0;

    while (1)
    {
        size_t samples = inmp441_read_block(packet.samples, AUDIO_SAMPLES_PER_PACKET );

        if (samples == AUDIO_SAMPLES_PER_PACKET)
        {
            packet.magic = AUDIO_PACKET_MAGIC;
            packet.version = AUDIO_PACKET_VERSION;
            packet.channels = AUDIO_CHANNELS;
            packet.sample_count = samples;
            packet.sample_rate = AUDIO_SAMPLE_RATE;
            packet.sequence_number = sequence++;

            esp_err_t ret = websocket_send_audio(&packet);

            if (ret != ESP_OK)
            {
                ESP_LOGW(  TAG, "Audio packet send failed: %s",  esp_err_to_name(ret));
            }
        }
    }
}