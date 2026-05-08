---

## Interpretation

- **Timings** are from a **desktop-class host** (`sinf`/`logf`/… reference). On Cortex-M (with or without FPU), soft-float costs, flash wait states, and IRQ latency dominate—**re-profile on your MCU**.
- **`qf_math`** timings reflect **`float`** paths; **libfixmath** / **fr_math** are **fixed-point** libraries measured here **via float↔fixed bridges** for uniform double-reference scoring — not “three floats racing.” Native fixed-only call paths drop most conversion overhead.
- **“vs libm”** is `t_libm / t_impl` for the measured loop (values **greater than 1** mean the implementation was faster than host libm in this microbenchmark).
- **Footprint table**: each row is **one library’s object code** — you normally ship **one** math stack; the harness links several **only** for side‑by‑side measurement (don’t add the byte counts together as “total firmware”).
