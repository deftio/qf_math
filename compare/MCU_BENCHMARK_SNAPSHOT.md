# MCU benchmark snapshot

_Generated:_ **2026-05-08 07:27:38Z** (`tools/mcu_benchmark_snapshot.py` — LilyGO / ESP32-S3 Arduino bench).

Tables match [`BENCHMARK_REPORT.md`](BENCHMARK_REPORT.md) sections (accuracy %, wall-clock microseconds, speed vs libm). Loop shape and grids match host **`make compare`** (`benchmark_core.c`). MCU timing uses **`esp_timer_get_time()`**.

Firmware: [`examples/lilygo_t_display_s3_bench/`](../examples/lilygo_t_display_s3_bench/README.md).

Regenerate:

```bash
make compare-deps
make mcu-benchmark-snapshot
# or: MCU_SERIAL_PORT=/dev/cu.usbmodem2101 python3 tools/mcu_benchmark_snapshot.py
```

---

### MCU benchmark snapshot

| Field | Value |
| --- | --- |
| Device | LilyGO T-Display-S3 · Arduino-ESP32 · esp_chip model=9 cores=2 revision=0 |
| qf_math | 1.0.0 (`QF_MATH_VERSION_HEX`=0x10000) |
| Loop shape | 8000 sample grid × 80000 outer × 64 inner calls |

---

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

Total microseconds for each benchmark loop (**MCU wall-clock**; timer is `esp_timer_get_time()` or host monotonic equivalent).

| Library | sin | cos | sqrt | ln | exp |
| :--- | ---:| ---:| ---:| ---:| ---:|
| **qf_math** | 3925555.000000 | 3817697.000000 | 1505073.000000 | 2149931.000000 | 1913128.000000 |
| **libm** (`sinf` …) | 5022062.000000 | 5064616.000000 | 1634081.000000 | 4708225.000000 | 4944292.000000 |
| **libfixmath** (float bridge) | 6081875.000000 | 6384041.000000 | 6915334.000000 | 354061770.000000 | 83601277.000000 |
| **fr_math** (float bridge) | 3853344.000000 | 5579354.000000 | 19004972.000000 | 5111826.000000 | 2644626.000000 |

### Speed vs libm (ratio)

`libm` time ÷ implementation time for the same loop — **above 1.0** means faster than **`sinf`/`cosf`/… on this MCU** in this microbenchmark.

**Fixed-point peer rows are bridged harness timings**: `float`→fixed→function→`float`. They keep the report comparable, but are not native `fix16_t` / `s32` pipeline timings.

| Library | sin | cos | sqrt | ln | exp |
| :--- | ---:| ---:| ---:| ---:| ---:|
| **qf_math** | 1.279325 | 1.326615 | 1.085715 | 2.189942 | 2.584402 |
| libm (reference) | 1.000000 | 1.000000 | 1.000000 | 1.000000 | 1.000000 |
| **libfixmath** (float bridge) | 0.825742 | 0.793324 | 0.236298 | 0.013298 | 0.059141 |
| **fr_math** (float bridge) | 1.303300 | 0.907742 | 0.085982 | 0.921046 | 1.869562 |
