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


#define SAMPLE_RATE     (36000)
#define I2S_NUM         (0)
#define WAVE_FREQ_HZ    (100)
#define PI              (3.14159265)
#define I2S_BCK_IO      (GPIO_NUM_4)
#define I2S_WS_IO       (GPIO_NUM_5)
#define I2S_DO_IO       (GPIO_NUM_18)
#define I2S_DI_IO       (-1)

#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)

static const char* TAG = "i2s_example";

static void setup_waves()
{
    size_t buf_size = sizeof(uint16_t) * SAMPLE_PER_CYCLE * 2;
    uint16_t *samples_data = malloc(buf_size);
    unsigned int i, sample_val;
    double sin_float;
    // double sin_float, triangle_float, triangle_step = (double) pow(2, bits) / SAMPLE_PER_CYCLE;
    size_t i2s_bytes_write = 0;

    printf("\r\nTest bits=%d free mem=%d, written data=%d\n", 16, esp_get_free_heap_size(), buf_size);

    // triangle_float = -(pow(2, bits)/2 - 1);

    for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
        sin_float = (sin(i * 2 * PI / SAMPLE_PER_CYCLE) + 1.0) * 0.5;
        // if(sin_float >= 0)
        //     triangle_float += triangle_step;
        // else
        //     triangle_float -= triangle_step;

        sin_float *= 255.0;
        samples_data[i*2] = samples_data[i*2+1] = (uint16_t)sin_float << 8;

        // if (bits == 16) {
        //     sample_val = 0;
        //     sample_val += (short)triangle_float;
        //     sample_val = sample_val << 16;
        //     sample_val += (short) sin_float;
        //     samples_data[i] = sample_val;
        // } else if (bits == 24) { //1-bytes unused
        //     samples_data[i*2] = ((int) triangle_float) << 8;
        //     samples_data[i*2 + 1] = ((int) sin_float) << 8;
        // } else {
        //     samples_data[i*2] = ((int) triangle_float);
        //     samples_data[i*2 + 1] = ((int) sin_float);
        // }

    }
    // ESP_LOGI(TAG, "set clock");
    // i2s_set_clk(I2S_NUM, SAMPLE_RATE, bits, 2);
    //Using push
    // for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
    //     if (bits == 16)
    //         i2s_push_sample(0, &samples_data[i], 100);
    //     else
    //         i2s_push_sample(0, &samples_data[i*2], 100);
    // }
    // or write
    ESP_LOGI(TAG, "write data");
    i2s_write(I2S_NUM, samples_data, buf_size, &i2s_bytes_write, 100);
    ESP_LOGI(TAG, "written bytes: %d", i2s_bytes_write);

    free(samples_data);
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
        .dma_buf_count = 6,
        .dma_buf_len = 60,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1                                //Interrupt level 1
    };
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, NULL);

    setup_waves();

    while (1) {
        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
}
