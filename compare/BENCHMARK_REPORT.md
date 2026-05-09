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
| UTC time | 2026-05-09 01:55:21Z |
| uname | Darwin arm64 25.4.0 |
| Compiler | Apple LLVM 21.0.0 (clang-2100.0.123.102) |
| Pointer size | 64-bit |
| qf_math | 1.0.0 (`QF_MATH_VERSION_HEX`=0x10000) |
| Loop shape | 8000 sample grid × 8000 outer × 64 inner calls |

### Accuracy

One generated matrix for all benchmarked functions. The `Metric` column defines each accuracy value: `abs %FS` for sine/cosine output amplitude, `abs` for tangent, `abs rad` for inverse trig, and `rel %` for sqrt/hypot/log/exp.

| Function | Metric | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | abs %FS | 0.000000 | 0.001886 | 0.773278 | 0.008172 | --- | --- | --- |
| `sin_deg` | abs %FS | 0.000000 | 0.001883 | 0.773297 | 0.006759 | --- | --- | --- |
| `sin_bam` | abs %FS | 0.000000 | 0.001885 | 0.772995 | 0.004343 | --- | --- | --- |
| `cos_rad` | abs %FS | 0.000000 | 0.001887 | 0.774185 | 0.009101 | --- | --- | --- |
| `cos_deg` | abs %FS | 0.000000 | 0.001884 | 0.774180 | 0.007342 | --- | --- | --- |
| `cos_bam` | abs %FS | 0.000000 | 0.001885 | 0.773652 | 0.004342 | --- | --- | --- |
| `tan_rad` | abs | 0.000000 | 0.000716 | 0.018273 | 0.000555 | --- | --- | --- |
| `tan_deg` | abs | 0.000000 | 0.000650 | 0.016301 | 0.000434 | --- | --- | --- |
| `tan_bam` | abs | 0.000000 | 0.000074 | 0.000068 | 0.000086 | --- | --- | --- |
| `asin` | abs rad | 0.000000 | 0.000231 | 0.010209 | 0.000361 | --- | --- | --- |
| `acos` | abs rad | 0.000000 | 0.000232 | 0.010220 | 0.000357 | --- | --- | --- |
| `atan` | abs rad | 0.000000 | 0.000932 | 0.010158 | 0.000950 | --- | --- | --- |
| `atan2` | abs rad | 0.000000 | 0.000108 | 0.000013 | 0.000314 | --- | --- | --- |
| `sqrt` | rel % | 0.000000 | 0.000472 | 0.380124 | 1.189381 | --- | --- | --- |
| `hypot` | rel % | 0.000000 | 0.000470 | --- | 0.000007 | --- | --- | --- |
| `hypot_fast` | rel % | 0.000000 | 0.137249 | --- | 0.137249 | --- | --- | --- |
| `ln` | rel % | 0.000000 | 0.271975 | 0.086993 | 0.398790 | --- | --- | --- |
| `exp` | rel % | 0.000000 | 0.001504 | 0.286388 | 0.574226 | --- | --- | --- |
### Wall-clock time (microseconds)

Total microseconds normalized to the metadata loop shape. ESP32-only peer rows may be measured with a shorter loop and scaled to the same outer-iteration count. Unsupported cells are `---`.

| Function | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | 1073.250000 | 564.791667 | 5967.041667 | 3223.666667 | --- | --- | --- |
| `sin_deg` | 1195.125000 | 602.500000 | 6807.250000 | 3634.583333 | --- | --- | --- |
| `sin_bam` | 1065.250000 | 712.250000 | 7028.791667 | 1148.791667 | --- | --- | --- |
| `cos_rad` | 1159.458333 | 688.750000 | 5972.375000 | 4706.041667 | --- | --- | --- |
| `cos_deg` | 1176.458333 | 696.625000 | 6606.416667 | 4093.000000 | --- | --- | --- |
| `cos_bam` | 1197.958333 | 675.250000 | 7264.458333 | 1348.458333 | --- | --- | --- |
| `tan_rad` | 1515.125000 | 854.916667 | 14855.000000 | 4194.875000 | --- | --- | --- |
| `tan_deg` | 1476.250000 | 842.625000 | 15747.875000 | 5444.625000 | --- | --- | --- |
| `tan_bam` | 1629.708333 | 987.750000 | 16575.500000 | 1453.916667 | --- | --- | --- |
| `asin` | 992.375000 | 4357.583333 | 24022.791667 | 6101.625000 | --- | --- | --- |
| `acos` | 1190.166667 | 4201.791667 | 24033.125000 | 5611.666667 | --- | --- | --- |
| `atan` | 1256.458333 | 9762.416667 | 5653.250000 | 8615.208333 | --- | --- | --- |
| `atan2` | 1148.625000 | 11025.375000 | 4992.666667 | 9516.041667 | --- | --- | --- |
| `sqrt` | 317.250000 | 841.291667 | 11191.541667 | 9543.416667 | --- | --- | --- |
| `hypot` | 1043.791667 | 834.875000 | --- | 22282.708333 | --- | --- | --- |
| `hypot_fast` | 856.708333 | 1382.333333 | --- | 1425.083333 | --- | --- | --- |
| `ln` | 1214.333333 | 1208.791667 | 341394.125000 | 3928.750000 | --- | --- | --- |
| `exp` | 839.916667 | 1135.083333 | 72611.000000 | 1682.250000 | --- | --- | --- |

