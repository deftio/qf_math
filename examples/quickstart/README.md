# Quick Start Example

Minimal C program demonstrating core qf_math functions: trig (radians, degrees, BAM), inverse trig, log/exp, sqrt/hypot, utility macros, waveform generators, and the fixed-radix bridge.

## Build and run

```bash
make
./quickstart
```

The only dependency is a C99 compiler (`gcc` or `clang`). No `-lm` needed — qf_math has no external dependencies.

## What it covers

| Category | Functions |
|----------|-----------|
| Trig (radians) | `qf_sin`, `qf_cos`, `qf_tan`, `qf_atan2` |
| Trig (degrees) | `qf_sin_deg`, `qf_cos_deg` |
| Trig (BAM) | `qf_sin_bam`, `qf_cos_bam` |
| Inverse trig | `qf_asin`, `qf_acos`, `qf_atan` |
| Log / exp | `qf_log2`, `qf_ln`, `qf_log10`, `qf_pow2`, `qf_exp`, `qf_pow` |
| Sqrt / hypot | `qf_sqrt`, `qf_hypot`, `qf_hypot_fast8` |
| Macros | `QF_DEG_TO_RAD`, `QF_RAD_TO_DEG`, `QF_CLAMP`, `QF_INTERP` |
| Waveforms | `qf_wave_sqr`, `qf_wave_tri`, `qf_wave_saw`, `qf_wave_noise` |
| Bridge | `QF_TO_FR`, `FR_TO_QF` (float ↔ fixed-radix conversion) |

## Integration into a project

Copy `src/qf_math.c` and `src/qf_math.h` into the project and compile `qf_math.c` as a single translation unit:

```bash
gcc -std=c99 -Os main.c qf_math.c -o main
```

For CMake, PlatformIO, ESP-IDF, or Arduino integration see the [Integration Guide](../../docs/INTEGRATION.md).
