# Kanshi (監視)

Kanshi is a real-time anomaly detection prototype built on µT-Kernel 3.0 and designed for the BBC micro:bit v2. The system samples vibration and temperature, extracts signal features, and runs a lightweight on-device inference pipeline to classify the current operating state as normal, warning, or critical.

This project combines a small RTOS task scheduler, sensor processing, a circular buffer, a health/alert state machine, and embedded firmware for a microcontroller target.

## Overview

Kanshi is intended as a portable embedded ML/RTOS demonstration:

- Vibration is sampled at 100 Hz in a dedicated high-priority task
- A 1-second sliding feature window is assembled from accelerometer data
- Time-domain features are computed from the window
- A lightweight anomaly classifier evaluates the feature vector
- Temperature is monitored to drive warning and critical events
- Alert state transitions are tracked and reported over serial output

## Hardware target

- BBC micro:bit v2
- MCU: Nordic nRF52833 (Cortex-M4F)
- Clock: 64 MHz
- Memory: 128 KB RAM, 512 KB Flash
- Sensor stack: onboard accelerometer and temperature sensor

## System behavior

The firmware creates several µT-Kernel tasks:

- Vibration sampling task
  - Runs at 10 ms period
  - Reads accelerometer data and pushes samples to a circular buffer
- Temperature monitoring task
  - Runs at 100 ms period
  - Tracks current temperature and emits warning/critical events
- ML inference task
  - Event-driven
  - Waits for a full window or thermal alert before processing features
- System monitor task
  - Periodic health reporting and watchdog/interaction handling

The project uses a lightweight anomaly model when TFLite Micro is disabled, as configured in the project settings. The default configuration sets the inference engine to the built-in fallback classifier path.

## Project structure

- `app_sample/` — application code, task logic, buffer logic, feature extraction, alert handling
- `include/` — public headers and configuration definitions
- `config/` — board and system configuration headers
- `device/` — device drivers for ADC, I2C, UART/serial and device-specific interfaces
- `kernel/` — µT-Kernel source structure and task framework
- `build_make/` — build system and target makefiles for the micro:bit port
- `etc/linker/` — linker script for the target board
- `lib/` — runtime libraries used by the kernel and firmware

## Configuration

The main configuration is in `include/kanshi_config.h`.

Key defaults:

- `SAMPLE_RATE_HZ = 100`
- `SAMPLE_PERIOD_MS = 10`
- `WINDOW_SAMPLES = 100`
- `NUM_FEATURES = 12`
- `TEMP_WARN_C = 45.0f`
- `TEMP_CRIT_C = 60.0f`
- `USE_TFLITE_MICRO = 0`
- `USE_SIMULATED_SENSORS = 1`

These settings are designed for a fast embedded loop and a compact, deterministic fallback detector.

## Build instructions

From the project root:

```bash
cd build_make
make clean && make
arm-none-eabi-objcopy -O ihex mtkernel_3.elf mtkernel_3.hex
```

The output file is `build_make/mtkernel_3.hex`.

## Flashing to the micro:bit

1. Connect the micro:bit to your computer via USB.
2. Copy the generated hex file to the mounted MICROBIT drive.
3. The board should flash automatically and reboot.

## Serial output

Monitor the debug UART output after reset:

```bash
screen /dev/tty.usbmodem* 115200
```

On macOS, the serial port may appear as something like `/dev/tty.usbmodemXXXXX`.

If `screen` is unavailable, `picocom` or a similar UART terminal can also be used:

```bash
picocom -b 115200 /dev/tty.usbmodemXXXXX
```

## Typical runtime behavior

When running normally, you should see:

- task startup messages
- vibration sampling activity
- window-ready event processing
- alert transitions between normal, warning, and critical states
- periodic system monitor output

## Notes on the design

This project is structured as an embedded demonstrator rather than a full production ML deployment. It emphasizes:

- deterministic task scheduling
- lightweight feature extraction
- easy cross-target porting through the µT-Kernel build system
- clear separation between sensor I/O, inference, and alert logic

## Dependencies

- GNU Make
- ARM GNU Toolchain (`arm-none-eabi-*`)
- BBC micro:bit v2 board
- µT-Kernel 3.0 source tree included in the project structure

## License

The project code in this repository is provided under the following terms:

- Application code: MIT
- µT-Kernel 3.0: T-License 2.2
- BSP and board support code: copyright belongs to the original authors as included with the source tree

## Status

This repository is an actively evolving embedded RTOS prototype for real-time anomaly detection on micro:bit hardware. It is best treated as a research and development project for firmware, scheduling, and embedded sensor analytics.

## Useful next steps

- integrate a real TFLite Micro model once model conversion and memory constraints are finalized
- add deeper logging or a host-based data collector
- expand the alert policy for more anomaly classes
- validate the build on a clean toolchain installation and a fresh micro:bit device
