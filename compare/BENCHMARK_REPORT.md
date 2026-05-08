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
| UTC time | 2026-05-08 16:03:16Z |
| uname | Darwin arm64 25.4.0 |
| Compiler | Apple LLVM 21.0.0 (clang-2100.0.123.102) |
| Pointer size | 64-bit |
| qf_math | 1.0.0 (`QF_MATH_VERSION_HEX`=0x10000) |
| Loop shape | 8000 sample grid × 80000 outer × 64 inner calls |

### Accuracy — max percent error

**sin / cos:** maximum absolute error versus `sin`/`cos` in double, expressed as **percent of unit amplitude** (output in [-1, 1], so multiply absolute error by 100).

**sqrt / ln / exp:** maximum **relative** error versus the double reference on the same sample grids as the C code, expressed as **percent** (max |(approx − ref) / ref| × 100).

| Function | qf_math | libfixmath | fr_math |
| :--- | ---:| ---:| ---:|
| `sin` | 0.001884 | 0.773278 | 0.008172 |
| `cos` | 0.001885 | 0.774185 | 0.009101 |
| `sqrt` | 0.000472 | 0.380124 | 1.189381 |
| `ln` | 0.271975 | 0.086993 | 0.398790 |
| `exp` | 0.001504 | 0.286388 | 0.574226 |

Numbers are printed with six digits after the decimal point. Current timed/error coverage is `sin`, `cos`, `sqrt`, `ln`, and `exp`; broader API accuracy (`tan`, inverse trig, `log2`, `log10`, `pow2`, `pow10`, `hypot`, etc.) belongs in a separate coverage table before it is mixed into these platform timing rows.

### Wall-clock time (microseconds)

Total microseconds for each benchmark loop on this host (see metadata).

| Library | sin | cos | sqrt | ln | exp |
| :--- | ---:| ---:| ---:| ---:| ---:|
| **qf_math** | 15123.000000 | 15384.541667 | 8471.625000 | 12342.375000 | 11301.041667 |
| **libm** (`sinf` …) | 12539.291667 | 11798.416667 | 3261.833333 | 12485.166667 | 8639.166667 |
| **libfixmath** (float bridge) | 57304.291667 | 59597.708333 | 114859.750000 | 3457138.250000 | 729386.041667 |
| **fr_math** (float bridge) | 29539.166667 | 47080.958333 | 98085.583333 | 39493.375000 | 16867.666667 |

### Speed vs libm (ratio)

`libm` time ÷ implementation time for the same loop — **above 1.0** means faster than host `sinf`/`cosf`/… **in this microbenchmark** (not representative of every MCU).

**Fixed-point peer rows are bridged harness timings**: `float`→fixed→function→`float`. They keep the report comparable, but are not native `fix16_t` / `s32` pipeline timings.

| Library | sin | cos | sqrt | ln | exp |
| :--- | ---:| ---:| ---:| ---:| ---:|
| **qf_math** | 0.829154 | 0.766901 | 0.385030 | 1.011569 | 0.764458 |
| libm (reference) | 1.000000 | 1.000000 | 1.000000 | 1.000000 | 1.000000 |
| **libfixmath** (float bridge) | 0.218819 | 0.197968 | 0.028398 | 0.003611 | 0.011844 |
| **fr_math** (float bridge) | 0.424497 | 0.250598 | 0.033255 | 0.316133 | 0.512173 |


### Library footprint (ROM-sized `.o` totals, decimal bytes)

Each **row is one stack** you might ship on its own (or as your SDK’s math layer). **Do not add the rows together** — real firmware picks **one** primary approximate math approach (plus vendor libm snippets). The benchmark binary links several libraries only to score them in one harness, not because products bundle them all. **qf_math** is **float**; **libfixmath** / **fr_math** are **fixed-point** (different animals — see **README.md** § Float vs fixed-point).

| Library | Bytes (dec) | What is counted |
| :--- | ---:|:---|
| **qf_math** | 9436 | Single TU `qf_math.o` at `-Os` (same as `make lib`). |
| **libfixmath** (bench subset) | 3632 | Sum of fix16 + trig + sqrt + exp objects used by this compare link — see repo Makefile. |
| **fr_math** | 8316 | `FR_math.c` with `-DFR_NO_PRINT`, `-Os`. |

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
