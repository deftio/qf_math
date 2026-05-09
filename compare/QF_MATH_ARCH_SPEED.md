# `qf_math` Speed vs libm by Architecture

Matrix values come from **`### Speed vs libm`** in each snapshot (**`libm`** time ÷ implementation time). **> 1** → that implementation beats `libm` in this microbenchmark; **< 1** → `libm` wins.

The main architecture columns are **`qf_math`**. The three **`fr_math`** MCU columns marked **(cast to float)** use the same harness with `float` inputs/outputs around the fixed-point core (**`float→s32→…→float`**); they are not raw fixed-only throughput.

Host column titles are **`uname`/`Compiler`** from **`BENCHMARK_REPORT.md` § Host metadata** (not the machine that runs `make benchmark-arch-speed`).

**`libm`** column (**1.00**) is the denominator baseline (`libm` ÷ `libm`); snapshots use the same first column.

`---` → that snapshot has no ratio for that function row.

| Source | Generated |
| :--- | :--- |
| Host bench · Darwin x86_64 (Apple libm) | [BENCHMARK_REPORT.md](BENCHMARK_REPORT.md) · 2026-05-09 20:12:24Z |
| ESP32-S3 | [SNAPSHOT ESP32S3.md](MCU_BENCHMARK_SNAPSHOT_ESP32S3.md) · 2026-05-09 19:45:24Z |
| Pico 2 ARM-M33 | [SNAPSHOT PICO2 ARM.md](MCU_BENCHMARK_SNAPSHOT_PICO2_ARM.md) · 2026-05-09 19:27:59Z |
| Pico 2 Hazard3 RISC-V | [SNAPSHOT PICO2 RISCV.md](MCU_BENCHMARK_SNAPSHOT_PICO2_RISCV.md) · 2026-05-09 19:34:52Z |

| Function | **libm** | Host `qf_math` | ESP32-S3 `qf_math` | ESP32-S3 `fr_math` (cast to float) | Pico ARM `qf_math` | Pico ARM `fr_math` (cast to float) | Pico RISC-V `qf_math` | Pico RISC-V `fr_math` (cast to float) |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `sin_rad` | 1.00 | 1.59 | 4.30 | 1.32 | 3.25 | 0.76 | 4.10 | 4.93 |
| `sin_deg` | 1.00 | 1.69 | 4.49 | 1.35 | 3.27 | 0.75 | 4.19 | 5.10 |
| `sin_bam` | 1.00 | 1.64 | 2.05 | 1.65 | 2.01 | 1.19 | 3.00 | 4.67 |
| `cos_rad` | 1.00 | 1.77 | 4.39 | 0.90 | 2.95 | 0.52 | 3.95 | 3.94 |
| `cos_deg` | 1.00 | 1.79 | 4.45 | 1.12 | 3.06 | 0.61 | 4.08 | 4.63 |
| `cos_bam` | 1.00 | 1.64 | 2.33 | 1.55 | 1.86 | 1.03 | 3.18 | 5.20 |
| `tan_rad` | 1.00 | 4.79 | 3.56 | 1.31 | 2.59 | 0.80 | 5.45 | 6.06 |
| `tan_deg` | 1.00 | 4.47 | 3.66 | 1.26 | 2.67 | 0.76 | 5.55 | 6.08 |
| `tan_bam` | 1.00 | 3.93 | 2.32 | 2.42 | 1.93 | 1.58 | 5.12 | 8.42 |
| `asin` | 1.00 | 0.22 | 0.85 | 0.77 | 3.58 | 3.10 | 0.89 | 4.23 |
| `acos` | 1.00 | 0.21 | 0.77 | 0.77 | 3.48 | 3.28 | 0.84 | 4.31 |
| `atan` | 1.00 | 0.74 | 1.53 | 0.44 | 0.87 | 0.09 | 1.52 | 2.37 |
| `atan2` | 1.00 | 0.78 | 2.65 | 0.79 | 0.71 | 0.09 | 1.93 | 2.78 |
| `sqrt` | 1.00 | 0.27 | 1.07 | 0.08 | 16.81 | 1.43 | 0.96 | 0.59 |
| `hypot` | 1.00 | 1.53 | 2.56 | 0.21 | 12.26 | 1.19 | 1.63 | 0.96 |
| `hypot_fast2` | 1.00 | 1.67 | 3.65 | --- | 13.31 | --- | 2.98 | --- |
| `hypot_fast` | 1.00 | 1.30 | 2.71 | 2.71 | 10.69 | 9.54 | 2.09 | 3.31 |
| `log2` | 1.00 | 0.97 | 2.49 | 1.30 | 0.94 | 0.45 | 1.91 | 5.07 |
| `ln` | 1.00 | 0.71 | 1.64 | 0.92 | 0.62 | 0.33 | 1.54 | 4.22 |
| `log10` | 1.00 | 0.77 | 2.04 | 1.16 | 0.75 | 0.41 | 1.72 | 4.71 |
| `pow2` | 1.00 | 0.84 | 4.62 | 5.57 | 2.21 | 2.63 | 4.57 | 14.66 |
| `exp` | 1.00 | 0.98 | 1.55 | 1.88 | 0.51 | 0.57 | 1.86 | 6.12 |
| `pow10` | 1.00 | 0.82 | 4.13 | 4.98 | 4.26 | 4.92 | 6.11 | 25.63 |
| `pow` | 1.00 | 0.82 | 2.36 | 2.12 | 2.43 | 2.09 | 2.87 | 10.52 |
