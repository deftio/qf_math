# qf_math API Reference

Complete reference for public symbols declared in [`src/qf_math.h`](../src/qf_math.h).

---

## Type

```c
typedef float qf;
```

All library functions operate on IEEE-754 single-precision floats. The `qf` typedef makes the intent explicit and allows grepping for the API surface.

---

## Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `QF_PI` | 3.14159265 | pi |
| `QF_TWO_PI` | 6.28318530 | 2 * pi |
| `QF_HALF_PI` | 1.57079632 | pi / 2 |
| `QF_E` | 2.71828182 | Euler's number |
| `QF_INV_E` | 0.36787944 | 1 / e |
| `QF_INV_PI` | 0.31830988 | 1 / pi |
| `QF_DEG2RAD_K` | 0.01745329 | pi / 180 |
| `QF_RAD2DEG_K` | 57.2957795 | 180 / pi |
| `QF_LOG2E` | 1.44269504 | log2(e) |
| `QF_LN2` | 0.69314718 | ln(2) |
| `QF_LOG2_10` | 3.32192809 | log2(10) |
| `QF_LOG10_2` | 0.30102999 | log10(2) |
| `QF_SQRT2` | 1.41421356 | sqrt(2) |
| `QF_INV_SQRT2` | 0.70710678 | 1 / sqrt(2) |
| `QF_SQRT3` | 1.73205080 | sqrt(3) |
| `QF_INV_SQRT3` | 0.57735026 | 1 / sqrt(3) |
| `QF_SQRT5` | 2.23606797 | sqrt(5) |
| `QF_SQRT10` | 3.16227766 | sqrt(10) |

---

## Conversion macros

### Fixed-radix bridge

```c
#define QF_TO_FR(val, radix)      // float -> fixed-radix s32, truncating
#define QF_TO_FR_RND(val, radix)  // float -> fixed-radix s32, round-to-nearest
#define FR_TO_QF(val, radix)      // fixed-radix s32 -> float
```

Bridge between `qf` floats and `fr_math`-style `s32` integers at system boundaries (sensor input, DAC output, protocol fields).

### Integer conversions

```c
#define QF_FROM_INT(x)   // int -> qf
#define QF_TO_INT(x)     // qf -> int32_t (truncate)
#define QF_ROUND(x)      // qf -> int32_t (round-to-nearest)
```

### Angular conversions

```c
#define QF_DEG_TO_RAD(x)   // degrees -> radians
#define QF_RAD_TO_DEG(x)   // radians -> degrees
```

### BAM (Binary Angular Measure) conversions

BAM is a `uint16_t` where one full revolution = 65536. Natural `uint16_t` wraparound handles modular arithmetic.

```c
#define QF_BAM_TO_RAD(bam)   // BAM -> radians
#define QF_RAD_TO_BAM(rad)   // radians -> BAM
#define QF_BAM_TO_DEG(bam)   // BAM -> degrees
#define QF_DEG_TO_BAM(deg)   // degrees -> BAM
```

| BAM value | Degrees | Radians |
|-----------|---------|---------|
| 0 | 0 | 0 |
| 16384 | 90 | pi/2 |
| 32768 | 180 | pi |
| 65536 | wraps to 0 | wraps to 0 |

---

## Utility macros

```c
#define QF_ABS(x)             // absolute value
#define QF_SGN(x)             // sign: -1.0, 0.0, or 1.0
#define QF_MIN(a, b)          // minimum
#define QF_MAX(a, b)          // maximum
#define QF_CLAMP(x, lo, hi)   // clamp x to [lo, hi]
#define QF_INTERP(a, b, t)    // linear interpolation, t in [0, 1]
```

---

## Trigonometric functions

### BAM-native

```c
qf qf_sin_bam(uint16_t bam);
qf qf_cos_bam(uint16_t bam);
qf qf_tan_bam(uint16_t bam);
```

Input: BAM phase (0..65535 = one full cycle). Output: `[-1, 1]` for sin/cos; tangent saturates near poles to `QF_TAN_MAX`.

### Radian input

```c
qf qf_sin(qf rad);
qf qf_cos(qf rad);
qf qf_tan(qf rad);
```

### Degree input

```c
qf qf_sin_deg(qf deg);
qf qf_cos_deg(qf deg);
qf qf_tan_deg(qf deg);
```

### Tangent pole clamp

```c
#define QF_TAN_MAX  8388608.0f
```

Exact poles return `QF_TAN_MAX`. Near-pole values clamp to +/- `QF_TAN_MAX`.

---

## Inverse trigonometric functions

```c
qf qf_acos(qf x);   // domain [-1, 1], output [0, pi]
qf qf_asin(qf x);   // domain [-1, 1], output [-pi/2, pi/2]
qf qf_atan(qf x);   // output [-pi/2, pi/2]
qf qf_atan2(qf y, qf x);  // output [-pi, pi]
```

Implementation uses cubic Hermite spans for `asin` on `[0, 3/4]`, `atan`-based fallback for `(3/4, 1)`, and six quadratic spans for `atan` on `[0, 1]` with reciprocal reduction for `x > 1`. See [ALGORITHMS.md](ALGORITHMS.md) for details.

---

## Logarithmic functions

```c
qf qf_log2(qf x);    // base-2 logarithm
qf qf_ln(qf x);      // natural logarithm
qf qf_log10(qf x);   // base-10 logarithm (excluded in lean build)
```

