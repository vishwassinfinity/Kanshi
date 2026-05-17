# Kanshi (監視)

Real-time anomaly detection system built on µT-Kernel 3.0, running on BBC micro:bit v2.

---

## What works right now

- µT-Kernel 3.0 boots on micro:bit v2 (nRF52833, Cortex-M4F)
- RTOS task running — LED blinks every 500ms via `tk_dly_tsk()`
- UART output confirmed over serial
- Build system working: ARM GCC 15.2 + GNU Make on macOS

---

## Hardware

BBC micro:bit v2 — nRF52833, Cortex-M4F @ 64MHz, 128KB RAM, 512KB Flash

---

## Building

```bash
cd build_make
make clean && make
arm-none-eabi-objcopy -O ihex mtkernel_3.elf mtkernel_3.hex
```

Copy `mtkernel_3.hex` to the MICROBIT USB drive. Board flashes and reboots automatically.

---

## Serial monitor

```bash
screen /dev/tty.usbmodem102 115200
```

Press reset button on the back of the board to see output.

---

## Stack

- RTOS: µT-Kernel 3.0 (Personal Media Corporation, micro:bit BSP)
- Compiler: ARM GCC Embedded 15.2
- Board: BBC micro:bit v2

---

## License

Application code — MIT

µT-Kernel 3.0 — T-License 2.2 (Copyright © 2006-2022 Ken Sakamura, TRON Forum)

BSP — Copyright © 2010-2023 Personal Media Corporation