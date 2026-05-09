<!--
  Regenerate this report with: `make compare-github-report`
  (requires network once for `compare-deps`, then host toolchain only.)
-->
# qf_math — host benchmark snapshot

This page is **machine-generated** so it renders on GitHub without running anything locally. For methodology, rebuilding, and MCU caveats, see [compare/README.md](README.md).

**Domains:** **`qf_math`** is **`float32`**; peers **libfixmath** / **fr_math** in these tables are **fixed-point** (bench uses float wrappers for a shared reference — see README § Float vs fixed-point).

**Also read:** [LIBRARIES.md](LIBRARIES.md) (platforms & representation), [FUNCTION_MATRIX.md](FUNCTION_MATRIX.md) (symbol coverage), [PEERS.md](PEERS.md) (other libraries in this problem space).

---

## Automated measurements

### Host metadata

| Field | Value |
| --- | --- |
| UTC time | 2026-05-09 20:12:24Z |
| uname | Darwin x86_64 25.4.0 |
| Compiler | Apple LLVM 21.0.0 (clang-2100.0.123.102) |
| Pointer size | 64-bit |
| qf_math | 1.0.1 (`QF_MATH_VERSION_HEX`=0x10001) |
| Loop shape | 8000 sample grid × 8000 outer × 64 inner calls |

### Accuracy — peak error

One generated matrix for all benchmarked functions over 8000 sample points. Sine/cosine and tangent rad/deg rows sweep signed `-rotation..+rotation` inputs; BAM rows sweep one unsigned cycle. Tangent pole samples use each implementation family’s saturation magnitude (`QF_TAN_MAX` for qf/float peers, fixed-point range for fixed-point peers), and fixed-point tangent peers are scored against the quantized bridge-domain angle. The `Metric` column defines each peak value: `abs %FS` for sine/cosine output amplitude, `abs` for tangent, `abs rad` for inverse trig, and `rel %` for sqrt/hypot/log/exp/pow. Bold marks the best non-`libm` approximation in each row.

| Function | Metric | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | abs %FS | 0.000000 | **0.001885** | 0.775131 | 0.007791 | --- | --- | --- |
| `sin_deg` | abs %FS | 0.000000 | **0.001885** | 0.775144 | 0.006335 | --- | --- | --- |
| `sin_bam` | abs %FS | 0.000000 | **0.001885** | 0.775155 | 0.004341 | --- | --- | --- |
| `cos_rad` | abs %FS | 0.000000 | **0.001885** | 0.775163 | 0.008684 | --- | --- | --- |
| `cos_deg` | abs %FS | 0.000000 | **0.001883** | 0.775150 | 0.006453 | --- | --- | --- |
| `cos_bam` | abs %FS | 0.000000 | **0.001885** | 0.775151 | 0.004342 | --- | --- | --- |
| `tan_rad` | abs | 0.000000 | 0.300599 | 20.376813 | **0.051472** | --- | --- | --- |
| `tan_deg` | abs | 0.000000 | **0.122804** | 20.376813 | 1.007205 | --- | --- | --- |
| `tan_bam` | abs | 0.000000 | **0.222870** | 20.325424 | 2.452361 | --- | --- | --- |
| `asin` | abs rad | 0.000000 | 0.000466 | 0.010209 | **0.000361** | --- | --- | --- |
| `acos` | abs rad | 0.000000 | 0.000466 | 0.010220 | **0.000357** | --- | --- | --- |
| `atan` | abs rad | 0.000000 | **0.000013** | 0.010158 | 0.000950 | --- | --- | --- |
| `atan2` | abs rad | 0.000000 | 0.001287 | 0.010172 | **0.000944** | --- | --- | --- |
| `sqrt` | rel % | 0.000000 | **0.000472** | 0.380124 | 1.189381 | --- | --- | --- |
| `hypot` | rel % | 0.000000 | 0.000473 | --- | **0.000007** | --- | --- | --- |
| `hypot_fast2` | rel % | 0.000000 | **1.408756** | --- | --- | --- | --- | --- |
| `hypot_fast` | rel % | 0.000000 | **0.137249** | --- | 0.137249 | --- | --- | --- |
| `log2` | rel % | 0.000000 | **0.001625** | 0.280398 | 0.316523 | --- | --- | --- |
| `ln` | rel % | 0.000000 | **0.001626** | 0.086993 | 0.398790 | --- | --- | --- |
| `log10` | rel % | 0.000000 | **0.001632** | 0.511634 | 0.511634 | --- | --- | --- |
| `pow2` | rel % | 0.000000 | **0.000327** | 0.047725 | 0.092220 | --- | --- | --- |
| `exp` | rel % | 0.000000 | **0.000351** | 0.286388 | 0.574226 | --- | --- | --- |
| `pow10` | rel % | 0.000000 | **0.000371** | 303710.475412 | 199.601630 | --- | --- | --- |
| `pow` | rel % | 0.000000 | **0.000387** | 0.045078 | 0.086622 | --- | --- | --- |

