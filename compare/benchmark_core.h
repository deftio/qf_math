/**
 * benchmark_core.h — Shared loops for qf_math vs libm vs libfixmath vs fr_math.
 *
 * Used by compare/benchmark_suite.c (host) and examples/esp32s3_benchmark (MCU).
 *
 * Copyright (c) 2002-2026, M. A. Chatterjee — qf_math portions BSD-2-Clause.
 */

#ifndef BENCHMARK_CORE_H
#define BENCHMARK_CORE_H

#include <stdint.h>
#include <stdio.h>

#define BENCH_N_ITER    80000
#define BENCH_N_SAMPLES 8000
#define BENCH_FR_RX     16

typedef struct bench_timer {
    void (*init)(void *ctx);
    uint64_t (*now)(void *ctx);
    double (*elapsed_us)(void *ctx, uint64_t start, uint64_t end);
    void *ctx;
} bench_timer_t;

enum bench_func_idx {
    BENCH_F_SIN = 0,
    BENCH_F_COS,
    BENCH_F_SQRT,
    BENCH_F_LN,
    BENCH_F_EXP,
    BENCH_NFUNC
};

enum bench_lib_idx {
    BENCH_L_QF = 0,
    BENCH_L_LIBM,
    BENCH_L_FX,
    BENCH_L_FR,
    BENCH_L_POLY,
    BENCH_NLIB
};

/**
 * acc_pct[func][col]: columns qf, fx, fr, internal poly — sin/cos = max abs
 * err ×100; sqrt/ln/exp = max relative err ×100. Unused entries are NAN.
 * time_us[func][lib]: microseconds for the full timed loop (same shape as host suite).
 */
typedef struct bench_results {
    double acc_pct[BENCH_NFUNC][4];
    double time_us[BENCH_NFUNC][BENCH_NLIB];
} bench_results_t;

void bench_results_init(bench_results_t *r);

/** Run sin, cos, sqrt, ln, exp benchmarks (same grids & iteration counts as host compare/). */
void bench_run_all(const bench_timer_t *timer, volatile float *sink, bench_results_t *out);

double bench_ratio_vs_libm(double us_libm, double us_impl);

/** Human-readable report to stdout (UART on ESP-IDF). */
void bench_print_human(const bench_results_t *r);

/** Caption variant for Markdown tables (shared with host `make compare-github-report` body). */
typedef enum {
    BENCH_MD_HOST = 0,
    BENCH_MD_MCU  = 1,
} bench_md_style_t;

/**
 * Markdown only: accuracy / wall-clock us / vs-libm tables — same columns as `compare/BENCHMARK_REPORT.md`.
 */
void bench_emit_markdown_tables(FILE *out, const bench_results_t *r, bench_md_style_t style);

/**
 * HTML comment + ### MCU benchmark snapshot (metadata row) + --- + tables (MCU captions).
 * Paste UART capture into docs (see `compare/MCU_BENCHMARK_SNAPSHOT.md`).
 */
void bench_emit_markdown_doc_snapshot(FILE *out, const bench_results_t *r, const char *device_line);

#endif /* BENCHMARK_CORE_H */
