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

#define BENCH_N_ITER       8000
#define BENCH_N_SAMPLES    8000
#define BENCH_PEER_N_ITER  800
#define BENCH_FR_RX        16

typedef struct bench_timer {
    void (*init)(void *ctx);
    uint64_t (*now)(void *ctx);
    double (*elapsed_us)(void *ctx, uint64_t start, uint64_t end);
    void *ctx;
} bench_timer_t;

enum bench_func_idx {
    BENCH_F_SIN = 0,
    BENCH_F_SIN_DEG,
    BENCH_F_SIN_BAM,
    BENCH_F_COS,
    BENCH_F_COS_DEG,
    BENCH_F_COS_BAM,
    BENCH_F_TAN,
    BENCH_F_TAN_DEG,
    BENCH_F_TAN_BAM,
    BENCH_F_ASIN,
    BENCH_F_ACOS,
    BENCH_F_ATAN,
    BENCH_F_ATAN2,
    BENCH_F_SQRT,
    BENCH_F_HYPOT,
    BENCH_F_HYPOT_FAST2,
    BENCH_F_HYPOT_FAST,
    BENCH_F_LOG2,
    BENCH_F_LN,
    BENCH_F_LOG10,
    BENCH_F_POW2,
    BENCH_F_EXP,
    BENCH_F_POW10,
    BENCH_F_POW,
    BENCH_NFUNC
};

enum bench_lib_idx {
    BENCH_L_QF = 0,
    BENCH_L_LIBM,
    BENCH_L_FX,
    BENCH_L_FR,
    BENCH_L_FASTTRIG,
    BENCH_L_ESPDSP,
    BENCH_L_ESPP,
    BENCH_NLIB
};

/**
 * acc[func][lib]: max error in the metric named by bench_func_metric(func).
 * mse[func][lib]: mean squared error in that same metric.
 * time_us[func][lib]: microseconds for the timed loop. Unsupported entries are NAN.
 */
typedef struct bench_results {
    double acc[BENCH_NFUNC][BENCH_NLIB];
    double mse[BENCH_NFUNC][BENCH_NLIB];
    double time_us[BENCH_NFUNC][BENCH_NLIB];
} bench_results_t;

typedef float (*bench_unary_fn_t)(float);
typedef float (*bench_binary_fn_t)(float, float);

void bench_results_init(bench_results_t *r);

/** Run the portable qf_math/libm/libfixmath/fr_math matrix. */
void bench_run_all(const bench_timer_t *timer, volatile float *sink, bench_results_t *out);

/** Fill one optional unary or binary peer cell using the shared grids/metrics. */
void bench_run_unary_peer(const bench_timer_t *timer,
                          volatile float *sink,
                          bench_results_t *out,
                          int lib_idx,
                          int func_idx,
                          bench_unary_fn_t fn);
void bench_run_binary_peer(const bench_timer_t *timer,
                           volatile float *sink,
                           bench_results_t *out,
                           int lib_idx,
                           int func_idx,
                           bench_binary_fn_t fn);

double bench_ratio_vs_libm(double us_libm, double us_impl);

const char *bench_func_name(int func_idx);
const char *bench_func_metric(int func_idx);
const char *bench_lib_name(int lib_idx);

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
 * Paste UART capture into docs (see `compare/MCU_BENCHMARK_SNAPSHOT_<TARGET>.md`).
 */
void bench_emit_markdown_doc_snapshot(FILE *out, const bench_results_t *r, const char *device_line);

#endif /* BENCHMARK_CORE_H */