### Accuracy — mean squared error

Mean squared error uses the same metric units as the peak-error table, squared. Bold marks the best non-`libm` approximation in each row.

| Function | Metric squared | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | abs %FS^2 | 0.000000 | **0.000001** | 0.022074 | 0.000006 | --- | --- | --- |
| `sin_deg` | abs %FS^2 | 0.000000 | **0.000001** | 0.022073 | 0.000005 | --- | --- | --- |
| `sin_bam` | abs %FS^2 | 0.000000 | **0.000001** | 0.022069 | 0.000002 | --- | --- | --- |
| `cos_rad` | abs %FS^2 | 0.000000 | **0.000001** | 0.022132 | 0.000006 | --- | --- | --- |
| `cos_deg` | abs %FS^2 | 0.000000 | **0.000001** | 0.022132 | 0.000006 | --- | --- | --- |
| `cos_bam` | abs %FS^2 | 0.000000 | **0.000001** | 0.022094 | 0.000002 | --- | --- | --- |
| `tan_rad` | abs^2 | 0.000000 | 0.000026 | 0.753949 | **0.000008** | --- | --- | --- |
| `tan_deg` | abs^2 | 0.000000 | **0.000005** | 0.753912 | 0.001138 | --- | --- | --- |
| `tan_bam` | abs^2 | 0.000000 | **0.000013** | 0.704843 | 0.004254 | --- | --- | --- |
| `asin` | abs rad^2 | 0.000000 | 0.000000 | 0.000023 | **0.000000** | --- | --- | --- |
| `acos` | abs rad^2 | 0.000000 | 0.000000 | 0.000023 | **0.000000** | --- | --- | --- |
| `atan` | abs rad^2 | 0.000000 | **0.000000** | 0.000047 | 0.000000 | --- | --- | --- |
| `atan2` | abs rad^2 | 0.000000 | 0.000000 | 0.000021 | **0.000000** | --- | --- | --- |
| `sqrt` | rel %^2 | 0.000000 | **0.000000** | 0.000018 | 0.000177 | --- | --- | --- |
| `hypot` | rel %^2 | 0.000000 | 0.000000 | --- | **0.000000** | --- | --- | --- |
| `hypot_fast2` | rel %^2 | 0.000000 | **0.519433** | --- | --- | --- | --- | --- |
| `hypot_fast` | rel %^2 | 0.000000 | 0.003018 | --- | **0.003018** | --- | --- | --- |
| `log2` | rel %^2 | 0.000000 | **0.000000** | 0.000011 | 0.000033 | --- | --- | --- |
| `ln` | rel %^2 | 0.000000 | **0.000000** | 0.000001 | 0.000043 | --- | --- | --- |
| `log10` | rel %^2 | 0.000000 | **0.000000** | 0.000040 | 0.000048 | --- | --- | --- |
| `pow2` | rel %^2 | 0.000000 | **0.000000** | 0.000048 | 0.000174 | --- | --- | --- |
| `exp` | rel %^2 | 0.000000 | **0.000000** | 0.001315 | 0.005274 | --- | --- | --- |
| `pow10` | rel %^2 | 0.000000 | **0.000000** | 12147284.783673 | 2538.588877 | --- | --- | --- |
| `pow` | rel %^2 | 0.000000 | **0.000000** | 0.000103 | 0.000338 | --- | --- | --- |

### Wall-clock time (microseconds)

Total microseconds normalized to the metadata loop shape. ESP32-only peer rows may be measured with a shorter loop and scaled to the same outer-iteration count. Unsupported cells are `---`.

