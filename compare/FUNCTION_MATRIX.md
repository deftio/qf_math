# Function coverage matrix

**Numeric domains:** column **QF** is **`float`** (`qf` typedef). Columns **FR** / **FX** are **fixed-point** (`s32` radix, `fix16_t`). This matrix is about **capability alignment**, not identical interchangeability — pick **float** vs **fixed** first, then an implementation. Host **`make compare`** timings for FR/FX include **float bridges** unless you profile native fixed calls yourself.

Symbols are grouped by problem domain. **●** = first-class API in typical builds; **◐** = alternate name, submodule, compile-time option, or narrower domain/type; **○** = not provided as a practical single-call equivalent.

Columns:

| Col | Library |
|-----|---------|
| **QF** | **qf_math** (`float`) |
| **FR** | **fr_math** (fixed radix `s32`; see `FR_math.h`) |
| **FX** | **libfixmath** (`fix16_t` Q16.16) |
| **DSP** | **CMSIS-DSP** fast math / common kernels ([Fast Math](https://arm-software.github.io/CMSIS-DSP/latest/group__groupFastMath.html)) |
| **LIBM** | ISO **math.h** (`float`/`double`) |

Host **speed** and **`.o` size** snapshots for QF/FR/FX subsets live in `make compare` / `make compare-report`; **platform** suitability is summarized here. **ArmMathM0**, **Qfplib**, **FastTrig**, **CMSIS-DSP**, **SLEEF**, **micromath**, SIMD header libs, … — see **[PEERS.md](PEERS.md)** for links and licensing notes.

### Upstream regression suites (optional)

| Library | Command |
|---------|---------|
| libfixmath | `make compare-tests` |
| fr_math | `make compare-fr-tests` |

> CMSIS-DSP cells summarize the *family* of routines (there are often separate `f32`, `q15`, `q31` variants—treat **◐** as “available but type-specific”).

## Core macros & utility

| Capability | QF | FR | FX | DSP | LIBM |
|------------|----|----|----|-----|------|
| Min/max/clamp/interp macros | ● | ● | ◐ | ◐ vector ops elsewhere | ◐ |
| `abs`/sign helpers | ● | ● | ● | ● intrinsics vary | ● |
| Deg ↔ rad ↔ BAM conversions | ● | ● | ◐ | ◐ | ◐ |

## Trigonometry

| Capability | QF | FR | FX | DSP | LIBM |
|------------|----|----|----|-----|------|
| sin/cos/tan (rad) | ● | ● | ● | ● `arm_sin_f32` / `arm_cos_f32` | ● |
| sin/cos/tan (degrees / helpers) | ● | ● | ◐ | ◐ | ◐ |
| BAM-native sin/cos/tan | ● | ● | ◐ | ◐ | ◐ |
| Inverse sin/cos/tan/atan2 | ● | ● | ● | ● `arm_atan2_f32` | ● |

## Logarithms & exponentials

| Capability | QF | FR | FX | DSP | LIBM |
|------------|----|----|----|-----|------|
| log2 / ln / log10 | ● | ● | ● | ● `arm_vlog_f32` etc. (vector) / partial scalar coverage | ● |
| exp / pow10 / pow2 | ● | ● / macro | ● | ◐ mostly vector/log pipelines | ● |

## Powers & roots

| Capability | QF | FR | FX | DSP | LIBM |
|------------|----|----|----|-----|------|
| sqrt | ● | ● | ● | ● `arm_sqrt_f32` | ● |
| hypot / fast hypot | ● / ● fast8 | ● / ● fast8 | ◐ | ◐ | ● |
| General `pow(x,y)` | ○ | ○ | ◐ limited | ◐ | ● |

## Waves & envelopes (embedded synth helpers)

| Capability | QF | FR | FX | DSP | LIBM |
|------------|----|----|----|-----|------|
| PWM/square/saw/triangle/noise | ● | ● | ○ | ◐ generators elsewhere | ○ |
| ADSR envelope | ● | ● | ○ | ◐ | ○ |

## Transforms & linear algebra (high level)

| Capability | QF | FR | FX | DSP | LIBM |
|------------|----|----|----|-----|------|
| 2D rotate / affine helpers | ○ | ● (`FR_math_2D.h`) | ◐ | ● matrices | ◐ |
| Complex FFT / FIR / CMSIS kernels | ○ | ◐ utilities only | ● `fix16_fft.c` | ● extensive | ◐ |

## Formatting / diagnostics

| Capability | QF | FR | FX | DSP | LIBM |
|------------|----|----|----|-----|------|
| Print/parse numbers (embedded-safe optional) | ○ | ◐ `FR_print*` optional | ● string helpers | ○ | ● printf ecosystem |

### How to extend this matrix

When you add functions to `qf_math`, update **QF** column here and add/adjust tests in `test/qf_math_test.c`. For vendor SDKs, cite the exact header symbols in your PR notes so future readers can verify **DSP** cells quickly.
