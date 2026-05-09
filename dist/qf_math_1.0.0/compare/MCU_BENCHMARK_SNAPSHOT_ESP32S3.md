# MCU benchmark snapshot

_Generated:_ **2026-05-09 05:35:25Z** (`tools/mcu_benchmark_snapshot.py` — LilyGO / ESP32-S3 Arduino bench).

Tables match [`BENCHMARK_REPORT.md`](BENCHMARK_REPORT.md) sections (accuracy %, wall-clock microseconds, speed vs libm). Loop shape and grids match host **`make compare`** (`benchmark_core.c`). MCU timing uses **`esp_timer_get_time()`**.

Firmware: [`examples/lilygo_t_display_s3_bench/`](../examples/lilygo_t_display_s3_bench/README.md).

Regenerate:

```bash
make compare-deps
# ESP32-S3 default:
make mcu-benchmark-snapshot
# Other boards: pass --project, --label, --timer, and --out explicitly.
```

---

### MCU benchmark snapshot

| Field | Value |
| --- | --- |
| Device | LilyGO T-Display-S3 · Arduino-ESP32 · esp_chip model=9 cores=2 revision=0 |
| qf_math | 1.0.0 (`QF_MATH_VERSION_HEX`=0x10000) |
| Loop shape | 8000 sample grid × 8000 outer × 64 inner calls |

---

### Accuracy — peak error

One generated matrix for all benchmarked functions over 8000 sample points. Sine/cosine and tangent rad/deg rows sweep signed `-rotation..+rotation` inputs; BAM rows sweep one unsigned cycle. Tangent pole samples use each implementation family’s saturation magnitude (`QF_TAN_MAX` for qf/float peers, fixed-point range for fixed-point peers), and fixed-point tangent peers are scored against the quantized bridge-domain angle. The `Metric` column defines each peak value: `abs %FS` for sine/cosine output amplitude, `abs` for tangent, `abs rad` for inverse trig, and `rel %` for sqrt/hypot/log/exp. Bold marks the best non-`libm` approximation in each row.

| Function | Metric | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | abs %FS | 0.000000 | **0.001884** | 0.775155 | 0.007801 | 0.015257 | --- | 0.388743 |
| `sin_deg` | abs %FS | 0.000000 | **0.001883** | 0.775144 | 0.006335 | 0.015247 | --- | 0.388747 |
| `sin_bam` | abs %FS | 0.000000 | **0.001885** | 0.775155 | 0.004341 | 0.014338 | --- | 0.393073 |
| `cos_rad` | abs %FS | 0.000000 | **0.001886** | 0.775151 | 0.008672 | 0.015248 | --- | 0.388739 |
| `cos_deg` | abs %FS | 0.000000 | **0.001883** | 0.775150 | 0.006432 | 0.015250 | --- | 0.388746 |
| `cos_bam` | abs %FS | 0.000000 | **0.001885** | 0.775151 | 0.004342 | 0.015656 | --- | 0.393077 |
| `tan_rad` | abs | 0.000000 | 0.300599 | 20.376813 | **0.051472** | 0.744179 | --- | --- |
| `tan_deg` | abs | 0.000000 | **0.037535** | 20.376813 | 1.007205 | 0.751325 | --- | --- |
| `tan_bam` | abs | 0.000000 | **0.222870** | 20.325424 | 2.452361 | 1.442062 | --- | --- |
| `asin` | abs rad | 0.000000 | **0.000231** | 0.010209 | 0.000361 | 0.002611 | --- | --- |
| `acos` | abs rad | 0.000000 | **0.000232** | 0.010220 | 0.000357 | 0.002611 | --- | --- |
| `atan` | abs rad | 0.000000 | **0.000190** | 0.010158 | 0.000950 | 0.000609 | --- | --- |
| `atan2` | abs rad | 0.000000 | **0.000000** | 0.000013 | 0.000314 | 0.000608 | --- | --- |
| `sqrt` | rel % | 0.000000 | **0.000472** | 0.380124 | 1.189381 | --- | 3.515928 | 0.175208 |
| `hypot` | rel % | 0.000000 | 0.000470 | --- | **0.000007** | --- | --- | --- |
| `hypot_fast2` | rel % | 0.000000 | **1.408747** | --- | --- | 2.633407 | --- | --- |
| `hypot_fast` | rel % | 0.000000 | **0.137249** | --- | 0.137249 | --- | --- | --- |
| `ln` | rel % | 0.000000 | 0.271975 | **0.086993** | 0.398790 | --- | --- | 6.536268 |
| `exp` | rel % | 0.000000 | **0.001504** | 0.286388 | 0.574226 | --- | --- | --- |

