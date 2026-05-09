# qf_math Speed vs libm by Architecture

This document is generated from the benchmark reports and shows only `qf_math` speed ratios. Values are `libm time / qf_math time`; values above `1.00` mean `qf_math` was faster than that platform's `libm`.

`---` means the source snapshot did not contain that function row. Regenerate the corresponding MCU snapshot after changing the shared benchmark matrix.

| Source | Generated |
| :--- | :--- |
| POSIX host | [BENCHMARK_REPORT.md](BENCHMARK_REPORT.md) · 2026-05-09 08:43:47Z |
| ESP32-S3 | [MCU_BENCHMARK_SNAPSHOT_ESP32S3.md](MCU_BENCHMARK_SNAPSHOT_ESP32S3.md) · 2026-05-09 05:35:25Z |
| Pico 2 ARM-M33 | [MCU_BENCHMARK_SNAPSHOT_PICO2_ARM.md](MCU_BENCHMARK_SNAPSHOT_PICO2_ARM.md) · 2026-05-09 08:14:43Z |
| Pico 2 Hazard3 RISC-V | [MCU_BENCHMARK_SNAPSHOT_PICO2_RISCV.md](MCU_BENCHMARK_SNAPSHOT_PICO2_RISCV.md) · 2026-05-09 08:22:54Z |

| Function | POSIX host | ESP32-S3 | Pico 2 ARM-M33 | Pico 2 Hazard3 RISC-V |
| :--- | ---: | ---: | ---: | ---: |
| `sin_rad` | 2.08 | 4.30 | 3.22 | 4.06 |
| `sin_deg` | 2.04 | 4.49 | 3.29 | 4.20 |
| `sin_bam` | 1.59 | 2.06 | 1.99 | 2.98 |
| `cos_rad` | 2.20 | 4.39 | 2.93 | 3.94 |
| `cos_deg` | 1.73 | 4.44 | 3.09 | 4.06 |
| `cos_bam` | 1.75 | 2.33 | 1.87 | 3.18 |
| `tan_rad` | 1.72 | 3.56 | 2.60 | 5.41 |
| `tan_deg` | 1.77 | 3.66 | 2.69 | 5.55 |
| `tan_bam` | 1.33 | 2.32 | 1.93 | 5.10 |
| `asin` | 0.21 | 1.10 | 3.55 | 0.88 |
| `acos` | 0.24 | 1.11 | 3.45 | 0.83 |
| `atan` | 0.99 | 1.20 | 0.87 | 1.52 |
| `atan2` | 0.79 | 1.41 | 0.70 | 1.93 |
| `sqrt` | 0.38 | 1.07 | 16.44 | 0.96 |
| `hypot` | 1.00 | 2.51 | 12.07 | 1.63 |
| `hypot_fast2` | 1.00 | 3.53 | 13.31 | 2.99 |
| `hypot_fast` | 0.72 | 2.65 | 10.54 | 2.08 |
| `log2` | 1.16 | --- | 0.94 | 1.90 |
| `ln` | 0.86 | 2.20 | 0.62 | 1.54 |
| `log10` | 0.87 | --- | 0.75 | 1.72 |
| `pow2` | 0.57 | --- | 2.14 | 4.52 |
| `exp` | 0.61 | 2.63 | 0.49 | 1.85 |
| `pow10` | 0.61 | --- | 4.15 | 6.06 |
| `pow` | 0.68 | --- | 2.40 | 2.86 |