### Speed vs libm (ratio)

`libm` time ÷ implementation time for the same function on this platform. Above 1.0 means faster than the platform `libm` call in this microbenchmark.

**Fixed-point peer rows are bridged harness timings**: `float`→fixed→function→`float`. They keep the report comparable, but are not native `fix16_t` / `s32` pipeline timings.

| Function | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | 1.000000 | 1.900258 | 0.179863 | 0.332928 | --- | --- | --- |
| `sin_deg` | 1.000000 | 1.983610 | 0.175566 | 0.328820 | --- | --- | --- |
| `sin_bam` | 1.000000 | 1.495612 | 0.151555 | 0.927279 | --- | --- | --- |
| `cos_rad` | 1.000000 | 1.683424 | 0.194137 | 0.246377 | --- | --- | --- |
| `cos_deg` | 1.000000 | 1.688797 | 0.178078 | 0.287432 | --- | --- | --- |
| `cos_bam` | 1.000000 | 1.774096 | 0.164907 | 0.888391 | --- | --- | --- |
| `tan_rad` | 1.000000 | 1.772249 | 0.101994 | 0.361185 | --- | --- | --- |
| `tan_deg` | 1.000000 | 1.751966 | 0.093743 | 0.271139 | --- | --- | --- |
| `tan_bam` | 1.000000 | 1.649920 | 0.098320 | 1.120909 | --- | --- | --- |
| `asin` | 1.000000 | 0.227735 | 0.041310 | 0.162641 | --- | --- | --- |
| `acos` | 1.000000 | 0.283252 | 0.049522 | 0.212088 | --- | --- | --- |
| `atan` | 1.000000 | 0.128704 | 0.222254 | 0.145842 | --- | --- | --- |
| `atan2` | 1.000000 | 0.104180 | 0.230062 | 0.120704 | --- | --- | --- |
| `sqrt` | 1.000000 | 0.377099 | 0.028347 | 0.033243 | --- | --- | --- |
| `hypot` | 1.000000 | 1.250237 | --- | 0.046843 | --- | --- | --- |
| `hypot_fast` | 1.000000 | 0.619755 | --- | 0.601164 | --- | --- | --- |
| `ln` | 1.000000 | 1.004584 | 0.003557 | 0.309089 | --- | --- | --- |
| `exp` | 1.000000 | 0.739960 | 0.011567 | 0.499282 | --- | --- | --- |


### Library footprint (ROM-sized `.o` totals, decimal bytes)

Each **row is one stack** you might ship on its own (or as your SDK’s math layer). **Do not add the rows together** — real firmware picks **one** primary approximate math approach (plus vendor libm snippets). The benchmark binary links several libraries only to score them in one harness, not because products bundle them all. **qf_math** is **float**; **libfixmath** / **fr_math** are **fixed-point** (different animals — see **README.md** § Float vs fixed-point).

**Where these numbers come from** — see [compare/README.md](README.md) § *Footprint rows (what is actually measured)*.

| Library | Bytes (dec) | What is counted |
| :--- | ---:|:---|
| **qf_math** (full) | 9500 | Single TU `qf_math.o` at `-Os` (same as `make lib`). |
| **qf_math** (peer-comparable lean) | 8144 | `qf_math.o` with `-DQF_MATH_LEAN`: rad/deg/BAM trig, inverse trig, `log2`/`ln`, `pow2`/`exp`, `sqrt`, `hypot`/`hypot_fast8` — no `log10`/`pow10`, waves, or ADSR (see `qf_math.h`). |
| **libfixmath** (bench subset) | 3632 | Sum of the `lf_*.o` objects from **Makefile** `LIBFIXMATH_SRCS` (trig + sqrt + exp + fix16 core) — only what the compare harness links. |
| **fr_math** | 8316 | `FR_math.c` with `-DFR_NO_PRINT`, `-Os`. |

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