### Accuracy — mean squared error

Mean squared error uses the same metric units as the peak-error table, squared. Bold marks the best non-`libm` approximation in each row.

| Function | Metric squared | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | abs %FS^2 | 0.000000 | **0.000001** | 0.022074 | 0.000006 | 0.000016 | --- | 0.025967 |
| `sin_deg` | abs %FS^2 | 0.000000 | **0.000001** | 0.022074 | 0.000005 | 0.000016 | --- | 0.025968 |
| `sin_bam` | abs %FS^2 | 0.000000 | **0.000001** | 0.022069 | 0.000002 | 0.000016 | --- | 0.025969 |
| `cos_rad` | abs %FS^2 | 0.000000 | **0.000001** | 0.022133 | 0.000006 | 0.000014 | --- | 0.025967 |
| `cos_deg` | abs %FS^2 | 0.000000 | **0.000001** | 0.022132 | 0.000006 | 0.000013 | --- | 0.025968 |
| `cos_bam` | abs %FS^2 | 0.000000 | **0.000001** | 0.022094 | 0.000002 | 0.000010 | --- | 0.025969 |
| `tan_rad` | abs^2 | 0.000000 | 0.000027 | 0.753949 | **0.000008** | 0.000849 | --- | --- |
| `tan_deg` | abs^2 | 0.000000 | **0.000001** | 0.753950 | 0.001127 | 0.000845 | --- | --- |
| `tan_bam` | abs^2 | 0.000000 | **0.000013** | 0.704843 | 0.004254 | 0.001584 | --- | --- |
| `asin` | abs rad^2 | 0.000000 | **0.000000** | 0.000023 | 0.000000 | 0.000000 | --- | --- |
| `acos` | abs rad^2 | 0.000000 | **0.000000** | 0.000023 | 0.000000 | 0.000000 | --- | --- |
| `atan` | abs rad^2 | 0.000000 | **0.000000** | 0.000047 | 0.000000 | 0.000000 | --- | --- |
| `atan2` | abs rad^2 | 0.000000 | **0.000000** | 0.000000 | 0.000000 | 0.000000 | --- | --- |
| `sqrt` | rel %^2 | 0.000000 | **0.000000** | 0.000018 | 0.000177 | --- | 3.841045 | 0.012125 |
| `hypot` | rel %^2 | 0.000000 | 0.000000 | --- | **0.000000** | --- | --- | --- |
| `hypot_fast2` | rel %^2 | 0.000000 | **0.519433** | --- | --- | 3.021117 | --- | --- |
| `hypot_fast` | rel %^2 | 0.000000 | 0.003018 | --- | **0.003018** | --- | --- | --- |
| `ln` | rel %^2 | 0.000000 | 0.000016 | **0.000001** | 0.000043 | --- | --- | 0.006856 |
| `exp` | rel %^2 | 0.000000 | **0.000001** | 0.001315 | 0.005274 | --- | --- | --- |

### Wall-clock time (microseconds)

Total microseconds normalized to the metadata loop shape. ESP32-only peer rows may be measured with a shorter loop and scaled to the same outer-iteration count. Unsupported cells are `---`.

