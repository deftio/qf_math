[![License](https://img.shields.io/badge/License-BSD%202--Clause-blue.svg)](https://opensource.org/licenses/BSD-2-Clause)
[![CI](https://github.com/deftio/qf_math/actions/workflows/ci.yml/badge.svg)](https://github.com/deftio/qf_math/actions/workflows/ci.yml)
[![Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen.svg)](#coverage-gate)
[![Version](https://img.shields.io/badge/version-1.0.1-blue.svg)](src/qf_math.h)
[![GitHub](https://img.shields.io/badge/GitHub-repo-181717.svg?logo=github)](https://github.com/deftio/qf_math)

[![PlatformIO](https://img.shields.io/badge/PlatformIO-library-teal.svg)](https://registry.platformio.org/libraries/deftio/qf_math)
[![Arduino](https://img.shields.io/badge/Arduino-from%20source-teal.svg)](https://github.com/deftio/qf_math)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-component-teal.svg)](https://components.espressif.com/components/deftio/qf_math)

# Quick Float Math (`qf_math`)

**qf_math** is a lightweight library for fast approximate math on IEEE-754 `float` — built for ARM Cortex-M, RISC-V, Xtensa/ESP32, and any 32-bit target. One translation unit, no heap, no dependencies, no FPU required (but benefits from one). Drop it into firmware for predictable-cost trig, log, exp, sqrt, hypot, waveforms, and ADSR — without pulling in full libm.  

Need fixed point?:  **[fr_math](https://github.com/deftio/fr_math)** is the fixed-point (pure integer) cousin with the same API.  
Docs: **[qf_math docs](https://deftio.github.io/qf_math/)** html pages for this library with algorithms, example code, metrics.  

## Float or fixed-point?  
Depending on whether the target has hardware floating point support different chocies may be appropriate.

| | **qf_math** (this library) | **[fr_math](https://github.com/deftio/fr_math)** |
|---|---|---|
| **Representation** | IEEE-754 `float` (32-bit) | Caller-selectable fixed-radix `int32_t` (e.g. s15.16, s19.12, s23.8) |
| **Best on** | Cortex-M4F/M7, ESP32, RP2040/RP2350 with FPU | Cortex-M0/M3, 8/16-bit MCUs, no-FPU targets |
| **Soft-float cost** | High (compiler inserts sw multiply/add) | Zero (integer ALU only) |
| **Dynamic range** | ~1e-38 to ~3e+38 | Determined by radix (e.g. R=16: ±32K; R=12: ±512K; R=8: ±8M) |
| **Typical accuracy** | < 0.002% FS (trig), < 0.001% rel (log/exp) | < 0.008% FS (trig), < 0.4% rel (log/exp) |
| **Code size** | ~10 KB (full), ~9 KB (lean) | ~10 KB (full), ~5 KB (lean) |


## Accuracy — qf_math vs fr_math vs libm

Peak error from identical benchmark test suites. Both libraries use the  approximation strategies (BAM phase, sub-step interpolation, poly interpolation,  etc) with differences from quantization and dynamic range tradeoffs.

| Function | Metric | libm | **qf_math** | **fr_math** |
|:---------|:-------|-----:|------------:|------------:|
| sin (rad) | %FS | 0 | **0.0019** | 0.0078 |
| cos (rad) | %FS | 0 | **0.0019** | 0.0087 |
| tan (rad) | abs | 0 | 0.30 | **0.051** |
| asin | abs rad | 0 | 0.00047 | **0.00036** |
| acos | abs rad | 0 | 0.00047 | **0.00036** |
| atan2 | abs rad | 0 | 0.0013 | **0.00094** |
| sqrt | rel % | 0 | **0.00047** | 1.19 |
| log2 | rel % | 0 | **0.0016** | 0.32 |
| exp | rel % | 0 | **0.00034** | 0.57 |
| pow | rel % | 0 | **0.00038** | 0.087 |
| hypot | rel % | 0 | **0.00047** | 0.000007 |

> **%FS** = % of full-scale (±1 output for trig). **rel %** = relative error vs `double` reference. **abs rad** = absolute radians. Bold = closer to libm. `libm` column is the `*f` reference (effectively 0 error at this precision).


## Speed vs libm — ESP32-S3 (with FPU)

Ratio of libm wall-clock time to library time for the same loop. **> 1.0** = faster than `sinf`/`sqrtf`/etc. on that platform. ESP32-S3 has a single-precision FPU, so both `qf_math` and `libm` use hardware float — the advantage comes from simpler approximations with fewer branches.

| Function | **qf_math** | **fr_math** | Notes |
|:---------|------------:|------------:|:------|
| sin (rad) | **4.30**× | 1.32× | qf_math: table + lerp; fr_math: integer table via float bridge |
| cos (rad) | **4.39**× | 0.90× | |
| tan (rad) | **3.56**× | 1.31× | |
| asin | 1.10× | 0.77× | |
| acos | 1.11× | 0.77× | |
| atan2 | **1.41**× | 0.70× | |
| sqrt | 1.07× | 0.08× | libm sqrtf is hard to beat (HW instruction on many cores) |
| exp | **2.63**× | 1.88× | |
| ln | **2.20**× | 0.92× | |
| hypot | **2.51**× | 0.21× | fr_math hypot goes through float bridge overhead |

> **fr_math** numbers include **float↔fixed bridge overhead** — native `s32` / `fix16_t` calls in a pure-integer pipeline are faster. See [`compare/BENCHMARK_CROSSPLATFORM.md`](compare/BENCHMARK_CROSSPLATFORM.md) for full matrices including libfixmath, FastTrig, and ESP-DSP.


## Quick start

```c
#include "qf_math.h"

qf y   = qf_sin(1.0f);            /* radians */
qf len = qf_hypot(dx, dy);
qf dB  = 20.0f * qf_log10(v / ref);
```

| | |
|---|---|
| **Language** | C99 (`float` API, `qf` typedef), plus optional C++ header wrapper |
| **Dependencies** | None at compile time for the library itself |
| **License** | BSD-2-Clause — see `LICENSE.txt` |

Version macros live in [`qf_math.h`](src/qf_math.h): `QF_MATH_VERSION` / `QF_MATH_VERSION_HEX`. When you bump the version string, update the **Version** badge at the top of this README to match.

For releases, prefer the version/release helpers so manifests, docs, badges, pages, and generated reports stay in sync:

```bash
python3 tools/qf_version.py show --format markdown
python3 tools/qf_version.py update 1.0.2
make make-release VERSION=1.0.2
make make-release RELEASE_ARGS="--dry-run --skip-docker --skip-peer-tests"
```


## Why use this?

- **Deterministic building blocks** for motor control, graphics-lite, sensor fusion, LED/audio synthesis, and telemetry pipelines.
- **Tight code size**: one translation unit with shared lookup tables (see `make size`).
- **Explicit trade-offs**: worst-case errors are documented in headers (e.g. trig ~3e-5 class behavior vs libm doubles).
- **No heap**, minimal stack depth in normal use, integer-friendly helpers (`QF_*` macros, BAM phase, fixed-radix bridges).


## API overview

Public surface is declared in `src/qf_math.h`:

- **Macros**: clamps, interpolation, deg/rad/BAM, radix bridges (`QF_TO_FR`, …).
- **Trig**: `qf_sin`, `qf_cos`, `qf_tan`, BAM-native variants, inverse trig.
- **Log/exp/pow**: `qf_log2`, `qf_ln`, `qf_log10`, `qf_pow2`, `qf_exp`, `qf_pow10`, `qf_pow`.
- **Length**: `qf_sqrt`, `qf_hypot`, `qf_hypot_fast2`, `qf_hypot_fast8` (piecewise-linear magnitude).
- **Audio-ish**: LFSR noise, PWM/square/saw/triangle waves, `qf_adsr_*` envelope.

Domain violations (`sqrt` of negative, `log` of non-positive) return `QF_DOMAIN_ERROR`.

Full reference: [`docs/API.md`](docs/API.md) — Algorithms: [`docs/ALGORITHMS.md`](docs/ALGORITHMS.md) — [Float math tradeoffs](https://deftio.github.io/qf_math/float-math-tradeoffs.html) (libm vs qf_math vs fixed-point vs DSP)


## Repository layout

```
src/           qf_math.c, qf_math.h/.hpp — drop-in C library + C++ wrapper
test/          qf_math_test.c         — correctness vs libm on host
docs/          Markdown documentation (algorithms, API, fr_math, integration)
pages/         GitHub Pages site (compact HTML + CSS; deploy via Actions)
examples/      optional demos — ESP-IDF, LilyGO T-Display-S3, Raspberry Pi Pico 2
docker/        multi-arch toolchain image + cross size report
compare/       matrices vs other math libs + benchmark harness (see below)
tools/         extra shell helpers & qf_math vs libm micro-benchmark
build/         compiled binaries, CMake outputs, cloned deps (gitignored)
```


## Build & test (host)

Requires a normal UNIX toolchain (`gcc` or `clang`) and `-lm` for **tests and benchmarks only** (references use `sinf`, `logf`, …).

### Coverage gate

[`make coverage-check`](Makefile) (also run in CI) rebuilds tests with gcov and **fails unless every executable line in `src/qf_math.c` is covered**. Use `make coverage` for the same build plus a human-readable gcov summary without enforcing 100%.

```bash
make help          # targets
make lib           # build/qf_math.o
make test          # build & run unit tests
make bench         # qf_math vs libm speed/accuracy (single TU bench)
make size          # object size report
make coverage      # gcov summary (optional gcov install; non-failing)
make coverage-check  # CI-style gate: 100% lines covered in src/qf_math.c
make clean
```


## Cross-compile code sizes (Docker)

Like **[fr_math](https://github.com/deftio/fr_math)**, this repo ships a **`docker/`** image (Ubuntu + common 32/64-bit cross-compilers + Espressif Xtensa) to compile **`src/qf_math.c`** for several CPUs and emit the source-of-truth size matrix (`build/docker_sizes.csv`).

```bash
make docker-sizes           # CSV + compact compare/README.md summary
make docker-sizes-rebuild   # rebuild image, then CSV + summary
```

Details: **[docker/README.md](docker/README.md)**.


## Comparisons vs other math stacks (`compare/`)

Documentation tables live under **`compare/`** (`LIBRARIES.md` for platforms/representation/footprint guidance, `FUNCTION_MATRIX.md` for symbol coverage vs **libfixmath**, **fr_math**, **CMSIS-DSP**, **libm**, …).

| Target | What it does |
|--------|----------------|
| `make compare-deps` | Shallow-clones **[libfixmath](https://github.com/PetteriAimonen/libfixmath)** (MIT) and **[fr_math](https://github.com/deftio/fr_math)** into `build/compare/third_party/`. |
| `make compare` | Builds `build/compare/benchmark_suite` — accuracy + wall-clock for **qf_math**, **libm**, **libfixmath** (float bridge), **fr_math** (fixed-point bridge), and a **Taylor poly** baseline. |
| `make compare-github-report` | Regenerates **[`compare/BENCHMARK_REPORT.md`](compare/BENCHMARK_REPORT.md)** for GitHub. |
| `make mcu-benchmark-snapshot` | Flash MCU bench and rewrite **`compare/MCU_BENCHMARK_SNAPSHOT*.md`** via UART. |
| `make benchmark-crossplatform` | Rewrite **[`compare/BENCHMARK_CROSSPLATFORM.md`](compare/BENCHMARK_CROSSPLATFORM.md)** — Host + MCU ratios from committed snapshots. |

**On silicon:** **[examples/lilygo_t_display_s3_bench](examples/lilygo_t_display_s3_bench/README.md)** (PlatformIO), **[examples/esp32s3_benchmark](examples/esp32s3_benchmark/README.md)** (ESP-IDF), or **[examples/pico2_benchmark](examples/pico2_benchmark/)** (Arduino, Pico 2 ARM/RISC-V) run the **same** [`benchmark_core.c`](compare/benchmark_core.c) loops on real hardware.


## Packaging & integration

### PlatformIO

Use `library.json` at the repo root: add this folder as a local library or publish following [PlatformIO library format](https://docs.platformio.org/en/latest/manifests/library-json/index.html). Headers resolve from `src/` (`#include "qf_math.h"` for C, `#include "qf_math.hpp"` for C++).

### Espressif ESP-IDF

`idf_component.yml` registers the component for the ESP-IDF Component Manager. `CMakeLists.txt` wraps `idf_component_register` for `src/qf_math.c` with include path `src/`.

Full guide: [`docs/INTEGRATION.md`](docs/INTEGRATION.md)


## Documentation

| File | Purpose |
|------|---------|
| [`docs/`](docs/) | **Markdown documentation** — algorithms, API reference, fr_math relationship, integration guide |
| [`pages/`](pages/) | **GitHub Pages** site (live at `https://deftio.github.io/qf_math/`) |
| [`compare/`](compare/) | Compare harness docs, benchmark reports, cross-platform matrices |
| `llms.txt` | Compact index for LLM tooling / crawling |
| `agents.md` | Conventions for coding agents working in this repo |


## Contributing & correctness

- Prefer extending tests in `test/qf_math_test.c` when adjusting numerical behavior.
- Keep binaries and fetched trees under `build/` only.
- Match existing comment and license style in touched files.


## Credits

Author: **M. A. Chatterjee** (`deftio` at `deftio` dot `com`). See `LICENSE.txt` for full BSD-2-Clause text.

Third-party comparison tooling pulls **libfixmath** (MIT) and **fr_math** (BSD-2-Clause); neither is bundled in git—they populate only under `build/compare/third_party/` when you run comparison targets.
