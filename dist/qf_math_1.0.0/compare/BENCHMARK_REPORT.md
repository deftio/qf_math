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
| UTC time | 2026-05-09 08:43:47Z |
| uname | Darwin arm64 25.4.0 |
| Compiler | Apple LLVM 21.0.0 (clang-2100.0.123.102) |
| Pointer size | 64-bit |
| qf_math | 1.0.0 (`QF_MATH_VERSION_HEX`=0x10000) |
| Loop shape | 8000 sample grid × 8000 outer × 64 inner calls |

### Accuracy — peak error

One generated matrix for all benchmarked functions over 8000 sample points. Sine/cosine and tangent rad/deg rows sweep signed `-rotation..+rotation` inputs; BAM rows sweep one unsigned cycle. Tangent pole samples use each implementation family’s saturation magnitude (`QF_TAN_MAX` for qf/float peers, fixed-point range for fixed-point peers), and fixed-point tangent peers are scored against the quantized bridge-domain angle. The `Metric` column defines each peak value: `abs %FS` for sine/cosine output amplitude, `abs` for tangent, `abs rad` for inverse trig, and `rel %` for sqrt/hypot/log/exp/pow. Bold marks the best non-`libm` approximation in each row.

| Function | Metric | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | abs %FS | 0.000000 | **0.001884** | 0.775155 | 0.007801 | --- | --- | --- |
| `sin_deg` | abs %FS | 0.000000 | **0.001883** | 0.775144 | 0.006335 | --- | --- | --- |
| `sin_bam` | abs %FS | 0.000000 | **0.001885** | 0.775155 | 0.004341 | --- | --- | --- |
| `cos_rad` | abs %FS | 0.000000 | **0.001886** | 0.775151 | 0.008672 | --- | --- | --- |
| `cos_deg` | abs %FS | 0.000000 | **0.001883** | 0.775150 | 0.006432 | --- | --- | --- |
| `cos_bam` | abs %FS | 0.000000 | **0.001885** | 0.775151 | 0.004342 | --- | --- | --- |
| `tan_rad` | abs | 0.000000 | 0.300599 | 20.376813 | **0.051472** | --- | --- | --- |
| `tan_deg` | abs | 0.000000 | **0.037535** | 20.376813 | 1.007205 | --- | --- | --- |
| `tan_bam` | abs | 0.000000 | **0.222870** | 20.325424 | 2.452361 | --- | --- | --- |
| `asin` | abs rad | 0.000000 | 0.000466 | 0.010209 | **0.000361** | --- | --- | --- |
| `acos` | abs rad | 0.000000 | 0.000466 | 0.010220 | **0.000357** | --- | --- | --- |
| `atan` | abs rad | 0.000000 | **0.000013** | 0.010158 | 0.000950 | --- | --- | --- |
| `atan2` | abs rad | 0.000000 | 0.001287 | 0.010172 | **0.000944** | --- | --- | --- |
| `sqrt` | rel % | 0.000000 | **0.000472** | 0.380124 | 1.189381 | --- | --- | --- |
| `hypot` | rel % | 0.000000 | 0.000470 | --- | **0.000007** | --- | --- | --- |
| `hypot_fast2` | rel % | 0.000000 | **1.408747** | --- | --- | --- | --- | --- |
| `hypot_fast` | rel % | 0.000000 | **0.137249** | --- | 0.137249 | --- | --- | --- |
| `log2` | rel % | 0.000000 | **0.001625** | 0.280398 | 0.316523 | --- | --- | --- |
| `ln` | rel % | 0.000000 | **0.001626** | 0.086993 | 0.398790 | --- | --- | --- |
| `log10` | rel % | 0.000000 | **0.001632** | 0.511634 | 0.511634 | --- | --- | --- |
| `pow2` | rel % | 0.000000 | **0.000327** | 0.047725 | 0.092220 | --- | --- | --- |
| `exp` | rel % | 0.000000 | **0.000344** | 0.286388 | 0.574226 | --- | --- | --- |
| `pow10` | rel % | 0.000000 | **0.000371** | 303710.475412 | 199.601630 | --- | --- | --- |
| `pow` | rel % | 0.000000 | **0.000379** | 0.045078 | 0.086622 | --- | --- | --- |

