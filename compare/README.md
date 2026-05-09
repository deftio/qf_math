# Compare suite (`compare/`)

This directory holds **documentation matrices** and a **host benchmark** centred on **`qf_math`**. All downloaded third-party sources and binaries remain under **`build/compare/`** (gitignored).

## Float vs fixed-point (different animals)

- **`qf_math` (this repo)** is a **`float` / IEEE-754 single-precision** library: lookup tables and interpolation use **`qf`** (`typedef float`), so you pay soft-float or FPU costs on targets without hardware FP.

- **`libfixmath`** and **`fr_math`** are **fixed-point** stacks — **`fix16_t` (Q16.16)** and **radix-scaled `s32`** respectively. Their strength is **integer-only** (or narrow-format) paths on **FPU-less** MCUs and predictable rounding.

Those are **not** interchangeable product choices like three skins on one API. You normally commit to **one domain** in your ISR-heavy pipeline and convert only at system boundaries.

**`make compare`** feeds fixed implementations **through float wrappers** so every implementation can be graded against the same double-precision references on the host. That isolates numerical shape vs **`sinf`**, but **it exaggerates cost vs shipping native fixed-only calls**.

If your chip cannot tolerate **`float`** in the loop body, prioritize **[fr_math](https://github.com/deftio/fr_math)** or **libfixmath** directly — **`qf_math`** is for when **`float` is acceptable**.

## Quick commands

| Makefile target | Purpose |
|-----------------|--------|
| `make compare-deps` | Git-clone **libfixmath** + **[fr_math](https://github.com/deftio/fr_math)** into `build/compare/third_party/`. |
| `make compare` | Build and run `build/compare/benchmark_suite`: **float-native** qf_math vs libm vs **fixed libraries measured via float bridges** (host convenience — see § Float vs fixed-point above). |
| `make compare-report` | Markdown **size table**: **full** `qf_math.o`, **peer lean** `qf_math_lean.o` (`-DQF_MATH_LEAN`), libfixmath subset (**fixed**), **`FR_math.o`**. Rows are **different stacks**, not one bundle. |
| `make compare-tests` | CMake-build **libfixmath** upstream `tests_ro64` under `build/compare/third_party/libfixmath-build`. |
| `make compare-github-report` | Rewrite **`compare/BENCHMARK_REPORT.md`** (committed Markdown for GitHub: bench tables + size breakdown). |
| `make compare-fr-tests` | Run **fr_math** upstream `make -f makefile test` (`compare/run_fr_math_tests.sh`; artifacts stay under `build/compare/third_party/fr_math/`). |
| `make mcu-benchmark-snapshot` | Flash **LilyGO T-Display-S3** bench and rewrite **`MCU_BENCHMARK_SNAPSHOT.md`** via UART (**pio**, **pyserial**, USB board; optional **`MCU_SERIAL_PORT`**). |
| `make benchmark-crossplatform` | Merge **`BENCHMARK_REPORT.md`** + **`MCU_BENCHMARK_SNAPSHOT.md`** into **`BENCHMARK_CROSSPLATFORM.md`** (relative Host \| MCU tables; no hardware). |

## Footprint rows (what is actually measured)

The **Library footprint** section in **`BENCHMARK_REPORT.md`** (from **`make compare-report`** / **`make compare-github-report`**) answers “how big is this \`.o\` on *my* host?” — **not** “how big is all of libm.”

| Row | Source artifact | Notes |
|-----|-----------------|--------|
| **qf_math (full)** | `build/qf_math.o` from **`make lib`** | Entire TU: trig, log/exp, sqrt/hypot, waves, ADSR, `log10` / `pow10`, etc. |
| **qf_math (peer-comparable lean)** | `build/qf_math_lean.o` from **`make lib-lean`** | Same TU with **`-DQF_MATH_LEAN`**: families used by **[`benchmark_core.c`](benchmark_core.c)** (radian/deg/BAM trig, inverse trig, `log2`/`ln`, `pow2`/`exp`, `sqrt`, `hypot`) — **no** waves, ADSR, or thin `log10`/`pow10` wrappers. Use this row to compare “math core” flash against the libfixmath bench subset. |
| **libfixmath (bench subset)** | Sum of **`build/compare/obj/lf_*.o`** | Only the \`.c\` files in **Makefile** **`LIBFIXMATH_SRCS`** (fix16 core + trig + sqrt + exp). Not the whole upstream tree. |
| **fr_math** | **`build/compare/obj/fr_math.o`** | **`FR_math.c`** with **`-DFR_NO_PRINT`**. |
| **libm** | *(no row)* | Vendor math is in prebuilt archives — the bench times **`sinf`** / **`sqrtf`** / … at runtime; there is no portable **`size libm.o`**. |

**ESP32 peers** (FastTrig, ESP-DSP, espp/math) show up in **timing** snapshots from LilyGO capture; they do not get automatic \`.o\` size rows here.

## Reading order

1. **`LIBRARIES.md`** — representation, typical targets, licensing, *qualitative* footprint notes.
2. **`FUNCTION_MATRIX.md`** — symbol-level coverage (● full · ◐ partial / alt API · ○ none).
3. **`PEERS.md`** — broader landscape & links (libraries not yet wired into the automated bench).
4. **`BENCHMARK_REPORT.md`** — last committed host snapshot (tables); regenerate with **`make compare-github-report`** before releases or after numerical changes.
5. **`BENCHMARK_CROSSPLATFORM.md`** — POSIX host vs MCU **Speed vs libm** ratios side by side (merged from **`BENCHMARK_REPORT.md`** + **`MCU_BENCHMARK_SNAPSHOT.md`**); run **`make benchmark-crossplatform`** after refreshing either snapshot.
6. Run **`make compare`** / **`make compare-report`** locally when you need numbers for *your* compiler / ISA / OS.
7. **`examples/lilygo_t_display_s3_bench/`** — **PlatformIO** for **LilyGO T-Display-S3** (`lilygo-t-display-s3`): **`pio run -t upload -t monitor`** after **`make compare-deps`**. This firmware appends ESP32-S3 fast-math peer rows for **FastTrig**, **ESP-DSP** sqrt, and **espp/math** scalar helpers, plus inverse trig rows for float-domain peers. See **[examples/lilygo_t_display_s3_bench/README.md](../examples/lilygo_t_display_s3_bench/README.md)**.
8. **`examples/esp32s3_benchmark/`** — **ESP-IDF** firmware (**ESP32-S3**) that runs **[`benchmark_core.c`](benchmark_core.c)** on-chip (`esp_timer_get_time()`); requires **`make compare-deps`** first. See **[examples/esp32s3_benchmark/README.md](../examples/esp32s3_benchmark/README.md)**.
9. **`MCU_BENCHMARK_SNAPSHOT.md`** — committed **on-device** tables; refresh with **`make mcu-benchmark-snapshot`** (UART script). Firmware also prints `DOC_TABLE_*` markers for manual capture.

## Caveats

- **Speed / domain**: **`qf_math`** timings are **float-path**. **`libfixmath`** / **`fr_math`** timings in this harness include **float↔fixed bridges**; native **`fix16_t` / radix `s32`** call paths drop most of that overhead. Host timings are still not MCU timings.
- **Wired peers**: the portable host/MCU core compares **qf_math**, **libm**, **libfixmath**, and **fr_math**. The LilyGO ESP32-S3 firmware additionally appends ESP32-only peer rows for **FastTrig**, **ESP-DSP** sqrt, and **espp/math** helper approximations; those rows are captured in **`MCU_BENCHMARK_SNAPSHOT.md`**.
- **Coverage**: portable timing/error tables cover **`sin`**, **`cos`**, **`sqrt`**, **`ln`**, and **`exp`**. The LilyGO ESP32-S3 snapshot additionally includes peer rows for scalar inverse trig (`asin`, `acos`, `atan`, `atan2`) where the libraries expose those functions.
- **Size**: `compare-report` lists **each** library’s `.o` footprint independently — **do not add the rows** (you normally ship one math approach). The benchmark binary links several stacks only to measure them together. Flash in a real binary still depends on the linker (`--gc-sections`), SDK math stubs, and soft-float glue.
- **CMSIS-DSP**, **TI IQMath**, etc. are documented in the matrices but are **not** vendored here—bring them via your SDK/pack manager.
