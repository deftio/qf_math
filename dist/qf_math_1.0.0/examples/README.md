# Examples

| Directory | Purpose |
|-----------|---------|
| [`lilygo_t_display_s3_bench/`](lilygo_t_display_s3_bench/README.md) | **PlatformIO + Arduino** — **LilyGO T-Display-S3**; same `benchmark_core.c` as `make compare`; Serial @ **115200** (TFT not used in this sketch). |
| [`esp32s3_benchmark/`](esp32s3_benchmark/README.md) | **ESP-IDF** firmware for a generic ESP32-S3 (`idf.py`); same kernels via UART. |
| [`pico2_benchmark/`](pico2_benchmark/) | **Arduino** sketch for **Raspberry Pi Pico 2 / Pico 2 W** (RP2350, ARM Cortex-M33 or Hazard3 RISC-V); same `benchmark_core.c` via Serial @ 115200. |

Host-side matrices and the Markdown snapshot remain under **[`compare/`](../compare/README.md)** (`make compare`, `make compare-github-report` → **`BENCHMARK_REPORT.md`**). Silicon capture: **`make mcu-benchmark-snapshot`** → **`compare/MCU_BENCHMARK_SNAPSHOT*.md`** (USB serial).
