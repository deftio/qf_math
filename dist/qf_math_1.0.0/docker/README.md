# Docker cross-build & code-size report

Same pattern as **[fr_math](https://github.com/deftio/fr_math)** (`docker/Dockerfile` + `docker/run.sh`): an **linux/amd64** Ubuntu image preloaded with common cross-compilers plus Espressif’s **xtensa-esp-elf** toolchain, used only to compile **`src/qf_math.c`** to `.o` files and read **`.text`** sizes via `size(1)`.

## Requirements

- Docker (Desktop or engine), able to run **amd64** images (Apple Silicon uses QEMU unless you build natively elsewhere).

## Usage

From the repository root:

```bash
./docker/run.sh
```

Force rebuild the image after Dockerfile edits:

```bash
./docker/run.sh --rebuild
```

Or equivalently:

```bash
docker build --platform linux/amd64 -t qf-math-sizes docker/
docker run --platform linux/amd64 --rm -v "$(pwd):/src" qf-math-sizes bash /src/docker/build_sizes.sh
```

## Outputs

| File | Purpose |
|------|---------|
| `build/docker_size_table.md` | Markdown table (stdout is the same table) |
| `build/docker_sizes.csv` | Machine-readable rows |
| `build/docker_size_report/` | Intermediate `.o` files |

The script also prints a **Cortex-M0** `-O0` / `-Os` / `-O2` / `-O3` comparison and, if **`make compare-deps`** was run on the host so **`build/compare/third_party/libfixmath/`** exists, a rough **qf_math vs libfixmath** text-size comparison for Cortex-M0.

## Makefile shortcut

```bash
make docker-sizes          # ./docker/run.sh
make docker-sizes-rebuild   # ./docker/run.sh --rebuild
```

## pocketdock (optional)

Same idea as fr_math — drive the container from Python ([pocketdock](https://github.com/deftio/pocketdock)):

```python
from pocketdock import Container

c = Container(dockerfile="docker/Dockerfile", tag="qf-math-sizes")
c.run("bash /src/docker/build_sizes.sh", mount={"src": "."})
```

## Note on floats

`qf_math` is **float32**. Bare-metal objects still rely on **libgcc** soft-float helpers when you link a full image — the numbers here are **library `.text` only**, not total firmware flash.
