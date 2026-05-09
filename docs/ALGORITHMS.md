# qf_math Algorithms

How the numerical routines work under the hood.


## Table-driven trigonometry

### Sine and cosine

A 512-entry full-cycle sine table (`gSIN_TAB`, 513 entries including sentinel) stores `sin(2*pi*k/512)` for `k = 0..512` as native `float` values. Cosine reuses the same table with a 90-degree (128-entry) phase offset.

Lookup procedure:

1. Convert input (radians, degrees, or BAM) to a fixed-point BAM phase with 7 extra fractional bits (total 23 bits of phase).
2. Upper 9 bits select the table index; lower 14 bits form the fractional sub-step.
3. Linear interpolation between adjacent table entries:
   ```
   out = lo + (hi - lo) * frac
   ```

The 7-bit BAM sub-step fractional system gives 128 interpolation steps between each pair of table entries, yielding effective resolution of 65536 steps per cycle (same as a 16-bit BAM).

### Tangent

A separate 512-entry full-cycle tangent table (`gTAN_TAB`) with the same indexing scheme. Near poles (within ~3000 BAM of pi/2 or 3*pi/2), the table is bypassed and a reciprocal-based pole approximation is used:

```
raw = 1/angle - angle/3
```

This avoids table entries near singularities where linear interpolation would produce large errors. The result is clamped to `QF_TAN_MAX` (8388608.0).

### Worst-case error

Sin/cos: ~0.01% of amplitude (vs double reference).
Tangent: ~1.7% of amplitude, excluding near-pole regions.


## BAM (Binary Angular Measure) phase system

Angles are represented internally as `uint16_t` BAM values where one full revolution = 65536. This gives several advantages for embedded systems:

- Natural wraparound: `uint16_t` overflow handles modular arithmetic for free.
- Integer-only phase accumulation for wave generators (no floating-point drift).
- Direct table indexing without division.

Conversion constants:
- `QF_BAM_PER_RAD` = 65536 / (2*pi) = 10430.378
- `QF_BAM_PER_DEG` = 65536 / 360 = 182.044


## Inverse trigonometry

### asin / acos

The shared kernel `qf_asin_pos_kernel(ax)` computes `asin(|x|)` for `x` in `[0, 1]` using a multi-strategy approach:

1. **Small inputs** (`|x| < 0.084`): return `x` (asin(x) ~ x for small x).
2. **Three cubic Hermite spans** on `[0, 3/4]` with knots at 0, 1/4, 1/2, 3/4. Each span uses C1-continuous Hermite basis functions (`h00`, `h10`, `h01`, `h11`) with precomputed endpoint values and derivatives.
3. **atan fallback** for `(3/4, 1)`: `asin(x) = atan(x / sqrt(1 - x^2))` using the identity `1 - x^2 = (1-x)(1+x)` for numerical stability near 1. This avoids cubic Hermite overshoot on a span where `asin'(u)` grows unboundedly as `u -> 1`.
4. **Tail approximation** for `x > 0.9975`: `asin(x) ~ pi/2 - sqrt(2(1-x))`.

`acos(x)` folds through `asin`: `acos(x) = pi/2 - asin(|x|)`, with sign adjustment for negative inputs.

### atan / atan2

`atan(x)` on `[0, 1]` uses six quadratic polynomial spans:

1. Multiply `x * 6` to select the span (each covers 1/6 of the unit interval).
2. Evaluate `(c2 * t + c1) * t + c0` with precomputed coefficients for each span.
3. For `x > 1`: `atan(x) = pi/2 - atan(1/x)` using fast reciprocal.

`atan2(y, x)` uses ratio swaps and quadrant folds:

1. Compute `q1_angle = atan(min/max)` in the first quadrant.
2. If `|y| > |x|`: `q1_angle = pi/2 - atan(|x|/|y|)`.
3. Map to correct quadrant based on signs of `x` and `y`.


## Logarithms

### log2

Uses IEEE 754 float structure:

1. Extract the unbiased exponent field: `exp_raw = (bits >> 23) & 0xFF - 127` (same as the integer part of `log2` for normals).
2. **Normal finite** (`exp` bits not zero, not Infinity): fractional mantissa `M` yields `t = M / 2^23`; `log2(m) = log2(1+t)` approximated by a degree-7 Horner polynomial in `t` (no constant term, exact at `t = 0`).
3. **Subnormals, NaN, Inf**, and edge patterns: reconstruct `m` in `[1, 2)` in float (implicit leading one), same as the legacy path, then evaluate the same polynomial on `m - 1`.
4. Result: `exp_raw + log2(m)`.

