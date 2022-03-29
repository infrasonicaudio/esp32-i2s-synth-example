/* I2S Example

    This example code will output 100Hz sine wave and triangle wave to 2-channel of I2S driver
    Every 5 seconds, it will change bits_per_sample [16, 24, 32] for i2s data

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include <math.h>

#define SAMPLE_RATE     (44100)
#define DMA_BUF_LEN     (256)
#define DMA_NUM_BUF     (2)
#define I2S_NUM         (0)
#define WAVE_FREQ_HZ    (440)
#define PI              (3.14159265)
#define PHASE_INC       (2.0 * PI * WAVE_FREQ_HZ / SAMPLE_RATE)

// #define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)

static const char* TAG = "i2s_example";

// static void setup_waves()
// {
//     size_t buf_size = sizeof(uint16_t) * SAMPLE_PER_CYCLE * 2;
//     uint16_t *samples_data = malloc(buf_size);
//     unsigned int i, sample_val;
//     double sin_float;
//     size_t i2s_bytes_write = 0;

//     printf("\r\nTest bits=%d free mem=%d, written data=%d\n", 16, esp_get_free_heap_size(), buf_size);

//     // triangle_float = -(pow(2, bits)/2 - 1);

//     for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
//         sin_float = (sin(i * 2 * PI / SAMPLE_PER_CYCLE) + 1.0) * 0.5;
//         sin_float *= 255.0;
//         samples_data[i*2] = samples_data[i*2+1] = (uint16_t)sin_float << 8;
//     }

//     ESP_LOGI(TAG, "write data");
//     i2s_write(I2S_NUM, samples_data, buf_size, &i2s_bytes_write, 100);
//     ESP_LOGI(TAG, "written bytes: %d", i2s_bytes_write);

//     free(samples_data);
// }

static float p = 0.0;
static int count = 0;
static uint16_t out_buf[DMA_BUF_LEN * 2];

static void write_buffer()
{
    float samp = 0.0;
    size_t bytes_written;

    for (int i=0; i < DMA_BUF_LEN; i++) {
        samp = (sinf(p) + 1.0) * 0.5;
        p += PHASE_INC;
        if (p >= 2.0*PI)
            p -= 2.0*PI;
        
        samp *= 255.0;
        out_buf[i*2] = out_buf[i*2+1] = (uint16_t)samp << 8;
    }

    i2s_write(I2S_NUM, out_buf, sizeof(out_buf), &bytes_written, portMAX_DELAY);
}

static void audio_task(void *user)
{
    while(1) write_buffer();
}

void app_main(void)
{
    //for 36Khz sample rates, we create 100Hz sine wave, every cycle need 36000/100 = 360 samples (4-bytes or 8-bytes each sample)
    //depend on bits_per_sample
    //using 6 buffers, we need 60-samples per buffer
    //if 2-channels, 16-bit each channel, total buffer is 360*4 = 1440 bytes
    //if 2-channels, 24/32-bit each channel, total buffer is 360*8 = 2880 bytes
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = DMA_NUM_BUF,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1                                //Interrupt level 1
    };
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, NULL);

    xTaskCreate(audio_task, "audio", 4096, NULL, configMAX_PRIORITIES - 1, NULL);

    while (1) {
        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
}
