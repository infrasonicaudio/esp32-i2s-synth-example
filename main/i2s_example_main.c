/* I2S Synthesis example

    This example code will output a sine wave of arbitrary frequency WAVE_FREQ_HZ
    (default 235Hz) at 44100kHz sample rate to the internal 8-bit DAC channels of
    the ESP32.

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
#define DMA_BUF_LEN     (32)
#define DMA_NUM_BUF     (2)
#define I2S_NUM         (0)
#define WAVE_FREQ_HZ    (235)
#define TWOPI           (6.28318531)
#define PHASE_INC       (TWOPI * WAVE_FREQ_HZ / SAMPLE_RATE)

static const char* TAG = "i2s_synth_example";

// Accumulated phase
static float p = 0.0;

// Output buffer (2ch interleaved)
static uint16_t out_buf[DMA_BUF_LEN * 2];

// Fill the output buffer and write to I2S DMA
static void write_buffer()
{
    float samp = 0.0;
    size_t bytes_written;

    for (int i=0; i < DMA_BUF_LEN; i++) {
        // Scale sine sample to 0-1 for internal DAC
        // (can't output negative voltage)
        samp = (sinf(p) + 1.0) * 0.5;

        // Increment and wrap phase
        p += PHASE_INC;
        if (p >= TWOPI)
            p -= TWOPI;
        
        // Scale to 8-bit integer range
        samp *= 255.0;

        // Shift to MSB of 16-bit int for internal DAC
        out_buf[i*2] = out_buf[i*2+1] = (uint16_t)samp << 8;
    }

    // Write with max delay. We want to push buffers as fast as we
    // can into DMA memory. If DMA memory isn't transmitted yet this
    // will yield the task until the interrupt fires when DMA buffer has 
    // space again. If we aren't keeping up with the real-time deadline,
    // audio will glitch and the task will completely consume the CPU,
    // not allowing any task switching interrupts to be processed.
    i2s_write(I2S_NUM, out_buf, sizeof(out_buf), &bytes_written, portMAX_DELAY);

    // You could put a taskYIELD() here to ensure other tasks always have a chance to run.
    // taskYIELD();
}

static void audio_task(void *userData)
{
    while(1) {
        write_buffer();
    }
}

void app_main(void)
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = DMA_NUM_BUF,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

    i2s_set_pin(I2S_NUM, NULL); // Internal DAC

    // Highest possible priority for realtime audio task
    xTaskCreate(audio_task, "audio", 1024, NULL, configMAX_PRIORITIES - 1, NULL);
}