| Function | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | 1977.917000 | **1246.500000** | 14721.125000 | 5494.375000 | --- | --- | --- |
| `sin_deg` | 1947.459000 | **1153.375000** | 15379.875000 | 6280.375000 | --- | --- | --- |
| `sin_bam` | 2441.083000 | **1484.041000** | 15127.333000 | 2084.792000 | --- | --- | --- |
| `cos_rad` | 2086.583000 | **1177.667000** | 14309.583000 | 5841.208000 | --- | --- | --- |
| `cos_deg` | 2025.708000 | **1134.584000** | 15512.542000 | 6465.916000 | --- | --- | --- |
| `cos_bam` | 2548.042000 | **1550.833000** | 15299.292000 | 2264.833000 | --- | --- | --- |
| `tan_rad` | 6650.625000 | **1389.625000** | 31092.584000 | 6240.125000 | --- | --- | --- |
| `tan_deg` | 6619.917000 | **1479.375000** | 31509.875000 | 8422.375000 | --- | --- | --- |
| `tan_bam` | 6536.459000 | **1662.041000** | 31275.708000 | 2227.042000 | --- | --- | --- |
| `asin` | **1873.042000** | 8516.292000 | 38811.750000 | 8634.750000 | --- | --- | --- |
| `acos` | **1872.917000** | 8780.708000 | 38931.459000 | 7847.000000 | --- | --- | --- |
| `atan` | **1861.625000** | 2527.791000 | 15025.084000 | 11711.333000 | --- | --- | --- |
| `atan2` | **1847.708000** | 2355.458000 | 14863.708000 | 13656.791000 | --- | --- | --- |
| `sqrt` | **319.125000** | 1195.500000 | 17928.666000 | 10859.541000 | --- | --- | --- |
| `hypot` | 1833.750000 | **1199.209000** | --- | 17393.000000 | --- | --- | --- |
| `hypot_fast2` | 1871.208000 | **1118.041000** | --- | --- | --- | --- | --- |
| `hypot_fast` | 1900.292000 | **1458.166000** | --- | 3028.083000 | --- | --- | --- |
| `log2` | **1856.709000** | 1919.375000 | 55013.750000 | 6803.958000 | --- | --- | --- |
| `ln` | **2024.167000** | 2841.625000 | 582256.125000 | 6438.834000 | --- | --- | --- |
| `log10` | **1959.125000** | 2552.042000 | 54853.833000 | 6986.875000 | --- | --- | --- |
| `pow2` | **1629.166000** | 1943.834000 | 120106.959000 | 3057.042000 | --- | --- | --- |
| `exp` | **2441.750000** | 2499.750000 | 118156.417000 | 2577.666000 | --- | --- | --- |
| `pow10` | **1712.167000** | 2085.000000 | 4759.084000 | 3305.792000 | --- | --- | --- |
| `pow` | **4611.792000** | 5613.708000 | 622476.375000 | 8814.208000 | --- | --- | --- |

### Speed vs libm (ratio)

`libm` time ÷ implementation time for the same function on this platform. Above 1.0 means faster than the platform `libm` call in this microbenchmark. Ratios are rounded to two decimal places because smaller differences are usually noise.

**Fixed-point peer rows are bridged harness timings**: `float`→fixed→function→`float`. They keep the report comparable, but are not native `fix16_t` / `s32` pipeline timings.

| Function | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | 1.00 | **1.59** | 0.13 | 0.36 | --- | --- | --- |
| `sin_deg` | 1.00 | **1.69** | 0.13 | 0.31 | --- | --- | --- |
| `sin_bam` | 1.00 | **1.64** | 0.16 | 1.17 | --- | --- | --- |
| `cos_rad` | 1.00 | **1.77** | 0.15 | 0.36 | --- | --- | --- |
| `cos_deg` | 1.00 | **1.79** | 0.13 | 0.31 | --- | --- | --- |
| `cos_bam` | 1.00 | **1.64** | 0.17 | 1.13 | --- | --- | --- |
| `tan_rad` | 1.00 | **4.79** | 0.21 | 1.07 | --- | --- | --- |
| `tan_deg` | 1.00 | **4.47** | 0.21 | 0.79 | --- | --- | --- |
| `tan_bam` | 1.00 | **3.93** | 0.21 | 2.94 | --- | --- | --- |
| `asin` | **1.00** | 0.22 | 0.05 | 0.22 | --- | --- | --- |
| `acos` | **1.00** | 0.21 | 0.05 | 0.24 | --- | --- | --- |
| `atan` | **1.00** | 0.74 | 0.12 | 0.16 | --- | --- | --- |
| `atan2` | **1.00** | 0.78 | 0.12 | 0.14 | --- | --- | --- |
| `sqrt` | **1.00** | 0.27 | 0.02 | 0.03 | --- | --- | --- |
| `hypot` | 1.00 | **1.53** | --- | 0.11 | --- | --- | --- |
| `hypot_fast2` | 1.00 | **1.67** | --- | --- | --- | --- | --- |
| `hypot_fast` | 1.00 | **1.30** | --- | 0.63 | --- | --- | --- |
| `log2` | **1.00** | 0.97 | 0.03 | 0.27 | --- | --- | --- |
| `ln` | **1.00** | 0.71 | 0.00 | 0.31 | --- | --- | --- |
| `log10` | **1.00** | 0.77 | 0.04 | 0.28 | --- | --- | --- |
| `pow2` | **1.00** | 0.84 | 0.01 | 0.53 | --- | --- | --- |
| `exp` | **1.00** | 0.98 | 0.02 | 0.95 | --- | --- | --- |
| `pow10` | **1.00** | 0.82 | 0.36 | 0.52 | --- | --- | --- |
| `pow` | **1.00** | 0.82 | 0.01 | 0.52 | --- | --- | --- |


