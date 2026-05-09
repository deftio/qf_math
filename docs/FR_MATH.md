# Choosing between Quick Float and Fixed-Point Math

**[qf_math](https://github.com/deftio/qf_math)** operates on IEEE-754 `float`. **[fr_math](https://github.com/deftio/fr_math)** operates on fixed-point integers with a **caller-selectable radix** — the radix parameter controls how many of the 32 bits are fractional (e.g. radix 16 = s15.16, radix 12 = s19.12, radix 8 = s23.8). Both libraries expose the same set of functions — trig, inverse trig, log, exp, sqrt, hypot, waveform generators, ADSR envelopes — so switching from one to the other is largely a matter of changing the include and the numeric type. The API names follow a parallel convention: `qf_sin()` / `fr_sin()`, `qf_log2()` / `FR_log2()`, and so on.


## Choosing between them

| Factor | qf_math (float) | fr_math (fixed-point) |
|--------|-----------------|----------------------|
| **Best targets** | Cortex-M4F/M7, ESP32, RP2040/RP2350 with FPU, any core with hardware single-precision | Cortex-M0/M0+, 8/16-bit MCUs, RISC-V without F extension, any core without FPU |
| **Soft-float cost** | High — compiler inserts software multiply/add when no FPU is present | Zero — pure integer ALU throughout |
| **Dynamic range** | ~1e-38 to ~3.4e+38 | Determined by radix choice (e.g. R=16: ±32K; R=12: ±512K; R=8: ±8M) |
| **Precision** | ~7 significant digits (float32) | Determined by radix choice (e.g. R=16: ~1.5e-5 resolution; R=12: ~2.4e-4) |
| **Typical accuracy** | < 0.002% FS (trig), < 0.001% rel (log/exp) | < 0.008% FS (trig), < 0.4% rel (log/exp) |
| **Table ROM** | ~4.4 KB (512-entry float tables) | ~908 bytes (quadrant/octant `unsigned short` tables) |
| **16-bit support** | Requires 32-bit float | Supports 16-bit platforms (s16 mode) |
| **Language** | C99 | C89 (pre-C99 fallback typedefs for compilers without `<stdint.h>`) |

In short: if the target has a hardware FPU or the pipeline already works in `float`, use qf_math. If the target has no FPU or soft-float overhead is unacceptable, use fr_math and pick the radix that fits the application's range/resolution needs. Both produce the same outputs for the same mathematical inputs — the accuracy and speed trade-offs differ with the numeric representation.


## What they share

Both libraries use BAM (Binary Angular Measure) phase, lookup-based approximation with sub-step linear interpolation, the same 8-segment piecewise-linear fast hypot, and identical waveform/ADSR API shapes.

| Feature | qf_math | fr_math |
|---------|---------|---------|
| BAM phase | `uint16_t`, 65536 = full turn | `uint16_t`, 65536 = full turn |
| Log2 / Pow2 tables | 65-entry, `float` | 65-entry, `u32` (s.16 fixed-point) |
| hypot_fast8 | 8-segment piecewise-linear (float multiply) | 8-segment piecewise-linear (shift-only, no multiply) |
| Waveforms | sqr/pwm/tri/saw/noise via BAM phase | sqr/pwm/tri/saw/noise via BAM phase |
| ADSR | `qf_adsr_t` (float levels) | `fr_adsr_t` (fixed-point levels) |


## Where the internals differ

The table layouts and numerical methods are tuned to each domain:

| Algorithm | qf_math | fr_math |
|-----------|---------|---------|
| **Sine table** | 512-entry **full-cycle**, `float` (2048 bytes) | 129-entry **first-quadrant**, `unsigned short` u0.15 (258 bytes); other quadrants via symmetry |
| **Tangent table** | 512-entry **full-cycle**, `float` (2048 bytes) | 65-entry **first-octant**, `unsigned short` u0.15 (130 bytes); second octant via `tan(x) = 1/tan(pi/2 - x)` |
| **Interpolation** | 7-bit sub-step linear interpolation | 7-bit sub-step linear interpolation + small-angle approximations near cardinals (< 0.7 deg) |
| **Inverse trig** | Piecewise Hermite polynomial spans; atan2 via reciprocal reduction | Binary search on ascending sine quadrant table; near-1.0 fast path `acos(x) ~ sqrt(2(1-x))`; atan2 via hypot_fast8 + conditional asin/acos |
| **Sqrt** | Newton-Raphson on `1/sqrt(x)`, seeded by IEEE 754 magic constant | Non-restoring shift-and-subtract digit-by-digit (no multiply, no divide) |
| **Reciprocal** | IEEE 754 magic constant + Newton-Raphson | Shift-based initial estimate + Newton-Raphson |
| **Exact hypot** | `qf_sqrt(x*x + y*y)` | `fr_isqrt64(x*x + y*y)` (64-bit integer product + digit-by-digit sqrt) |
| **Small-angle paths** | None (table covers full range uniformly) | `sin(theta) ~ theta`, `tan(delta) ~ delta` near zero-crossings; `cot(delta) ~ 1/delta` near poles; atan2 uses `asin(x) ~ x` below ~4.8 deg |

qf_math uses larger `float` tables (4 bytes/entry) for uniform full-cycle indexing. fr_math packs quadrant/octant tables into `unsigned short` (2 bytes/entry) and recovers the full circle via symmetry, keeping total table ROM under 1 KB.


## Switching between them

Because the API names are parallel, porting between the two is straightforward. The main change is the numeric type and the include:

```c
/* qf_math — float pipeline */
#include "qf_math.h"
qf y   = qf_sin(angle);
qf len = qf_hypot(dx, dy);
qf dB  = 20.0f * qf_log10(v / ref);

/* fr_math — fixed-point pipeline (radix 16 = s15.16) */
#define R 16
#include "FR_math.h"
s32 y   = fr_sin(angle, R);
s32 len = FR_hypot(dx, dy, R);
s32 dB  = FR_MUL(I2FR(20, R), FR_log10(FR_DIV(v, ref, R), R, R), R);
```

fr_math calls carry an explicit radix parameter — change `R` to trade range for resolution. qf_math functions take and return plain `float`.

### Bridging macros

`qf_math.h` includes macros for converting between `float` and fixed-radix integers at system boundaries:

```c
int32_t fr_val = QF_TO_FR(1.234f, 16);     // float → Q16.16 (truncating)
int32_t fr_val = QF_TO_FR_RND(1.234f, 16);  // float → Q16.16 (rounding)
qf float_val   = FR_TO_QF(80871, 16);       // Q16.16 → float
```

The C++ wrapper provides the same conversions:

```cpp
int32_t fr = qf_math::to_fr(1.234f, 16);
qf back    = qf_math::from_fr(fr, 16);
```


## Speed comparison

The `compare/` harness benchmarks both libraries (and others) against libm on the same hardware. Ratios below are **libm time / library time** — values above 1.0 mean the library is faster than libm.

| Function | qf_math | fr_math | Notes |
|----------|--------:|--------:|:------|
| sin (rad) | **4.30**x | 1.32x | |
| cos (rad) | **4.39**x | 0.90x | |
| tan (rad) | **3.56**x | 1.31x | |
| asin | 1.10x | 0.77x | |
| acos | 1.11x | 0.77x | |
| atan2 | **1.41**x | 0.70x | |
| sqrt | 1.07x | 0.08x | libm sqrtf often maps to a hardware instruction |
| exp | **2.63**x | 1.88x | |
| ln | **2.20**x | 0.92x | |
| hypot | **2.51**x | 0.21x | fr_math hypot goes through float bridge overhead |

ESP32-S3 (with FPU). fr_math numbers include **float↔fixed bridge overhead** — native `s32` / `fix16_t` calls in a pure-integer pipeline are faster. See [`compare/BENCHMARK_CROSSPLATFORM.md`](../compare/BENCHMARK_CROSSPLATFORM.md) for full matrices including multiple architectures, libfixmath, FastTrig, and ESP-DSP.