### Accuracy — mean squared error

Mean squared error uses the same metric units as the peak-error table, squared. Bold marks the best non-`libm` approximation in each row.

| Function | Metric squared | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | abs %FS^2 | 0.000000 | **0.000001** | 0.022074 | 0.000006 | --- | --- | --- |
| `sin_deg` | abs %FS^2 | 0.000000 | **0.000001** | 0.022074 | 0.000005 | --- | --- | --- |
| `sin_bam` | abs %FS^2 | 0.000000 | **0.000001** | 0.022069 | 0.000002 | --- | --- | --- |
| `cos_rad` | abs %FS^2 | 0.000000 | **0.000001** | 0.022133 | 0.000006 | --- | --- | --- |
| `cos_deg` | abs %FS^2 | 0.000000 | **0.000001** | 0.022132 | 0.000006 | --- | --- | --- |
| `cos_bam` | abs %FS^2 | 0.000000 | **0.000001** | 0.022094 | 0.000002 | --- | --- | --- |
| `tan_rad` | abs^2 | 0.000000 | 0.000027 | 0.753949 | **0.000008** | --- | --- | --- |
| `tan_deg` | abs^2 | 0.000000 | **0.000001** | 0.753950 | 0.001127 | --- | --- | --- |
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
| `pow10` | rel %^2 | 0.000000 | **0.000000** | 12147284.785276 | 2538.590406 | --- | --- | --- |
| `pow` | rel %^2 | 0.000000 | **0.000000** | 0.000103 | 0.000338 | --- | --- | --- |

### Wall-clock time (microseconds)

Total microseconds normalized to the metadata loop shape. ESP32-only peer rows may be measured with a shorter loop and scaled to the same outer-iteration count. Unsupported cells are `---`.

| Function | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | 3298.791667 | **1587.625000** | 9329.541667 | 4035.625000 | --- | --- | --- |
| `sin_deg` | 1194.333333 | **585.000000** | 7431.416667 | 4264.666667 | --- | --- | --- |
| `sin_bam` | 1511.500000 | **949.750000** | 9512.125000 | 1514.541667 | --- | --- | --- |
| `cos_rad` | 1522.333333 | **691.333333** | 7079.791667 | 5429.458333 | --- | --- | --- |
| `cos_deg` | 1295.041667 | **749.458333** | 7214.625000 | 4334.541667 | --- | --- | --- |
| `cos_bam` | 1251.125000 | **715.458333** | 7263.833333 | 1318.875000 | --- | --- | --- |
| `tan_rad` | 1468.541667 | **854.333333** | 13461.500000 | 4385.791667 | --- | --- | --- |
| `tan_deg` | 1482.250000 | **835.541667** | 14203.250000 | 11779.041667 | --- | --- | --- |
| `tan_bam` | 1426.000000 | **1069.125000** | 17556.416667 | 1613.916667 | --- | --- | --- |
| `asin` | **1066.291667** | 5055.666667 | 23774.958333 | 6039.000000 | --- | --- | --- |
| `acos` | **1161.958333** | 4808.291667 | 23779.458333 | 5581.541667 | --- | --- | --- |
| `atan` | **1242.166667** | 1252.375000 | 5637.625000 | 8487.500000 | --- | --- | --- |
| `atan2` | **1177.083333** | 1488.500000 | 6081.166667 | 9119.041667 | --- | --- | --- |
| `sqrt` | **317.416667** | 830.875000 | 11192.083333 | 9462.583333 | --- | --- | --- |
| `hypot` | 833.250000 | **832.000000** | --- | 15600.791667 | --- | --- | --- |
| `hypot_fast2` | **833.083333** | 833.500000 | --- | --- | --- | --- | --- |
| `hypot_fast` | **833.041667** | 1161.041667 | --- | 1359.750000 | --- | --- | --- |
| `log2` | 1151.916667 | **996.208333** | 24372.666667 | 3882.416667 | --- | --- | --- |
| `ln` | **1147.875000** | 1329.375000 | 339461.708333 | 3872.083333 | --- | --- | --- |
| `log10` | **1151.250000** | 1319.333333 | 29406.833333 | 4597.833333 | --- | --- | --- |
| `pow2` | **777.750000** | 1356.583333 | 66019.916667 | 1652.291667 | --- | --- | --- |
| `exp` | **845.500000** | 1389.041667 | 72677.500000 | 1690.708333 | --- | --- | --- |
| `pow10` | **850.583333** | 1386.791667 | 1782.291667 | 1796.958333 | --- | --- | --- |
| `pow` | **1807.083333** | 2657.625000 | 377902.500000 | 5457.083333 | --- | --- | --- |

