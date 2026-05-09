# LilyGO T-Display-S3 — PlatformIO benchmark

Target board: **[LilyGO T-Display-S3](https://www.lilygo.cc/products/t-display-s3)** — ESP32-S3, 16 MB flash, ST7789 TFT (this sketch **does not** drive the display; output is **Serial @ 115200**).

Same portable workload as **[`compare/benchmark_core.c`](../../compare/benchmark_core.c)** / **`make compare`** (qf_math, newlib `sinf`/`…`, libfixmath + fr_math via float bridges), plus ESP32-S3-only fast-math peer sections for **FastTrig**, **ESP-DSP** sqrt, **espp/math** scalar helpers, and inverse trig (`asin`, `acos`, `atan`, `atan2`) where the float-domain peer exposes those functions.

From repo root, **`make mcu-benchmark-snapshot`** (needs **pyserial**, **pio**, USB) flashes this sketch and overwrites **[`compare/MCU_BENCHMARK_SNAPSHOT_ESP32S3.md`](../../compare/MCU_BENCHMARK_SNAPSHOT_ESP32S3.md)** (~many minutes — full compare loops).

Firmware still emits Markdown between serial markers `:::: DOC_TABLE_START ::::` … `:::: DOC_TABLE_END ::::` for manual capture if needed.

## One-time setup

1. **Compare deps** (libfixmath + fr_math sources):

   ```bash
   cd /path/to/qf_math
   make compare-deps
   ```

2. **PlatformIO `espressif32` ≥ 7.x** so board ID `lilygo-t-display-s3` exists. From this folder:

   ```bash
   pio pkg update
   ```

   If you still see `Unknown board ID 'lilygo-t-display-s3'`, your global platform was stale; updating inside this project pulls **Espressif 32 @ 7.x**.

3. **`intelhex` for esptool** (some installs hit `ModuleNotFoundError: No module named 'intelhex'` when building the bootloader):

   ```bash
   pio system info | awk '/Python Executable/ { print $NF }' | xargs -I PY PY -m pip install intelhex
   ```

   Use the **Python executable** line from `pio system info` (Homebrew PIO often uses its own `libexec` Python, not `~/.platformio/penv`).

## Build, flash, monitor

```bash
cd examples/lilygo_t_display_s3_bench
pio run -t upload -t monitor
```

- Pick the USB CDC port if prompted; or set `--upload-port /dev/cu.usbmodemXXXX` (macOS) / `COMx` (Windows).
- **`monitor_speed` is 115200** (see `platformio.ini`).

List ports:

```bash
pio device list
arduino-cli board list   # shows serial ports when board is connected
```

## Arduino CLI (FQBN only)

If you use **arduino-cli** for other workflows, the matching **FQBN** for this hardware is:

```text
esp32:esp32:lilygo_t_display_s3
```

Install core (once): `arduino-cli core install esp32:esp32`

This repo’s benchmark pulls many `.c` files via **`build_src_filter`** in PlatformIO; there is **no** standalone Arduino-CLI sketch here. Use **`pio run`** for this example unless you copy sources into your own sketch layout.

## Notes

- **`ARDUINO_LOOP_STACK_SIZE=65536`** — required because `benchmark_core.c` allocates ~32 KiB of `float` grids on stack per function.
- **FastTrig** is pulled from PlatformIO. **ESP-DSP** and **espp/math** are represented by tiny local scalar subsets because their full repositories pull large component/demo trees that do not build cleanly as Arduino PlatformIO libraries.
- **USB CDC:** many T-Display-S3 boards use USB serial; if the monitor is empty, try another USB mode / driver per [LilyGO’s docs](https://github.com/Xinyuan-LilyGO/T-Display-S3).
- **Different LilyGO board** (e.g. T3-S3): duplicate `platformio.ini`, change `board = …`, and check [PlatformIO boards](https://docs.platformio.org/en/latest/boards/index.html).
