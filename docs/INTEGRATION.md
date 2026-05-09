# Integration Guide

How to add qf_math to your project.

---

## Plain C (manual)

Copy `src/qf_math.c` and `src/qf_math.h` into your project. Compile `qf_math.c` as a single translation unit alongside your application code.

```bash
gcc -std=c99 -Os -c qf_math.c -o qf_math.o
gcc -std=c99 -Os main.c qf_math.o -o main
```

No external dependencies are required for the library itself. The `-lm` flag is only needed for tests and benchmarks (which use `sinf`, `logf`, etc. as reference implementations).

```c
#include "qf_math.h"

qf y = qf_sin(1.0f);
qf len = qf_hypot(dx, dy);
```

---

## CMake

The root `CMakeLists.txt` is configured for ESP-IDF component registration. For standalone CMake projects, add qf_math as a subdirectory or create a simple target:

```cmake
add_library(qf_math STATIC src/qf_math.c)
target_include_directories(qf_math PUBLIC src/)
```

Then link against it:

```cmake
target_link_libraries(your_app PRIVATE qf_math)
```

### Lean build

To compile the reduced-footprint subset (no log10, pow10, waves, ADSR):

```cmake
target_compile_definitions(qf_math PRIVATE QF_MATH_LEAN)
```

---

## PlatformIO

The repo includes a `library.json` at the root. Add qf_math as a library dependency in your `platformio.ini`:

```ini
[env:your_board]
lib_deps =
    deftio/qf_math
```

Or reference the git repo directly:

```ini
lib_deps =
    https://github.com/deftio/qf_math.git
```

Headers resolve from `src/`:

```cpp
#include "qf_math.h"    // C API
#include "qf_math.hpp"   // C++ wrapper (optional)
```

Lean subset (omit `log10`, `pow10`, waves, ADSR):

```ini
build_flags =
    -DQF_MATH_LEAN
```

Arduino IDE installs also see [`library.properties`](../library.properties) (`src/` layout).

Publish check (optional): [`pio pkg pack`](https://docs.platformio.org/en/latest/core/userguide/pkg/cmd_pack.html) in the repo root must succeed.

---

## ESP-IDF component

The repo includes `idf_component.yml` for the ESP-IDF Component Manager.

### Via component manager

Add to your project's `idf_component.yml`:

```yaml
dependencies:
  deftio/qf_math:
    version: ">=1.0.0"
```

### Via git submodule or local path

Add the qf_math repo as a component in your project's `components/` directory:

```
components/
  qf_math/
    src/
      qf_math.c
      qf_math.h
    CMakeLists.txt
    idf_component.yml
```

The root `CMakeLists.txt` registers `src/qf_math.c` with include path `src/`. Compile flags use `-Wall`/`-Wextra`/`-Wshadow`/`-Wconversion` **without** `-Werror`, so downstream apps are less likely to break on toolchain or IDF churn.

Then include from any application component:

```c
#include "qf_math.h"
```

---

## Arduino (from source)

Copy `src/qf_math.c`, `src/qf_math.h`, and optionally `src/qf_math.hpp` into your Arduino sketch folder or a local library directory.

For Arduino IDE, the `.c` file will be compiled automatically. Include as usual:

```cpp
#include "qf_math.h"

void setup() {
    float y = qf_sin(1.0f);
}
```

For the C++ wrapper:

```cpp
#include "qf_math.hpp"

void setup() {
    auto y = qf_math::sin(1.0f);
}
```

---

## C++ usage via qf_math.hpp

The header-only C++ wrapper lives at `src/qf_math.hpp`. It includes `qf_math.h` automatically and wraps the C API in `namespace qf_math`.

```cpp
#include "qf_math.hpp"

// Free functions
auto y = qf_math::sin(1.0f);
auto len = qf_math::hypot(3.0f, 4.0f);
auto db = qf_math::log2(amplitude);

// ADSR envelope (class wrapper)
qf_math::adsr env(1000, 500, 0.7f, 2000);
env.trigger();
for (int i = 0; i < n; i++) {
    sample[i] *= env.step();
}
env.release();
```

The wrapper adds no storage, allocation, exceptions, RTTI, or runtime dependencies beyond the C library.

---

## Compiler flags

Recommended flags for the library:

```
-std=c99 -Os -Wall -Wextra -Wpedantic
```

For C++:

```
-std=c++11 -Os -Wall -Wextra -Wpedantic
```

### Optional defines

| Define | Effect |
|--------|--------|
| `QF_MATH_LEAN` | Exclude log10, pow10, wave generators, ADSR (smaller footprint) |
| `QF_MATH_COVERAGE` | Expose internal test helpers (used by `make coverage`) |