### ln, log10

Derived from `log2` by constant multiplication:
- `ln(x) = log2(x) * ln(2)`
- `log10(x) = log2(x) * log10(2)`


## Exponentials

### pow2

1. **Nearest-int split**: `x + (3·2^22)` exploit → integer `n` and remainder `f` with `|f| ≤ 0.5`; if unreliable, floor split and map `frac ∈ [0,1)` onto the polynomial range.
2. **Degree-5 Horner** approximates `2^f` on `[-0.5, 0.5]`.
3. **Scale**: add `n` to the IEEE exponent of the polynomial result when the sum stays in `[1, 254]`; otherwise `poly * make_pow2i(n)` (same caps as legacy `make_pow2i`).

### exp, pow10

Derived from `pow2` by base conversion:
- `exp(x) = pow2(x * log2(e))`
- `pow10(x) = pow2(x * log2(10))`


## Square root

Newton-Raphson iterations on the inverse square root:

1. **Initial estimate** via the classic fast inverse square root technique (Quake III style):
   ```c
   conv.u = 0x5F375A86u - (conv.u >> 1);
   ```
   The magic constant `0x5F375A86` provides a good initial `1/sqrt(x)` approximation by exploiting IEEE 754 float layout (the exponent field is roughly halved).

2. **Two Newton-Raphson iterations** for `1/sqrt(x)`:
   ```c
   y = y * (1.5 - 0.5 * x * y * y);
   ```

3. **Final multiply**: `sqrt(x) = x * (1/sqrt(x))`.

Worst-case relative error: ~0.0005%.


## Reciprocal

Newton-Raphson iterations with a magic constant initial estimate:

```c
v.u = 0x7EF311C3u - v.u;   // initial 1/x estimate
y = y * (2 - x * y);       // Newton-Raphson refinement
```

Three iterations for general use (`qf_inv_pos`), two iterations where reduced precision is acceptable (`qf_inv_pos_atan`).


## Hypot variants

### qf_hypot (exact)

Direct computation: `sqrt(x*x + y*y)` via `qf_sqrt`.

### qf_hypot_fast2 (2-segment piecewise)

Classifies `lo/hi` ratio into two regions and applies linear combination:
- `lo > hi/2`: `0.814*hi + 0.592*lo`
- Otherwise: `0.986*hi + 0.236*lo`

Peak error: ~1.4%.

### qf_hypot_fast8 (8-segment piecewise)

Based on US Patent 6,567,777 B1 (Chatterjee, expired).

After sorting `|x|, |y|` into `hi, lo`, the ratio `beta = lo/hi` is classified into 8 segments using only comparisons against `hi` scaled by powers of two (no division):

| Segment | beta range | a coefficient | b coefficient |
|---------|-----------|---------------|---------------|
| 1 | (0.875, 1] | 0.73046875 | 0.68359375 |
| 2 | (0.75, 0.875] | 0.78027344 | 0.62622070 |
| 3 | (0.625, 0.75] | 0.828125 | 0.56298828 |
| 4 | (0.5, 0.625] | 0.87280273 | 0.48925781 |
| 5 | (0.375, 0.5] | 0.91796875 | 0.3984375 |
| 6 | (0.25, 0.375] | 0.95507813 | 0.29882813 |
| 7 | (0.125, 0.25] | 0.98388672 | 0.18383789 |
| 8 | [0, 0.125] | 0.99902344 | 0.06201172 |

Result: `a * hi + b * lo`. All coefficients are derived from shift-only expressions (sums of powers of two), making them exact in the binary representation.

Peak error: ~0.10%.


## Error budgets and worst-case accuracy

| Function | Error metric | Worst case |
|----------|-------------|------------|
| sin/cos | % of amplitude | ~0.01% |
| tan | % of amplitude (excl. near-pole) | ~1.7% |
| acos | % of pi (radians) | ~0.008% |
| atan2 | % of pi (radians) | ~0.03% |
| log2/ln | absolute delta vs double | ~4.4e-05 |
| pow2/exp | relative vs double | ~0.0015% |
| sqrt | relative vs double | ~0.0005% |
| hypot | relative vs double | ~0.0005% |
| hypot_fast8 | relative vs double | ~0.14% |

These figures are from host benchmarks (see `make bench` or the [Pages site](../pages/index.html)). MCU results vary; always profile on your target silicon.
