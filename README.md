[![License](https://img.shields.io/badge/License-BSD%202--Clause-blue.svg)](https://opensource.org/licenses/BSD-2-Clause)
[![CI](https://github.com/deftio/qf_math/actions/workflows/ci.yml/badge.svg)](https://github.com/deftio/qf_math/actions/workflows/ci.yml)
[![Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen.svg)](#coverage-gate)
[![Docs](https://img.shields.io/badge/docs-compare%20%26%20bench-blue.svg)](compare/README.md)
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](src/qf_math.h)
[![GitHub](https://img.shields.io/badge/GitHub-repo-181717.svg?logo=github)](https://github.com/deftio/qf_math)
   
[![PlatformIO](https://img.shields.io/badge/PlatformIO-library-teal.svg)](https://registry.platformio.org/libraries/deftio/qf_math)
[![Arduino](https://img.shields.io/badge/Arduino-from%20source-teal.svg)](https://github.com/deftio/qf_math)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-component-teal.svg)](https://components.espressif.com/components/deftio/qf_math)

# Quick Float Math (`qf_math`)

**qf_math** is a compact, dependency-free C99 library for **fast approximate math on IEEE-754 `float`**. It targets embedded firmware where you want predictable cost and small flash footprint: table-driven trig, logarithms, exponentials, `sqrt`, `hypot`, waveform helpers, and a floating-point ADSR envelope—without pulling in full libm semantics on platforms where that matters.

[github.com/deftio/fr_math](github.com/deftio/fr_math) is the fixed point (pure integer) cousin of this library with identical apis.

| | |
|---|---|
| **Language** | C99 (`float` API, `qf` typedef), plus optional C++ header wrapper |
| **Dependencies** | None at compile time for the library itself |
| **License** | BSD-2-Clause — see `LICENSE.txt` |
| **Layout** | `src/` library · `test/` unit tests · `build/` all binaries & fetched third-party sources |

Version macros live in [`qf_math.h`](src/qf_math.h): `QF_MATH_VERSION` / `QF_MATH_VERSION_HEX`. When you bump the version string, update the **Version** badge at the top of this README to match.

---

## Why use this?

- **Deterministic building blocks** for motor control, graphics-lite, sensor fusion, LED/audio synthesis, and telemetry pipelines.
- **Tight code size**: one translation unit with shared lookup tables (see `make size`).
- **Explicit trade-offs**: worst-case errors are documented in headers (e.g. trig ~3e-5 class behavior vs libm doubles).
- **No heap**, minimal stack depth in normal use, integer-friendly helpers (`QF_*` macros, BAM phase, fixed-radix bridges).

---

## FPU-less alternative: [`fr_math`](https://github.com/deftio/fr_math)

For targets **without a hardware FPU** (or when you want to avoid soft-float cost), use **[fr_math](https://github.com/deftio/fr_math)**—a **battle-tested** sister library that exposes the **same API** using **pure fixed-point** math, with **16-bit** platform support when that is a requirement.

---

## Repository layout

```
src/           qf_math.c, qf_math.h/.hpp — drop-in C library + C++ wrapper
test/          qf_math_test.c         — correctness vs libm on host
docs/          Markdown documentation (algorithms, API, fr_math, integration)
pages/         GitHub Pages site (compact HTML + CSS; deploy via Actions)
examples/      optional demos — ESP-IDF (`esp32s3_benchmark`), LilyGO T-Display-S3 PlatformIO (`lilygo_t_display_s3_bench`), Raspberry Pi Pico 2 Arduino (`pico2_benchmark`)
docker/        multi-arch toolchain image + cross size report (fr_math-style)
compare/       matrices vs other math libs + benchmark harness (see below)
tools/         extra shell helpers & qf_math vs libm micro-benchmark
build/         compiled binaries, CMake outputs, cloned deps (gitignored)
```

Everything that resembles an artifact—including vendored comparison libraries—stays under **`build/`** (for example `build/compare/third_party/`) so your tree stays clean.

---

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

---

## Cross-compile code sizes (Docker)

Like **[fr_math](https://github.com/deftio/fr_math)**, this repo ships a **`docker/`** image (Ubuntu + common cross-compilers + Espressif Xtensa) to compile **`src/qf_math.c`** for several CPUs and emit **`.text`** sizes (`build/docker_size_table.md`).

```bash
make docker-sizes           # same as ./docker/run.sh
make docker-sizes-rebuild   # force docker image rebuild first
```

Details: **[docker/README.md](docker/README.md)**.

---

## Comparisons vs other math stacks (`compare/`)

Documentation tables live under **`compare/`** (`LIBRARIES.md` for platforms/representation/footprint guidance, `FUNCTION_MATRIX.md` for symbol coverage vs **libfixmath**, **fr_math**, **CMSIS-DSP**, **libm**, …).

| Target | What it does |
|--------|----------------|
| `make compare-deps` | Shallow-clones **[libfixmath](https://github.com/PetteriAimonen/libfixmath)** (MIT) and **[fr_math](https://github.com/deftio/fr_math)** into `build/compare/third_party/`. |
| `make compare` (alias `make compare-matrix`) | Builds `build/compare/benchmark_suite` — accuracy + wall-clock for **qf_math**, **libm**, **libfixmath** (float bridge), **fr_math** (Q16.16 bridge), and a **Taylor poly** baseline. |
| `make compare-report` | Prints a Markdown **size table** (`size(1)` totals) for `qf_math.o`, the libfixmath object subset, and `FR_math.o`. |
| `make compare-github-report` | Regenerates **[`compare/BENCHMARK_REPORT.md`](compare/BENCHMARK_REPORT.md)** for GitHub (embedded bench tables + sizes). |
| `make compare-tests` | CMake-builds libfixmath `tests_ro64` under `build/compare/third_party/libfixmath-build` and runs it. |
| `make compare-fr-tests` | Runs **fr_math**’s upstream `make test` against the shallow clone (`compare/run_fr_math_tests.sh`). |
| `make mcu-benchmark-snapshot` | Flash MCU bench and rewrite **`compare/MCU_BENCHMARK_SNAPSHOT*.md`** via UART (**pio** / Arduino, **pyserial**). Supports LilyGO T-Display-S3, ESP32-S3, and Raspberry Pi Pico 2. |
| `make benchmark-crossplatform` | Rewrite **`compare/BENCHMARK_CROSSPLATFORM.md`** — Host \| MCU **Speed vs libm** ratios from the committed snapshots (no hardware). |

**Interpretation:** Desktop timings are *not* MCU timings. Bridged benchmarks include float↔fixed overhead that disappears when you stay natively in `fix16_t` / fixed-radix `s32`. The automated `s32` peer row is **fr_math**; other fast libraries are surveyed in [`compare/PEERS.md`](compare/PEERS.md) but are not wired into the reproducible host/ESP32-S3 harness yet. Use these numbers for intuition and regression tracking, not as a substitute for profiling on your silicon.

**On silicon:** **[examples/lilygo_t_display_s3_bench](examples/lilygo_t_display_s3_bench/README.md)** (PlatformIO, **LilyGO T-Display-S3**), **[examples/esp32s3_benchmark](examples/esp32s3_benchmark/README.md)** (ESP-IDF), or **[examples/pico2_benchmark](examples/pico2_benchmark/)** (Arduino, **Raspberry Pi Pico 2** ARM / RISC-V) run the **same** [`benchmark_core.c`](compare/benchmark_core.c) loops so UART / USB serial captures wall-clock on real hardware.

**Host vs MCU (relative only):** **[compare/BENCHMARK_CROSSPLATFORM.md](compare/BENCHMARK_CROSSPLATFORM.md)** merges **Speed vs libm** tables from [`compare/BENCHMARK_REPORT.md`](compare/BENCHMARK_REPORT.md) (POSIX snapshot) and the MCU snapshots (`MCU_BENCHMARK_SNAPSHOT*.md` — ESP32-S3, Pico 2 ARM, Pico 2 RISC-V); regenerate with **`make benchmark-crossplatform`** after refreshing those files.

---

## Packaging & integration

### PlatformIO

- **Manifest:** [`library.json`](library.json) (registry: [deftio/qf_math](https://registry.platformio.org/libraries/deftio/qf_math)).
- **`lib_deps`:** `deftio/qf_math` or `https://github.com/deftio/qf_math.git`
- **Lean subset** (smaller ROM, no `log10` / `pow10` / waves / ADSR): add to the env `build_flags = -DQF_MATH_LEAN`
- **`export` exclude** in the manifest trims compare/tests/docs from the PIO-installed copy; clones via **git URL** still get the full repo (including **`examples/`**).
- Sanity check locally: `pio pkg pack .` must succeed before publishing.

### Arduino Library Manager / Arduino IDE

- **Manifest:** [`library.properties`](library.properties) beside `library.json` (Arduino 2.x discovers `src/` layout).
- Install from ZIP of the repo or symlink the folder under `Arduino/libraries/`; include `#include <qf_math.h>` (Arduino adds library `src/` to the include path).

### Espressif ESP-IDF

- **Component Manager:** [`idf_component.yml`](idf_component.yml) + [`CMakeLists.txt`](CMakeLists.txt). Component tarball excludes bulky trees (`examples/`, `compare/`, …); **`src/`**, root **`CMakeLists.txt`**, **`LICENSE.txt`**, and **`README.md`** remain usable.
- **Consumer projects:** `#include "qf_math.h"`; optional **`QF_MATH_LEAN`** via `target_compile_definitions` on your target or **`idf.py menuconfig`/CMake extra flags**.
- **Warnings:** component build uses `-Wall -Wextra -Wshadow -Wconversion` **without** `-Werror` so toolchain/IDF updates are less likely to break downstream apps.

Add as a dependency (path or git URL) per Espressif docs.

---

## API overview

Public surface is declared in `src/qf_math.h`:

- **Macros**: clamps, interpolation, deg/rad/BAM, radix bridges (`QF_TO_FR`, …).
- **Trig**: `qf_sin`, `qf_cos`, `qf_tan`, BAM-native variants, inverse trig.
- **Log/exp/pow**: `qf_log2`, `qf_ln`, `qf_log10`, `qf_pow2`, `qf_exp`, `qf_pow10`, `qf_pow`.
- **Length**: `qf_sqrt`, `qf_hypot`, `qf_hypot_fast2`, `qf_hypot_fast8` (piecewise-linear magnitude).
- **Audio-ish**: LFSR noise, PWM/square/saw/triangle waves, `qf_adsr_*` envelope.

Domain violations (`sqrt` of negative, `log` of non-positive) return `QF_DOMAIN_ERROR`.

---

## Documentation for automation

| File | Purpose |
|------|---------|
| [`docs/`](docs/) | **Markdown documentation** — algorithms, API reference, fr_math relationship, integration guide |
| [`pages/`](pages/) | **GitHub Pages** site (enable *Settings → Pages → GitHub Actions*; live at `https://deftio.github.io/qf_math/` after first deploy) |
| [`compare/README.md`](compare/README.md) | Compare harness docs (`make compare`, sizes, upstream suites, GitHub report) |
| [`compare/BENCHMARK_REPORT.md`](compare/BENCHMARK_REPORT.md) | Last checked-in **host benchmark + size snapshot** (run `make compare-github-report` to refresh) |
| [`compare/BENCHMARK_CROSSPLATFORM.md`](compare/BENCHMARK_CROSSPLATFORM.md) | Host vs MCU **relative** ratios (`make benchmark-crossplatform`; refresh snapshots first) |
| `llms.txt` | Compact index for LLM tooling / crawling |
| `agents.md` | Conventions for coding agents working in this repo |

---

## Contributing & correctness

- Prefer extending tests in `test/qf_math_test.c` when adjusting numerical behavior.
- Keep binaries and fetched trees under `build/` only.
- Match existing comment and license style in touched files.

---

## Credits

Author: **M. A. Chatterjee** (`deftio` at `deftio` dot `com`). See `LICENSE.txt` for full BSD-2-Clause text.

Third-party comparison tooling pulls **libfixmath** (MIT) and **fr_math** (BSD-2-Clause); neither is bundled in git—they populate only under `build/compare/third_party/` when you run comparison targets.
