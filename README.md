| Supported Targets | ESP32 |
| ----------------- | ----- |

# I2S Real-time synth example

This example sets up a task to generate real-time synthesized audio (a sine wave of arbitrary frequency)
and output via I2S using the internal 8-bit DACs on the ESP32

## How to Use Example

### Hardware Required

* A development board with ESP32 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for power supply and programming

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

Not recommended to use headphones. At best they probably will not work as the internal 
DAC cannot drive enough current through them, at worst it could burn out the DACs.