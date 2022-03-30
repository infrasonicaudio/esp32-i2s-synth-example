| Supported Targets | ESP32 |
| ----------------- | ----- |

# I2S real-time synthesis example

This example sets up a task to generate real-time synthesized audio (a sine wave of arbitrary frequency)
and output via I2S using the internal 8-bit DACs on the ESP32.

The example code uses a 32-sample, 2ch buffer size for minimal latency, with only 2 DMA buffers.
The expectation is that the CPU is able to keep up with producing a new 32 sample buffer while
DMA is transmitting the other buffer to I2S. You can change these values to increase available
time to run DSP while trading off for latency. [Good overview here](https://blog.atomic14.com/2021/04/20/esp32-i2s-dma-buf-len-buf-count.html).

## How to Use Example

### Hardware Required

* A development board with ESP32 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for power supply and programming
* Some way to connect the DAC pins to line-level inputs (mixer or audio interface)

⚠️ Not recommended to connect headphones directly to the DAC pins. At best they probably will not work, 
since the internal DAC cannot drive enough current through them, at worst it could burn out the DACs.

### Configure the Project

```
idf.py menuconfig
```

An `sdkconfig.defaults` file is provided for convenience. The only changes beyond system
defaults are to increase the CPU Frequency to 240 MHz and max out the RTOS tick rate at 1000Hz.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

You should see some information about the I2S clocks logged to the console. A sine wave
will be produced on the internal DAC pins of the ESP32. You can listen to the output
by wiring the DAC pins to a line-level audio input, e.g. on an audio interface or mixer.
