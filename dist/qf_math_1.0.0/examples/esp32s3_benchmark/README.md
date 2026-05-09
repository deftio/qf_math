# ESP32-S3 on-device benchmark

This is an **ESP-IDF** app that links **`qf_math`** together with **libfixmath**, **fr_math**, and **newlib `libm`** (`sinf`, …), then runs the **same** numerical loops as `make compare` on your PC ([`compare/benchmark_core.c`](../../compare/benchmark_core.c)).

## What you get

- **Wall-clock** numbers from **`esp_timer_get_time()`** (microsecond resolution, FreeRTOS tick jitter still applies).
- **Accuracy** checks versus double references (same grids as the host harness).
- **Important:** fixed-point peers are still exercised **through float bridges** here — same caveat as [`compare/README.md`](../../compare/README.md) § Float vs fixed-point. For native `fix16_t` / radix-`s32` timings you would add separate loops later.

The UART log ends with Markdown between serial markers `:::: DOC_TABLE_START ::::` and `:::: DOC_TABLE_END ::::`. The checked-in **`MCU_BENCHMARK_SNAPSHOT_ESP32S3.md`** is refreshed from the LilyGO PlatformIO build via **`make mcu-benchmark-snapshot`** (same kernels).

## Prerequisites

1. **ESP-IDF** v5.x installed and sourced (`IDF_PATH`, `idf.py` on `PATH`).
2. From the **repository root**, fetch compare dependencies (libfixmath + fr_math sources):

   ```bash
   make compare-deps
   ```

   That populates `build/compare/third_party/` (gitignored). `main/CMakeLists.txt` pulls sources from there.

3. **Main task stack:** each benchmark uses ~32 KiB of stack (`float inputs[8000]`). This example sets **`CONFIG_ESP_MAIN_TASK_STACK_SIZE=65536`** in `sdkconfig.defaults`. Do not shrink it without changing `benchmark_core.c`.

## Build / flash / monitor

```bash
cd examples/esp32s3_benchmark
idf.py set-target esp32s3
idf.py build flash monitor
```

Power the board, connect USB-UART, then reset — the full report prints once over the serial console (same layout as the host `benchmark_suite` human mode).

**LilyGO T-Display-S3 (USB + optional TFT):** use **[`examples/lilygo_t_display_s3_bench/`](../lilygo_t_display_s3_bench/README.md)** — PlatformIO board **`lilygo-t-display-s3`**, `pio run -t upload -t monitor`.

## Troubleshooting

- **CMake error — missing `fix16.c`:** run `make compare-deps` from the repo root first.
- **Panics / stack overflow:** confirm `CONFIG_ESP_MAIN_TASK_STACK_SIZE` is at least **65536** (merge `sdkconfig.defaults` into your `sdkconfig`, or `idf.py menuconfig` → Main task stack size).
- **Wi-Fi / Bluetooth:** left disabled by default; enable only if you need them — extra ISR load shifts timing.

## Files

| Path | Role |
|------|------|
| `main/app_main.c` | ESP timer adapter + `bench_run_all` / `bench_print_human` |
| `main/CMakeLists.txt` | Sources + include paths into repo root + `build/compare/third_party/` |
| `CMakeLists.txt` | ESP-IDF project wrapper |
| `sdkconfig.defaults` | ESP32-S3 target hints, large main stack, `-Os` friendly defaults |
