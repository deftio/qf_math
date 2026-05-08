# Peer libraries & references (survey)

Open-source and vendor ecosystems that overlap **qf_math** (small **float32** approximations and/or **fixed-point** kernels on MCUs). The portable host bench (`compare/benchmark_suite.c`) wires **libm**, **libfixmath**, and **fr_math**; the LilyGO ESP32-S3 firmware also appends a small ESP32-only fast-math peer section.

**`qf_math`** is **`float`-domain**; **`libfixmath`** and **`fr_math`** are **fixed-point**. The bench bridges the latter two so timing/error scores share one reference path — **not** a claim they are the same product class as **`qf_math`** (see **[README.md](README.md)** § Float vs fixed-point).

## Wired into `make compare` today

| Name | Link | Model | Notes |
|------|------|-------|--------|
| **fr_math** | [github.com/deftio/fr_math](https://github.com/deftio/fr_math) | Fixed radix `s32` | Integer-first; sibling design lineage to qf_math. |
| **libfixmath** | [github.com/PetteriAimonen/libfixmath](https://github.com/PetteriAimonen/libfixmath) | Q16.16 `fix16_t` | Ubiquitous embedded fixed math. |
| Host **libm** | toolchain | `float`/`double` | Reference timings / accuracy only (not “tiny ROM”). |

An internal Taylor **poly7+fmod** sanity check exists for `sin`, but it is not a maintained peer library and is intentionally omitted from the generated Markdown comparison tables.

**Bench wiring:** **`qf_math`** is exercised as **`float`** end-to-end; **`fr_math`** / **libfixmath** paths convert at the harness boundary for scoring — strip those wrappers when profiling native **`s32` / `fix16_t`** firmware.

## Wired into the LilyGO ESP32-S3 snapshot

These rows are printed by `examples/lilygo_t_display_s3_bench/` after the portable tables and are intended for on-device comparison only:

| Name | Link | Current harness coverage | Notes |
|------|------|--------------------------|-------|
| **FastTrig** | [RobTillaart/FastTrig](https://github.com/RobTillaart/FastTrig) | `sin`, `cos` | Pulled as a PlatformIO library; degree-based API is wrapped for radian inputs. |
| **ESP-DSP** | [espressif/esp-dsp](https://github.com/espressif/esp-dsp) | `sqrt` | Uses the scalar `dsps_sqrtf_f32_ansi` approximation locally; the full repo pulls demo apps under PlatformIO. |
| **espp/math** | [esp-cpp/espp](https://github.com/esp-cpp/espp) | `sin`, `cos`, `sqrt`, `ln` | Uses a local header-only subset of `components/math/include/fast_math.hpp`; the full component tree is too large for this Arduino benchmark dependency path. |

---

## Embedded / MCU stacks people actually cite

Compact trig, log, or full **single-precision** suites aimed at **small flash** or **soft-float** MCUs (C, assembly, or Arduino wrappers).

| Name | Link | Model | Why people care | License / caveat |
|------|------|-------|-----------------|------------------|
| **CMSIS-DSP** | [ARM-software/CMSIS-DSP](https://github.com/ARM-software/CMSIS-DSP) | `f32`, Q15, Q31 | Default **Arm** DSP path in many SDKs; **arm_sin_f32** et al. | Apache 2.0 |
| **ArmMathM0** | [TimPaterson/ArmMathM0](https://github.com/TimPaterson/ArmMathM0) | `float` tuned asm/C | Explicit **Cortex-M0/M0+** `sinf`/`cosf`/… smaller/faster than generic libm claims | MIT |
| **Qfplib** family | [quinapalus.com/qfplib.html](http://quinapalus.com/qfplib.html) · e.g. [mysterywolf/Qfplib-M0-tiny](https://github.com/mysterywolf/Qfplib-M0-tiny), [Qfplib-M3](https://github.com/mysterywolf/Qfplib-M3) | IEEE SP FP on **Cortex-M** | Very common **“replace soft-float / libm”** recommendation on forums; asm-heavy | **GPL-2.0** — review before shipping proprietary firmware |
| **FastTrig** | [RobTillaart/FastTrig](https://github.com/RobTillaart/FastTrig) | Tiny LUT + interp | Arduino ecosystem; trades accuracy for **~few hundred bytes** tables | MIT |
| **ESP-DSP** | [espressif/esp-dsp](https://github.com/espressif/esp-dsp) | ESP32 DSP kernels (`f32`, `s16`) | Official Espressif DSP library; optimized ESP32 implementations and math kernels such as sqrt / sin-cos generator paths | Apache 2.0 |
| **espp/math** | [ESP Component Registry: espp/math](https://components.espressif.com/components/espp/math) | ESP32 C++ fast math helpers | ESP-IDF component with `fast_sqrt`, `fast_ln`, `fast_sin`, `fast_cos` style approximations | MIT |
| **speedtrig** | [mathvav/speedtrig](https://github.com/mathvav/speedtrig) | Arduino trig shortcuts | Small AVR/Arduino niche; author points users at FastTrig for newer work | MIT |
| **q** | [github.com/howerj/q](https://github.com/howerj/q) | Q16.16 + CORDIC | Minimal deps; interesting for integer-only cores | See upstream |
| **openlibm** | [JuliaMath/openlibm](https://github.com/JuliaMath/openlibm) | Portable libm-quality | Quality reference / BSD-ish stacks—not ROM-minimal | BSD-like |

---

## Hosted / SIMD “fast math” (usually wrong ROM budget for tiny MCU)

High throughput on **x86/ARM NEON** with vector ISAs — great for desktops and application processors, rarely the same trade space as **16–128 KB** flash MCUs.

| Name | Link | Notes |
|------|------|--------|
| **SLEEF** | [sleef.org](https://sleef.org) · [shibatch/sleef](https://github.com/shibatch/sleef) | SIMD `float`/`double`; accurate & fast on servers / NEON / AVX |
| **micromath** | [tarcieri/micromath](https://github.com/tarcieri/micromath) | **Rust** `no_std` approximations (sin/cos/vectors/stats) |
| **math_intrinsics** | [Geolm/math_intrinsics](https://github.com/Geolm/math_intrinsics) | Header lib: cos/sin/acos… via **AVX/NEON**, branch-light |
| **FABE** | [farukalpay/FABE](https://github.com/farukalpay/FABE) | SIMD trig (AVX2/AVX-512/NEON); throughput-focused |

---

## Future coverage expansions

The ESP32-S3 run now includes FastTrig, ESP-DSP sqrt, and espp/math helpers. Remaining work is broader function coverage for those rows plus non-ESP32 libraries:

- **FastTrig** — broaden LilyGO coverage beyond `sin`/`cos` if `tan` / `atan2` accuracy ranges are useful.
- **ESP-DSP** — investigate meaningful scalar or generator-based `sin`/`cos` comparisons; today only `sqrt` is a direct scalar fit.
- **espp/math** — keep the local subset aligned with upstream `fast_math.hpp`; consider a true ESP-IDF component benchmark separately from Arduino/PlatformIO.
- **ArmMathM0** — strong Arm-only story for **float** MCUs; good for a Cortex-M0/M0+ target, not the ESP32-S3 run.
- **Qfplib** — well-known Cortex-M float replacement (`sin`, `cos`, `tan`, `atan`, `log`, `exp`, `sqrt`), but GPL-2.0/asm-heavy, so keep optional and clearly separated.
- **CMSIS-DSP** single-object extraction — useful on Arm (`arm_sin_f32`, `arm_cos_f32`, `arm_sqrt_f32`, vector log paths), but SDK/pack integration is harder to vendor uniformly.
- **emFloat** / vendor SDK math libs — popular commercial or vendor comparisons; document only unless licensing/toolchains make automated runs reproducible.

Pull requests that add **optional** Makefile targets (artifacts still under `build/compare/`) are welcome.