### Library footprint (ROM-sized `.o` totals, decimal bytes)

Each **row is one compiled library/object variant** you might ship on its own (or as your SDK’s math layer). **Do not add the rows together** — real firmware picks **one** primary approximate math approach (plus vendor libm snippets). The benchmark binary links several libraries only to score them in one harness, not because products bundle them all. **qf_math** is **float**; **libfixmath** / **fr_math** are **fixed-point** (different animals — see **README.md** § Float vs fixed-point).

**Where these numbers come from** — see [compare/README.md](README.md) § *Footprint rows (what is actually measured)*.

| Library | Variant | Bytes (dec) | What is counted |
| :--- | :--- | ---:|:---|
| **qf_math** | full | 13064 | Single TU `qf_math.o` at `-Os` (same as `make lib`). |
| **qf_math** | lean | 9124 | `qf_math.o` with `-DQF_MATH_LEAN`: radian trig, inverse trig, `log2`/`ln`, `pow2`/`exp`/`pow`, `sqrt`, and `hypot_fast8` — no degree/BAM trig entry points, exact `hypot`, `hypot_fast2`, `log10`/`pow10`, waves, or ADSR (see `qf_math.h`). |
| **libfixmath** | bench subset | 4827 | Sum of the `lf_*.o` objects from **Makefile** `LIBFIXMATH_SRCS` (trig + sqrt + exp + fix16 core) — only what the compare harness links. |
| **fr_math** | full | 11292 | `FR_math.c` at `-Os` with default feature set. |
| **fr_math** | lean | 5872 | `FR_math.c` with `-DFR_LEAN -DFR_NO_PRINT`: radian trig, inverse trig, log/exp, sqrt; omits degree/BAM wrappers, hypot, waves/ADSR, and print helpers. |
| **fr_math** | bench no-print | 9560 | `FR_math.c` with `-DFR_NO_PRINT`, `-Os`; this is the object linked by the portable compare harness. |
| **FastTrig** | ESP32 peer object | 2562 | PlatformIO-built `FastTrig.cpp.o` when the LilyGO benchmark has been built; `---` if unavailable on this machine. |
| **ESP-DSP** | local scalar subset | --- | No standalone library object in this harness; the sqrt approximation is a tiny local wrapper in `main.cpp`. |
| **espp/math** | local scalar subset | --- | Header-only/local scalar subset in the LilyGO benchmark, not a separately sized library object. |

> **Note:** There is no single **`libm`** row — vendor math lives in prebuilt toolchain archives; the perf tables time `sinf`/`cosf`/…, not a standalone `.o` total.

<details><summary>libfixmath object breakdown (bench link only)</summary>

| Object file | Bytes |
| :--- | ---:|
| `lf_fix16_exp.o` | 986 |
| `lf_fix16_sqrt.o` | 280 |
| `lf_fix16_trig.o` | 1378 |
| `lf_fix16.o` | 1590 |
| `lf_fract32.o` | 402 |
| `lf_uint32.o` | 191 |

</details>

---

## Interpretation

- **Timings** are from a **desktop-class host** (`sinf`/`logf`/… reference). On Cortex-M (with or without FPU), soft-float costs, flash wait states, and IRQ latency dominate—**re-profile on your MCU**.
- **`qf_math`** timings reflect **`float`** paths; **libfixmath** / **fr_math** are **fixed-point** libraries measured here **via float↔fixed bridges** for uniform double-reference scoring — not “three floats racing.” Native fixed-only call paths drop most conversion overhead.
- **“vs libm”** is `t_libm / t_impl` for the measured loop (values **greater than 1** mean the implementation was faster than host libm in this microbenchmark).
- **Footprint table**: each row is **one library’s object code** — you normally ship **one** math stack; the harness links several **only** for side‑by‑side measurement (don’t add the byte counts together as “total firmware”).
