# Agents — qf_math

Guidance for automated coding agents editing this repository.

## Layout (do not fight it)

- **`src/`** — Library only (`qf_math.c`, `qf_math.h`). Keep this TU self-contained and allocation-free unless explicitly requested.
- **`test/`** — Host-side regression (`qf_math_test.c`). Extend here when changing numerical behavior.
- **`compare/`** — Peer-library matrices (`*.md`), benchmark harness (`benchmark_suite.c`, **`benchmark_core.c`** — shared with ESP **`examples/`** trees), fetch/report scripts. All clones/build artifacts stay under `build/compare/`.
- **`examples/`** — Optional on-device benches: **`examples/lilygo_t_display_s3_bench/`** (PlatformIO, LilyGO T-Display-S3), **`examples/esp32s3_benchmark/`** (ESP-IDF), and **`examples/pico2_benchmark/`** (Arduino, Raspberry Pi Pico 2).
- **`docs/`** — Markdown documentation (algorithms, API reference, fr_math relationship, integration guide).
- **`pages/`** — GitHub Pages HTML site (compact single-page bench + overview); deploy via GitHub Actions.
- **`tools/`** — Host helpers (`qf_math_bench.c`, `run_libfixmath_tests.sh`); keep MCU-light.
- **`build/`** — All binaries, CMake trees, cloned deps. Never commit; never author primary sources only here.

## Build commands

- Unit tests: `make test`
- Library object: `make lib`
- qf vs libm benchmark: `make bench`
- Multi-library compare bench + matrices docs: see `compare/README.md`; run `make compare` / `make compare-report`
- Regenerate GitHub-facing Markdown report: `make compare-github-report` → `compare/BENCHMARK_REPORT.md`
- Upstream libfixmath tests: `make compare-tests`
- Upstream fr_math tests: `make compare-fr-tests`
- Cross-target ROM/code-size table (Docker, fr_math-style): `make docker-sizes` → `build/docker_size_table.md` (see `docker/README.md`)
- MCU compare benchmark (needs **`make compare-deps`**): **`examples/lilygo_t_display_s3_bench/`** (`pio run -t upload -t monitor`), **`examples/esp32s3_benchmark/`** (`idf.py`), or **`examples/pico2_benchmark/`** (Arduino IDE / `arduino-cli`).
- Silicon snapshot: **`make mcu-benchmark-snapshot`** → **`compare/MCU_BENCHMARK_SNAPSHOT*.md`** (**pio** / Arduino, **pyserial**, USB).

## Editing rules

- Preserve SPDX/license headers and existing comment tone.
- Prefer `-std=c99` compatible code; avoid GCC-only extensions unless already present nearby.
- Numerical changes require coordinated tolerance updates in `test/qf_math_test.c` when behavior shifts.
- Do not add silent network fetches outside `compare/fetch_deps.sh` (invoked by Makefile `compare-deps`) and documented compare targets.

## Manifests

- `library.json` — PlatformIO; keep `src/` as include dir.
- `idf_component.yml` + root `CMakeLists.txt` — ESP-IDF component wiring only; fatal-error if configured standalone without `ESP_PLATFORM`. Published component uses strong warnings but not `-Werror` on the component target.

## Markdown policy

Root `README.md` is canonical user-facing. **`docs/`** holds Markdown documentation (algorithms, API, fr_math, integration). **`pages/`** holds the GitHub Pages HTML site. **`compare/*.md`** holds methodology matrices; **`compare/BENCHMARK_REPORT.md`** is the tooling-generated snapshot (`make compare-github-report`)—refresh it after materially changing numerical behavior if you want GitHub viewers to see fresh tables.
