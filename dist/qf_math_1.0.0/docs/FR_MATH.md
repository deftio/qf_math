# Relationship to fr_math

**[qf_math](https://github.com/deftio/qf_math)** and **[fr_math](https://github.com/deftio/fr_math)** are sister libraries by the same author. They share algorithmic heritage but target different numeric domains.

---

## Shared algorithms

Both libraries use the same core techniques:

| Technique | qf_math | fr_math |
|-----------|---------|---------|
| Sine table | 512-entry full-cycle, float | 512-entry full-cycle, s15.16 fixed-point |
| Tangent table | 512-entry full-cycle, float | 512-entry full-cycle, s15.16 |
| BAM phase system | `uint16_t`, 65536 = full turn | `uint16_t`, 65536 = full turn |
| Log2 table | 65-entry mantissa, float | 65-entry mantissa, s15.16 |
| Pow2 table | 65-entry fractional, float | 65-entry fractional, s15.16 |
| Inverse trig (atan) | Piecewise polynomial + reciprocal reduction | Piecewise polynomial + reciprocal reduction |
| hypot_fast8 | 8-segment piecewise-linear (float) | 8-segment piecewise-linear (integer shift-only) |
| Wave generators | BAM-phased sqr/pwm/tri/saw/noise | BAM-phased sqr/pwm/tri/saw/noise |
| ADSR envelope | `qf_adsr_t` (float levels) | `fr_adsr_t` (fixed-point levels) |

The table values are mathematically identical; only the numeric representation differs.

---

## Key differences

| Aspect | qf_math | fr_math |
|--------|---------|---------|
| **Numeric domain** | IEEE-754 `float` (float32) | Fixed-point integer (Q16.16 / configurable radix) |
| **Type** | `typedef float qf` | `typedef int32_t s32` |
| **FPU requirement** | Benefits from hardware FPU | No FPU needed; pure integer arithmetic |
| **16-bit support** | Requires 32-bit float | Supports 16-bit platforms (s16 mode) |
| **Precision** | ~7 significant digits (float32) | ~4.8 significant digits (Q16.16) |
| **Dynamic range** | ~1e-38 to ~3.4e+38 | Fixed range determined by radix |
| **Sqrt method** | Newton-Raphson on 1/sqrt(x) via IEEE 754 magic constant | Leading-bit + Newton-Raphson on integer representation |
| **Reciprocal** | IEEE 754 magic constant + Newton-Raphson | Shift-based initial estimate + Newton-Raphson |
| **Language** | C99 | C89 |

---

## When to use which

### Use qf_math when:

- Your target has a **hardware FPU** (Cortex-M4F, Cortex-M7, ESP32, most Cortex-A).
- You already work in **float** throughout your pipeline (sensor fusion, graphics, control loops).
- You need **wide dynamic range** (values spanning many orders of magnitude).
- You want a **lighter alternative to libm** with predictable cost and smaller flash.

### Use fr_math when:

- Your target has **no FPU** (Cortex-M0/M0+, many 8/16-bit MCUs, RISC-V without F extension).
- You want to **avoid soft-float overhead** entirely.
- Your values stay within **fixed ranges** that map well to Q16.16 or similar formats.
- You need **16-bit platform** support.
- Deterministic integer timing matters more than precision.

---

## Bridging between the two

The macros in `qf_math.h` convert between `qf` floats and `fr_math`-style fixed-radix integers:

```c
// Float to fixed-radix (truncating)
int32_t fr_val = QF_TO_FR(1.234f, 16);    // → 80871 (Q16.16)

// Float to fixed-radix (rounding)
int32_t fr_val = QF_TO_FR_RND(1.234f, 16);

// Fixed-radix back to float
qf float_val = FR_TO_QF(80871, 16);       // → ~1.234f
```

Use these at system boundaries: sensor ADC → float pipeline → DAC output, or when mixing qf_math and fr_math code in the same project.

The C++ wrapper provides the same conversions:

```cpp
int32_t fr = qf_math::to_fr(1.234f, 16);
qf back = qf_math::from_fr(fr, 16);
```

---

## Benchmark comparison context

The `compare/` harness benchmarks qf_math against fr_math (among others). When reading those numbers:

- **Float-to-fixed bridge overhead** inflates fr_math timings on float-native hosts. The conversion cost disappears when you stay natively in fixed-point.
- **Host desktop timings** are not representative of MCU behavior. On a Cortex-M0 without FPU, fr_math will be faster. On a Cortex-M4F with FPU, qf_math will be faster.
- The `compare/BENCHMARK_CROSSPLATFORM.md` table merges host and MCU snapshots for side-by-side comparison.

See [`compare/README.md`](../compare/README.md) for methodology and interpretation guidance.