### Speed vs libm (ratio)

`libm` time ÷ implementation time for the same function on this platform. Above 1.0 means faster than the platform `libm` call in this microbenchmark. Ratios are rounded to two decimal places because smaller differences are usually noise.

**Fixed-point peer rows are bridged harness timings**: `float`→fixed→function→`float`. They keep the report comparable, but are not native `fix16_t` / `s32` pipeline timings.

| Function | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | 1.00 | **2.08** | 0.35 | 0.82 | --- | --- | --- |
| `sin_deg` | 1.00 | **2.04** | 0.16 | 0.28 | --- | --- | --- |
| `sin_bam` | 1.00 | **1.59** | 0.16 | 1.00 | --- | --- | --- |
| `cos_rad` | 1.00 | **2.20** | 0.22 | 0.28 | --- | --- | --- |
| `cos_deg` | 1.00 | **1.73** | 0.18 | 0.30 | --- | --- | --- |
| `cos_bam` | 1.00 | **1.75** | 0.17 | 0.95 | --- | --- | --- |
| `tan_rad` | 1.00 | **1.72** | 0.11 | 0.33 | --- | --- | --- |
| `tan_deg` | 1.00 | **1.77** | 0.10 | 0.13 | --- | --- | --- |
| `tan_bam` | 1.00 | **1.33** | 0.08 | 0.88 | --- | --- | --- |
| `asin` | **1.00** | 0.21 | 0.04 | 0.18 | --- | --- | --- |
| `acos` | **1.00** | 0.24 | 0.05 | 0.21 | --- | --- | --- |
| `atan` | **1.00** | 0.99 | 0.22 | 0.15 | --- | --- | --- |
| `atan2` | **1.00** | 0.79 | 0.19 | 0.13 | --- | --- | --- |
| `sqrt` | **1.00** | 0.38 | 0.03 | 0.03 | --- | --- | --- |
| `hypot` | 1.00 | **1.00** | --- | 0.05 | --- | --- | --- |
| `hypot_fast2` | **1.00** | 1.00 | --- | --- | --- | --- | --- |
| `hypot_fast` | **1.00** | 0.72 | --- | 0.61 | --- | --- | --- |
| `log2` | 1.00 | **1.16** | 0.05 | 0.30 | --- | --- | --- |
| `ln` | **1.00** | 0.86 | 0.00 | 0.30 | --- | --- | --- |
| `log10` | **1.00** | 0.87 | 0.04 | 0.25 | --- | --- | --- |
| `pow2` | **1.00** | 0.57 | 0.01 | 0.47 | --- | --- | --- |
| `exp` | **1.00** | 0.61 | 0.01 | 0.50 | --- | --- | --- |
| `pow10` | **1.00** | 0.61 | 0.48 | 0.47 | --- | --- | --- |
| `pow` | **1.00** | 0.68 | 0.00 | 0.33 | --- | --- | --- |