Returns `QF_DOMAIN_ERROR` for `x <= 0`. `ln` and `log10` are derived from `log2` via constant multiplication.

---

## Exponential functions

```c
qf qf_pow2(qf x);    // 2^x
qf qf_exp(qf x);     // e^x
qf qf_pow(qf x, qf y); // x^y, positive x only
qf qf_pow10(qf x);   // 10^x (excluded in lean build)
```

`exp` and `pow10` are derived from `pow2` via base conversion. `pow` is derived from `pow2(y * log2(x))` and returns `QF_DOMAIN_ERROR` for `x <= 0`.

---

## Square root and magnitude

```c
qf qf_sqrt(qf x);                 // sqrt(x), returns QF_DOMAIN_ERROR for x < 0
qf qf_hypot(qf x, qf y);         // sqrt(x^2 + y^2) via qf_sqrt
qf qf_hypot_fast2(qf x, qf y);   // 2-segment piecewise-linear (~1.4% peak error)
qf qf_hypot_fast8(qf x, qf y);   // 8-segment piecewise-linear (~0.10% peak error)
```

`qf_hypot_fast8` uses the same algorithm as `FR_hypot_fast8` (based on US Patent 6,567,777 B1, Chatterjee, expired). No division required.

---

## Wave generators

All take a `uint16_t` BAM phase (`0..65535` = one full cycle) and return `qf` in `[-1, 1]` (except `tri_morph`: `[0, 1]`). Excluded in lean build.

```c
qf qf_wave_sqr(uint16_t phase);
qf qf_wave_pwm(uint16_t phase, uint16_t duty);
qf qf_wave_tri(uint16_t phase);
qf qf_wave_saw(uint16_t phase);
qf qf_wave_tri_morph(uint16_t phase, uint16_t break_point);
qf qf_wave_noise(uint32_t *state);
```

### Phase increment helper

```c
#define QF_HZ2BAM_INC(hz, sample_rate)
```

Compute a per-sample BAM phase increment for a given frequency and sample rate:

```c
uint16_t phase = 0;
uint16_t inc = QF_HZ2BAM_INC(440, 48000);
for (...) { sample = qf_sin_bam(phase); phase += inc; }
```

---

## ADSR envelope generator

Same lifecycle as `fr_math`'s `fr_adsr_t` but with `qf` levels in `[0.0, 1.0]`. Excluded in lean build.

```c
typedef struct qf_adsr_s {
    uint8_t state;
    qf      level;
    qf      sustain;
    qf      attack_inc;
    qf      decay_dec;
    qf      release_dec;
} qf_adsr_t;

void qf_adsr_init(qf_adsr_t *env,
                   uint32_t attack_samples,
                   uint32_t decay_samples,
                   qf sustain_level,
                   uint32_t release_samples);
void qf_adsr_trigger(qf_adsr_t *env);
void qf_adsr_release(qf_adsr_t *env);
qf   qf_adsr_step(qf_adsr_t *env);
```

### State constants

| Constant | Value |
|----------|-------|
| `QF_ADSR_IDLE` | 0 |
| `QF_ADSR_ATTACK` | 1 |
| `QF_ADSR_DECAY` | 2 |
| `QF_ADSR_SUSTAIN` | 3 |
| `QF_ADSR_RELEASE` | 4 |

### Usage

```c
qf_adsr_t env;
qf_adsr_init(&env, 1000, 500, 0.7f, 2000);
qf_adsr_trigger(&env);
for (...) sample *= qf_adsr_step(&env);
qf_adsr_release(&env);
for (...) sample *= qf_adsr_step(&env);
```

---

## Domain error sentinel

```c
#define QF_DOMAIN_ERROR (-1.0e30f)
```

Returned by `qf_sqrt` (negative input), `qf_log2` / `qf_ln` / `qf_log10` (non-positive input). Not a NaN; compare with `==`.

---

## Lean build mode

```c
#define QF_MATH_LEAN
```

Define before including `qf_math.h` (or pass `-DQF_MATH_LEAN` to the compiler) to compile a reduced subset that omits `log10`, `pow10`, wave generators, and ADSR. Use for ROM sizing comparisons in the `compare/` harness.

The compile-time flag `QF_MATH_LEAN_BUILD` is set to `1` when lean mode is active, `0` otherwise.

---

## C++ wrapper (`qf_math.hpp`)

[`src/qf_math.hpp`](../src/qf_math.hpp) is a header-only C++ facade over the C API. It adds no storage, allocation, exceptions, RTTI, or new runtime dependencies.

Everything lives in `namespace qf_math`:

- **Type aliases**: `qf_math::scalar` (`qf`), `qf_math::adsr_t` (`qf_adsr_t`)
- **Constants**: `qf_math::pi`, `qf_math::two_pi`, `qf_math::e`, etc.
- **Free functions**: `qf_math::sin()`, `qf_math::cos()`, `qf_math::sqrt()`, etc. — thin `inline` wrappers
- **ADSR class**: `qf_math::adsr` with `trigger()`, `release()`, `step()` methods

```cpp
#include "qf_math.hpp"

auto y = qf_math::sin(1.0f);
auto len = qf_math::hypot(dx, dy);

qf_math::adsr env(1000, 500, 0.7f, 2000);
env.trigger();
for (...) sample *= env.step();
```

---

## Version macros

```c
#define QF_MATH_VERSION      "1.0.0"
#define QF_MATH_VERSION_HEX   0x010000
```
