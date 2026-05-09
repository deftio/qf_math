# Quick Start

## Include and compile

Copy `src/qf_math.c` and `src/qf_math.h` into the project. Compile `qf_math.c` as a single translation unit alongside application code. No external dependencies — no `-lm` required.

```bash
gcc -std=c99 -Os main.c qf_math.c -o main
```

```c
#include "qf_math.h"
```

All functions take and return `qf` (a `typedef` for `float`).


## Trigonometry

Three input conventions: **radians**, **degrees**, and **BAM** (Binary Angular Measure, where `uint16_t` 65536 = one full revolution).

```c
/* Radians */
qf y = qf_sin(1.0f);           /* 0.8415 */
qf x = qf_cos(1.0f);           /* 0.5403 */
qf t = qf_tan(0.5f);           /* 0.5463 */

/* Degrees */
qf s45 = qf_sin_deg(45.0f);    /* 0.7071 */
qf c60 = qf_cos_deg(60.0f);    /* 0.5000 */

/* BAM (16384 = 90 deg) */
qf s90 = qf_sin_bam(16384);    /* 1.0000 (exact) */
```

### Inverse trig

```c
qf angle = qf_asin(0.5f);      /* 0.5236 rad (30 deg) */
qf angle = qf_acos(0.5f);      /* 1.0472 rad (60 deg) */
qf angle = qf_atan2(3.0f, 4.0f); /* 0.6435 rad */
```


## Log, exp, pow

```c
qf l2  = qf_log2(8.0f);        /* 3.0 */
qf l   = qf_ln(2.71828f);      /* 1.0 */
qf l10 = qf_log10(1000.0f);    /* 3.0 */

qf p2  = qf_pow2(3.0f);        /* 8.0 */
qf e1  = qf_exp(1.0f);         /* 2.7183 */
qf p   = qf_pow(2.0f, 10.0f);  /* 1024.0 */
```


## Sqrt and hypot

```c
qf r  = qf_sqrt(2.0f);              /* 1.4142 */
qf h  = qf_hypot(3.0f, 4.0f);       /* 5.0000 */
qf hf = qf_hypot_fast8(3.0f, 4.0f); /* 5.0015 (0.1% worst-case error, faster) */
```


## Utility macros

```c
qf rad  = QF_DEG_TO_RAD(180.0f);             /* 3.1416 */
qf deg  = QF_RAD_TO_DEG(QF_PI);              /* 180.0 */
qf c    = QF_CLAMP(5.0f, 0.0f, 3.0f);        /* 3.0 */
qf mid  = QF_INTERP(0.0f, 10.0f, 0.25f);     /* 2.5 */
int32_t a = QF_ABS(-7.0f);                    /* 7.0 */
```


## Waveform generators

All waveforms use `uint16_t` BAM phase (0–65535 = one cycle) and return values in [-1, +1].

```c
qf sq = qf_wave_sqr(8192);            /* +1.0 (first half) */
qf tr = qf_wave_tri(16384);           /* +1.0 (peak) */
qf sw = qf_wave_saw(32768);           /*  0.0 (midpoint) */

uint32_t noise_state = 12345;
qf n  = qf_wave_noise(&noise_state);  /* pseudo-random in [-1, 1] */
```

Phase increment for a given frequency:

```c
uint16_t inc = QF_HZ2BAM_INC(440, 48000);  /* 440 Hz at 48 kHz sample rate */
```


## ADSR envelope

```c
qf_adsr_t env;
qf_adsr_init(&env, 1000, 500, 0.7f, 2000);  /* attack, decay, sustain, release (samples) */
qf_adsr_trigger(&env);

for (int i = 0; i < num_samples; i++) {
    sample[i] *= qf_adsr_step(&env);
}

qf_adsr_release(&env);  /* begin release phase */
```


## Fixed-radix bridge (qf_math ↔ fr_math)

Convert between `float` and fixed-radix integers at system boundaries:

```c
int32_t fr = QF_TO_FR(1.234f, 16);      /* float → Q16.16 (truncating) */
int32_t fr = QF_TO_FR_RND(1.234f, 16);  /* float → Q16.16 (rounding) */
qf back    = FR_TO_QF(80871, 16);        /* Q16.16 → float */
```


## Domain errors

Functions with restricted domains (`qf_sqrt` of negative, `qf_log2` of non-positive) return `QF_DOMAIN_ERROR` (-1e30f).

```c
qf bad = qf_sqrt(-1.0f);
if (bad == QF_DOMAIN_ERROR) { /* handle error */ }
```


## Runnable example

A complete working program is in [`examples/quickstart/`](../examples/quickstart/). Build with `make` and run `./quickstart`.

## Further reading

- [API reference](API.md) — every public function, macro, and constant
- [Algorithms](ALGORITHMS.md) — how the approximations work
- [Integration guide](INTEGRATION.md) — CMake, PlatformIO, ESP-IDF, Arduino
