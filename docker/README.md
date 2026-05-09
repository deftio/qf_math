# Docker cross-build & code-size report

Same pattern as **[fr_math](https://github.com/deftio/fr_math)** (`docker/Dockerfile` + `docker/run.sh`): a **linux/amd64** Ubuntu image preloaded with common cross-compilers plus Espressif’s **xtensa-esp-elf** toolchain.

The report compiles **`src/qf_math.c`** in both **lean** and **full** modes for 32/64-bit targets. If `make compare-deps` has fetched `fr_math` into `build/compare/third_party/`, the same run also compiles **`FR_math.c`** in **core** and **full** modes so the table is apples-to-apples across targets and flags.

## Requirements

- Docker (Desktop or engine), able to run **amd64** images (Apple Silicon uses QEMU unless you build natively elsewhere).

## Usage

From the repository root:

```bash
make docker-sizes
```

Force rebuild the image after Dockerfile edits:

```bash
make docker-sizes-rebuild
```

Or equivalently:

```bash
docker build --platform linux/amd64 -t qf-math-sizes docker/
docker run --platform linux/amd64 --rm -v "$(pwd):/src" qf-math-sizes bash /src/docker/build_sizes.sh
python3 tools/update_cross_target_size_docs.py
```

## Outputs

| File | Purpose |
|------|---------|
| `build/docker_sizes.csv` | Source-of-truth matrix with text/data/bss/total columns |
| `build/docker_size_report/` | Intermediate `.o` files |

The script prints a compact stdout summary and a **Cortex-M0** `-O0` / `-Os` / `-O2` / `-O3` comparison for qf lean/full. `make docker-sizes` then runs `tools/update_cross_target_size_docs.py`, which refreshes the compact section in `compare/README.md` from the CSV.

## Makefile shortcut

```bash
make docker-sizes          # CSV + compare/README summary
make docker-sizes-rebuild  # rebuild image, then CSV + summary
```

## pocketdock (optional)

Same idea as fr_math — drive the container from Python ([pocketdock](https://github.com/deftio/pocketdock)):

```python
from pocketdock import Container

c = Container(dockerfile="docker/Dockerfile", tag="qf-math-sizes")
c.run("bash /src/docker/build_sizes.sh", mount={"src": "."})
```

## Note on floats

`qf_math` is **float32**. Bare-metal objects still rely on **libgcc** soft-float helpers when you link a full image — the numbers here are **object-file sizes only**, not total firmware flash.

The trig tables are part of the object totals. That is intentional: this report is meant to make table footprint visible when comparing `qf_math` against integer-first libraries such as `fr_math`.
