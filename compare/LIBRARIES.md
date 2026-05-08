# Library comparison — platforms, representation, footprint

**Domains:** **`qf_math`** is **`float32`** (IEEE SP tables + interpolation). **`libfixmath`** (`fix16_t`) and **`fr_math`** (radix-scaled `s32`) are **fixed-point** — overlapping problem space, **different ABI and rounding story**. The compare harness scores fixed stacks **through float wrappers** on the host so every implementation shares the same double reference; shipping firmware normally commits to **one** domain end-to-end (see **[README.md](README.md)** § Float vs fixed-point).

Legend for the **Footprint** column: qualitative ROM estimate for the *math-heavy slice people actually link* (single precision / common fixed kernels). Your linker script, `-ffunction-sections`, and CPU/FPU dominate final numbers—use `make compare-report` for concrete `.o` dec totals on your host build.

| Library | Numeric model | Works without HW FPU | Typical platforms | License | Footprint (rule of thumb) |
|---------|----------------|----------------------|-------------------|---------|---------------------------|
| **qf_math** (this repo) | IEEE `float32` tables + interpolation | Needs soft-float or native FPU | Any C99 toolchain with `float` | BSD-2-Clause | ~5–7 KB class for `qf_math.o` @ `-Os` (see `make size`) |
| **fr_math** | Signed integer fixed-radix (`s32`, caller radix) | Yes (integer-only path) | ARM/RISC-V/AVR-class MCUs | BSD-2-Clause (see upstream `LICENSE` / headers) | ~4–9 KB class for `FR_math.c` depending on flags (`FR_NO_PRINT`, waves, …) |
| **libfixmath** | Q16.16 `fix16_t` | Yes | Very broad; optional `FIXMATH_NO_64BIT` paths | MIT | ~few KB core + optional LUT/cache compile switches |
| **CMSIS-DSP** | `float32`, Q15, Q31 kernels | Integer kernels yes | **Arm Cortex-M/A** primarily | Apache 2.0 | Wide range: single `arm_sin_f32` tiny vs full DSP lib large |
| **C standard libm** | `double`/`float` (vendor) | Needs soft-float/FPU | Hosted + many embedded toolchains | toolchain-dependent | Often largest & slowest on small MCUs |
| **TI IQmath / CMSIS NN variants** | Vendor-specific fixed formats | Yes | TI C2000 / TMS320C6x / designated TI MCUs | TI proprietary / BSP license | Vendor ROM tables |
| **fastapprox** (Nicolas Capens et al., header-style snippets) | `float` polynomials | Needs float | GPU-ish / PC demos; rare on tiny MCUs | Mixed (check file headers) | Extremely small per function |
| **ArmMathM0** ([TimPaterson](https://github.com/TimPaterson/ArmMathM0)) | `float` routines tuned for **M0/M0+** | Needs float | Cortex-M0/M0+ | MIT | Compact `sinf`/`cosf`/… vs generic libm |
| **Qfplib** ([quinapalus](http://quinapalus.com/qfplib.html); e.g. [Qfplib-M0-tiny](https://github.com/mysterywolf/Qfplib-M0-tiny), [Qfplib-M3](https://github.com/mysterywolf/Qfplib-M3)) | IEEE SP FP routines (asm-heavy) | Needs float | Cortex-M (forum favorite vs soft-float) | **GPL-2.0** — compliance review for closed products | ~1 KB “tiny” to ~12 KB “full” class claims |
| **FastTrig** ([RobTillaart](https://github.com/RobTillaart/FastTrig)) | Small LUT + interpolation | Integer-friendly paths | Arduino-class MCUs | MIT | ~hundreds of bytes class |
| **speedtrig** ([mathvav/speedtrig](https://github.com/mathvav/speedtrig)) | Arduino trig helpers | Integer-ish | AVR/Arduino niche | MIT | Very small; upstream suggests FastTrig for new designs |
| **q** ([howerj](https://github.com/howerj/q)) | Q16.16 + CORDIC-style transcendentals | Yes | Portable embedded | See upstream `LICENSE` | Minimal deps |
| **micromath** ([tarcieri/micromath](https://github.com/tarcieri/micromath)) | Rust `no_std` approx (trig, vectors, stats) | Needs float (`no_std`) | **Embedded Rust** | Apache-2.0 / MIT | Sized for `no_std`, not libm parity |
| **openlibm** ([JuliaMath/openlibm](https://github.com/JuliaMath/openlibm)) | Portable libm-grade `float`/`double` | Needs soft-float/FPU | Hosted + many BSPs | BSD-style | Quality reference—not “minimal ROM” oriented |
| **SLEEF** ([sleef.org](https://sleef.org)) | SIMD-accelerated `float`/`double` | Needs SIMD-capable host | Servers / desktops / SIMD SoCs | BSL-1.0 | Large vs MCU ROM budgets |
| **math_intrinsics** · **FABE** (see **[PEERS.md](PEERS.md)**) | Header/SIMD trig kernels | NEON / AVX… | Application processors | varies | Throughput-first; different problem than 64 KB flash MCUs |

Further links and “future bench” notes: **[PEERS.md](PEERS.md)**.

| Artifact | Role |
|----------|------|
| `compare/benchmark_suite.c` | Measures **qf_math** (native **`float`**), **libm**, **libfixmath** (**fixed**, float bridge), **fr_math** (**fixed**, float bridge), plus a naïve Taylor **poly** baseline. |
| `compare/report_sizes.sh` | Prints **per-library** `.o` totals (host build). Rows are **not** summed—pick one stack for real firmware. |
| `build/compare/third_party/*` | Ephemeral clones—never committed. |

## Peer relationships worth noting

- **qf_math** is the floating companion to **fr_math**: shared algorithm heritage (BAM phases, table layouts, `hypot_fast8`, waves/ADSR concepts). Pair **`qf_math`** at FP boundaries with **`fr_math`** deep inside ISR-heavy integer cores when you need strict determinism.