| Function | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | 500037.000000 | **116170.000000** | 640541.000000 | 378932.000000 | 240620.000000 | --- | 258240.000000 |
| `sin_deg` | 541014.000000 | **120533.000000** | 677248.000000 | 400972.000000 | 223400.000000 | --- | 294650.000000 |
| `sin_bam` | 265359.000000 | **129125.000000** | 683756.000000 | 160908.000000 | 249040.000000 | --- | 264580.000000 |
| `cos_rad` | 500122.000000 | **114016.000000** | 657978.000000 | 553675.000000 | 292110.000000 | --- | 324820.000000 |
| `cos_deg` | 545270.000000 | **122681.000000** | 703302.000000 | 485308.000000 | 275000.000000 | --- | 361330.000000 |
| `cos_bam` | 301254.000000 | **129126.000000** | 709637.000000 | 194816.000000 | 281330.000000 | --- | 331190.000000 |
| `tan_rad` | 635623.000000 | **178588.000000** | 1510331.000000 | 485865.000000 | 414390.000000 | --- | --- |
| `tan_deg` | 652780.000000 | **178587.000000** | 1546832.000000 | 518516.000000 | 397230.000000 | --- | --- |
| `tan_bam` | 409371.000000 | 176429.000000 | 1545451.000000 | **169500.000000** | 409890.000000 | --- | --- |
| `asin` | 619106.000000 | **561219.000000** | 1879308.000000 | 802820.000000 | 623160.000000 | --- | --- |
| `acos` | 584811.000000 | **524594.000000** | 1905410.000000 | 755478.000000 | 659720.000000 | --- | --- |
| `atan` | 488085.000000 | 406354.000000 | 880861.000000 | 1102370.000000 | **298900.000000** | --- | --- |
| `atan2` | 801881.000000 | 568423.000000 | 632691.000000 | 1140317.000000 | **395530.000000** | --- | --- |
| `sqrt` | 161312.000000 | 150564.000000 | 689419.000000 | 1900558.000000 | --- | **47430.000000** | 105510.000000 |
| `hypot` | 524412.000000 | **208554.000000** | --- | 2448086.000000 | --- | --- | --- |
| `hypot_fast2` | 524409.000000 | 148361.000000 | --- | --- | **131180.000000** | --- | --- |
| `hypot_fast` | 524416.000000 | **197763.000000** | --- | 197837.000000 | --- | --- | --- |
| `ln` | 468729.000000 | 212894.000000 | 35403726.000000 | 509092.000000 | --- | --- | **124870.000000** |
| `exp` | 492336.000000 | **187135.000000** | 8357724.000000 | 262397.000000 | --- | --- | --- |

### Speed vs libm (ratio)

`libm` time ÷ implementation time for the same function on this platform. Above 1.0 means faster than the platform `libm` call in this microbenchmark. Ratios are rounded to two decimal places because smaller differences are usually noise.

**Fixed-point peer rows are bridged harness timings**: `float`→fixed→function→`float`. They keep the report comparable, but are not native `fix16_t` / `s32` pipeline timings.

| Function | libm | **qf_math** | **libfixmath** (float bridge) | **fr_math** (float bridge) | **FastTrig** | **ESP-DSP** | **espp/math** |
| :--- | ---:| ---:| ---:| ---:| ---:| ---:| ---:|
| `sin_rad` | 1.00 | **4.30** | 0.78 | 1.32 | 2.08 | --- | 1.94 |
| `sin_deg` | 1.00 | **4.49** | 0.80 | 1.35 | 2.42 | --- | 1.84 |
| `sin_bam` | 1.00 | **2.06** | 0.39 | 1.65 | 1.07 | --- | 1.00 |
| `cos_rad` | 1.00 | **4.39** | 0.76 | 0.90 | 1.71 | --- | 1.54 |
| `cos_deg` | 1.00 | **4.44** | 0.78 | 1.12 | 1.98 | --- | 1.51 |
| `cos_bam` | 1.00 | **2.33** | 0.42 | 1.55 | 1.07 | --- | 0.91 |
| `tan_rad` | 1.00 | **3.56** | 0.42 | 1.31 | 1.53 | --- | --- |
| `tan_deg` | 1.00 | **3.66** | 0.42 | 1.26 | 1.64 | --- | --- |
| `tan_bam` | 1.00 | 2.32 | 0.26 | **2.42** | 1.00 | --- | --- |
| `asin` | 1.00 | **1.10** | 0.33 | 0.77 | 0.99 | --- | --- |
| `acos` | 1.00 | **1.11** | 0.31 | 0.77 | 0.89 | --- | --- |
| `atan` | 1.00 | 1.20 | 0.55 | 0.44 | **1.63** | --- | --- |
| `atan2` | 1.00 | 1.41 | 1.27 | 0.70 | **2.03** | --- | --- |
| `sqrt` | 1.00 | 1.07 | 0.23 | 0.08 | --- | **3.40** | 1.53 |
| `hypot` | 1.00 | **2.51** | --- | 0.21 | --- | --- | --- |
| `hypot_fast2` | 1.00 | 3.53 | --- | --- | **4.00** | --- | --- |
| `hypot_fast` | 1.00 | **2.65** | --- | 2.65 | --- | --- | --- |
| `ln` | 1.00 | 2.20 | 0.01 | 0.92 | --- | --- | **3.75** |
| `exp` | 1.00 | **2.63** | 0.06 | 1.88 | --- | --- | --- |