### Library footprint (ROM-sized `.o` totals, decimal bytes)

Each **row is one compiled library/object variant** you might ship on its own (or as your SDK’s math layer). **Do not add the rows together** — real firmware picks **one** primary approximate math approach (plus vendor libm snippets). The benchmark binary links several libraries only to score them in one harness, not because products bundle them all. **qf_math** is **float**; **libfixmath** / **fr_math** are **fixed-point** (different animals — see **README.md** § Float vs fixed-point).

**Where these numbers come from** — see [compare/README.md](README.md) § *Footprint rows (what is actually measured)*.

| Library | Variant | Bytes (dec) | What is counted |
| :--- | :--- | ---:|:---|
| **qf_math** | full | 10308 | Single TU `qf_math.o` at `-Os` (same as `make lib`). |
| **qf_math** | lean | 8952 | `qf_math.o` with `-DQF_MATH_LEAN`: rad/deg/BAM trig, inverse trig, `log2`/`ln`, `pow2`/`exp`/`pow`, `sqrt`, `hypot`/`hypot_fast2`/`hypot_fast8` — no `log10`/`pow10`, waves, or ADSR (see `qf_math.h`). |
| **libfixmath** | bench subset | 3632 | Sum of the `lf_*.o` objects from **Makefile** `LIBFIXMATH_SRCS` (trig + sqrt + exp + fix16 core) — only what the compare harness links. |
| **fr_math** | full | 9768 | `FR_math.c` at `-Os` with default feature set. |
| **fr_math** | lean | 5340 | `FR_math.c` with `-DFR_LEAN -DFR_NO_PRINT`: radian trig, inverse trig, log/exp, sqrt; omits degree/BAM wrappers, hypot, waves/ADSR, and print helpers. |
| **fr_math** | bench no-print | 8316 | `FR_math.c` with `-DFR_NO_PRINT`, `-Os`; this is the object linked by the portable compare harness. |
| **FastTrig** | ESP32 peer object | 2562 | PlatformIO-built `FastTrig.cpp.o` when the LilyGO benchmark has been built; `---` if unavailable on this machine. |
| **ESP-DSP** | local scalar subset | --- | No standalone library object in this harness; the sqrt approximation is a tiny local wrapper in `main.cpp`. |
| **espp/math** | local scalar subset | --- | Header-only/local scalar subset in the LilyGO benchmark, not a separately sized library object. |

> **Note:** There is no single **`libm`** row — vendor math lives in prebuilt toolchain archives; the perf tables time `sinf`/`cosf`/…, not a standalone `.o` total.

<details><summary>libfixmath object breakdown (bench link only)</summary>

| Object file | Bytes |
| :--- | ---:|
| `lf_fix16_exp.o` | 864 |
| `lf_fix16_sqrt.o` | 216 |
| `lf_fix16_trig.o` | 1132 |
| `lf_fix16.o` | 1080 |
| `lf_fract32.o` | 212 |
| `lf_uint32.o` | 128 |

</details>

---

## Interpretation

- **Timings** are from a **desktop-class host** (`sinf`/`logf`/… reference). On Cortex-M (with or without FPU), soft-float costs, flash wait states, and IRQ latency dominate—**re-profile on your MCU**.
- **`qf_math`** timings reflect **`float`** paths; **libfixmath** / **fr_math** are **fixed-point** libraries measured here **via float↔fixed bridges** for uniform double-reference scoring — not “three floats racing.” Native fixed-only call paths drop most conversion overhead.
- **“vs libm”** is `t_libm / t_impl` for the measured loop (values **greater than 1** mean the implementation was faster than host libm in this microbenchmark).
- **Footprint table**: each row is **one library’s object code** — you normally ship **one** math stack; the harness links several **only** for side‑by‑side measurement (don’t add the byte counts together as “total firmware”).
