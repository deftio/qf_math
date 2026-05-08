/**
 *  @file qf_math_bench.c — Speed and accuracy benchmark for qf_math vs libm
 *
 *  Measures wall-clock time and error (max absolute, mean absolute,
 *  RMS) for each qf_math function against the corresponding libm reference.
 *
 *  Invoke with `--html-table` to emit an HTML fragment for GitHub Pages (`make docs-pages`).
 *
 *  Copyright (c) 2002-2026, M. A. Chatterjee
 *  All rights reserved. BSD-2-Clause — see qf_math.h for full text.
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "qf_math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*=======================================================
 * Portable high-resolution timer
 */
#ifdef __APPLE__
#include <mach/mach_time.h>
static double timer_ns_per_tick = 0.0;
static void timer_init(void) {
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    timer_ns_per_tick = (double)info.numer / (double)info.denom;
}
static uint64_t timer_now(void) { return mach_absolute_time(); }
static double timer_elapsed_us(uint64_t start, uint64_t end) {
    return (double)(end - start) * timer_ns_per_tick / 1000.0;
}
#else
static void timer_init(void) {}
static uint64_t timer_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}
static double timer_elapsed_us(uint64_t start, uint64_t end) {
    return (double)(end - start) / 1000.0;
}
#endif

/*=======================================================
 * Error tracking
 */
typedef struct {
    double max_abs;
    double sum_abs;
    double sum_sq;
    int    count;
    float  worst_input;
} err_stats_t;

static void err_init(err_stats_t *e) {
    e->max_abs = 0.0;
    e->sum_abs = 0.0;
    e->sum_sq  = 0.0;
    e->count   = 0;
    e->worst_input = 0.0f;
}

static void err_add(err_stats_t *e, double err, float input) {
    double a = fabs(err);
    if (a > e->max_abs) { e->max_abs = a; e->worst_input = input; }
    e->sum_abs += a;
    e->sum_sq  += err * err;
    e->count++;
}

static double err_mean(err_stats_t *e) { return (e->count > 0) ? e->sum_abs / e->count : 0.0; }
static double err_rms(err_stats_t *e)  { return (e->count > 0) ? sqrt(e->sum_sq / e->count) : 0.0; }

/*=======================================================
 * Benchmark parameters
 */
#define N_ITER     100000   /* iterations for timing            */
#define N_SAMPLES  10000    /* samples for accuracy measurement */

/* Volatile sink to prevent dead-code elimination */
static volatile float g_sink;

typedef enum {
    SWEEP_TRIG_AMP,/* absolute error ×100 ⇒ % of ±1 output */
    SWEEP_REL,
    SWEEP_RAD_ERR,/* absolute rad error shown as % of π */
    SWEEP_ABS_LIN /* absolute Δ vs double on the bench grid */
} sweep_err_kind_t;

typedef struct {
    const char *fn_html;
    const char *metric_note;
    sweep_err_kind_t kind;
    err_stats_t      e;
    double           ms_qf;
    double           ms_libm;
} sweep_row_t;

static double bench_speed_ratio(double t_libm, double t_impl)
{
    if (t_impl < 1e-12)
        return 0.0;
    return t_libm / t_impl;
}

static void fmt_err_cells(sweep_err_kind_t k, err_stats_t *e, char *max_cell, size_t nm,
                          char *mean_cell, size_t ns)
{
    switch (k) {
        case SWEEP_TRIG_AMP:
            snprintf(max_cell, nm, "%.4g%% amp", e->max_abs * 100.0);
            snprintf(mean_cell, ns, "%.4g%% amp", err_mean(e) * 100.0);
            break;
        case SWEEP_REL:
            snprintf(max_cell, nm, "%.4g%% rel", e->max_abs * 100.0);
            snprintf(mean_cell, ns, "%.4g%% rel", err_mean(e) * 100.0);
            break;
        case SWEEP_RAD_ERR:
            snprintf(max_cell, nm, "%.4g%% of pi", e->max_abs / M_PI * 100.0);
            snprintf(mean_cell, ns, "%.4g%% of pi", err_mean(e) / M_PI * 100.0);
            break;
        case SWEEP_ABS_LIN:
            snprintf(max_cell, nm, "%.4g abs", e->max_abs);
            snprintf(mean_cell, ns, "%.4g abs", err_mean(e));
            break;
    }
}

/*=======================================================
 * Benchmarks
 */

static void meas_sin(sweep_row_t *row)
{
    float inputs[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++)
        inputs[i] = -6.28f + 12.56f * ((float)i / (float)N_SAMPLES);

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref = sin((double)inputs[i]);
        double got = (double)qf_sin(inputs[i]);
        err_add(&row->e, got - ref, inputs[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_sin(inputs[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = sinf(inputs[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "sin";
    row->metric_note = "radians; ref = sin in double";
    row->kind        = SWEEP_TRIG_AMP;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_sin(void)
{
    sweep_row_t w;
    meas_sin(&w);
    printf("  sin        %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_cos(sweep_row_t *row)
{
    float inputs[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++)
        inputs[i] = -6.28f + 12.56f * ((float)i / (float)N_SAMPLES);

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref = cos((double)inputs[i]);
        double got = (double)qf_cos(inputs[i]);
        err_add(&row->e, got - ref, inputs[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_cos(inputs[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = cosf(inputs[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "cos";
    row->metric_note = "radians; ref = cos in double";
    row->kind        = SWEEP_TRIG_AMP;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_cos(void)
{
    sweep_row_t w;
    meas_cos(&w);
    printf("  cos        %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_tan(sweep_row_t *row)
{
    float inputs[N_SAMPLES];
    int    n = 0;
    for (int i = 0; i < N_SAMPLES; i++) {
        float x = -1.5f + 3.0f * ((float)i / (float)N_SAMPLES);
        float d = fabsf(fabsf(x) - 1.5707963f);
        if (d > 0.05f)
            inputs[n++] = x;
    }

    err_init(&row->e);
    for (int i = 0; i < n; i++) {
        double ref = tan((double)inputs[i]);
        double got = (double)qf_tan(inputs[i]);
        err_add(&row->e, got - ref, inputs[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_tan(inputs[i % n]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = tanf(inputs[i % n]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "tan";
    row->metric_note = "excludes inputs within 0.05 rad of poles";
    row->kind        = SWEEP_TRIG_AMP;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_tan(void)
{
    sweep_row_t w;
    meas_tan(&w);
    printf("  tan        %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_acos(sweep_row_t *row)
{
    float inputs[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++)
        inputs[i] = -1.0f + 2.0f * ((float)i / (float)(N_SAMPLES - 1));

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref = acos((double)inputs[i]);
        double got = (double)qf_acos(inputs[i]);
        err_add(&row->e, got - ref, inputs[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_acos(inputs[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = acosf(inputs[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "acos";
    row->metric_note = "domain [-1, 1]; radians out";
    row->kind        = SWEEP_RAD_ERR;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_acos(void)
{
    sweep_row_t w;
    meas_acos(&w);
    printf("  acos       %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_atan2(sweep_row_t *row)
{
    float inputs_y[N_SAMPLES], inputs_x[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++) {
        float ang = -3.14f + 6.28f * ((float)i / (float)N_SAMPLES);
        inputs_x[i] = cosf(ang) * (1.0f + (float)(i % 100));
        inputs_y[i] = sinf(ang) * (1.0f + (float)(i % 100));
    }

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref = atan2((double)inputs_y[i], (double)inputs_x[i]);
        double got = (double)qf_atan2(inputs_y[i], inputs_x[i]);
        err_add(&row->e, got - ref, inputs_y[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_atan2(inputs_y[i % N_SAMPLES], inputs_x[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = atan2f(inputs_y[i % N_SAMPLES], inputs_x[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "atan2";
    row->metric_note = "swept (y,x) magnitudes; ref = atan2 double";
    row->kind        = SWEEP_RAD_ERR;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_atan2(void)
{
    sweep_row_t w;
    meas_atan2(&w);
    printf("  atan2      %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_log2(sweep_row_t *row)
{
    float inputs[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++)
        inputs[i] = 0.001f + 100.0f * ((float)i / (float)N_SAMPLES);

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref = log2((double)inputs[i]);
        double got = (double)qf_log2(inputs[i]);
        err_add(&row->e, got - ref, inputs[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_log2(inputs[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = log2f(inputs[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "log2";
    row->metric_note = "x in (0.001, 100]; absolute Δ vs double";
    row->kind        = SWEEP_ABS_LIN;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_log2(void)
{
    sweep_row_t w;
    meas_log2(&w);
    printf("  log2       %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_ln(sweep_row_t *row)
{
    float inputs[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++)
        inputs[i] = 0.001f + 100.0f * ((float)i / (float)N_SAMPLES);

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref = log((double)inputs[i]);
        double got = (double)qf_ln(inputs[i]);
        err_add(&row->e, got - ref, inputs[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_ln(inputs[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = logf(inputs[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "ln";
    row->metric_note = "x in (0.001, 100]; absolute Δ vs double";
    row->kind        = SWEEP_ABS_LIN;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_ln(void)
{
    sweep_row_t w;
    meas_ln(&w);
    printf("  ln         %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_exp(sweep_row_t *row)
{
    float inputs[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++)
        inputs[i] = -10.0f + 20.0f * ((float)i / (float)N_SAMPLES);

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref = exp((double)inputs[i]);
        double got = (double)qf_exp(inputs[i]);
        double rel = (ref != 0.0) ? (got - ref) / ref : (got - ref);
        err_add(&row->e, rel, inputs[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_exp(inputs[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = expf(inputs[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "exp";
    row->metric_note = "relative vs double on grid";
    row->kind        = SWEEP_REL;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_exp(void)
{
    sweep_row_t w;
    meas_exp(&w);
    printf("  exp  (rel) %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_pow2(sweep_row_t *row)
{
    float inputs[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++)
        inputs[i] = -10.0f + 20.0f * ((float)i / (float)N_SAMPLES);

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref = pow(2.0, (double)inputs[i]);
        double got = (double)qf_pow2(inputs[i]);
        double rel = (ref != 0.0) ? (got - ref) / ref : (got - ref);
        err_add(&row->e, rel, inputs[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_pow2(inputs[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = exp2f(inputs[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "pow2";
    row->metric_note = "relative vs double; libm ref = exp2f";
    row->kind        = SWEEP_REL;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_pow2(void)
{
    sweep_row_t w;
    meas_pow2(&w);
    printf("  pow2 (rel) %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_sqrt(sweep_row_t *row)
{
    float inputs[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++)
        inputs[i] = 0.001f + 10000.0f * ((float)i / (float)N_SAMPLES);

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref = sqrt((double)inputs[i]);
        double got = (double)qf_sqrt(inputs[i]);
        double rel = (ref != 0.0) ? (got - ref) / ref : (got - ref);
        err_add(&row->e, rel, inputs[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_sqrt(inputs[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = sqrtf(inputs[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "sqrt";
    row->metric_note = "relative vs double";
    row->kind        = SWEEP_REL;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_sqrt(void)
{
    sweep_row_t w;
    meas_sqrt(&w);
    printf("  sqrt (rel) %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_hypot(sweep_row_t *row)
{
    float inputs_x[N_SAMPLES], inputs_y[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++) {
        float ang = 6.28f * ((float)i / (float)N_SAMPLES);
        float r = 1.0f + 999.0f * ((float)(i % 100) / 100.0f);
        inputs_x[i] = cosf(ang) * r;
        inputs_y[i] = sinf(ang) * r;
    }

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref =
            sqrt((double)inputs_x[i] * inputs_x[i] + (double)inputs_y[i] * inputs_y[i]);
        double got = (double)qf_hypot(inputs_x[i], inputs_y[i]);
        double rel = (ref != 0.0) ? (got - ref) / ref : (got - ref);
        err_add(&row->e, rel, inputs_x[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_hypot(inputs_x[i % N_SAMPLES], inputs_y[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = hypotf(inputs_x[i % N_SAMPLES], inputs_y[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "hypot";
    row->metric_note = "relative vs double hypot";
    row->kind        = SWEEP_REL;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_hypot(void)
{
    sweep_row_t w;
    meas_hypot(&w);
    printf("  hypot(rel) %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void meas_hypot_fast8(sweep_row_t *row)
{
    float inputs_x[N_SAMPLES], inputs_y[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++) {
        float ang = 6.28f * ((float)i / (float)N_SAMPLES);
        float r = 1.0f + 999.0f * ((float)(i % 100) / 100.0f);
        inputs_x[i] = cosf(ang) * r;
        inputs_y[i] = sinf(ang) * r;
    }

    err_init(&row->e);
    for (int i = 0; i < N_SAMPLES; i++) {
        double ref =
            sqrt((double)inputs_x[i] * inputs_x[i] + (double)inputs_y[i] * inputs_y[i]);
        double got = (double)qf_hypot_fast8(inputs_x[i], inputs_y[i]);
        double rel = (ref != 0.0) ? (got - ref) / ref : (got - ref);
        err_add(&row->e, rel, inputs_x[i]);
    }

    uint64_t t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = qf_hypot_fast8(inputs_x[i % N_SAMPLES], inputs_y[i % N_SAMPLES]);
    uint64_t t1 = timer_now();
    double us_qf = timer_elapsed_us(t0, t1);

    t0 = timer_now();
    for (int r = 0; r < N_ITER; r++)
        for (int i = 0; i < 100; i++)
            g_sink = hypotf(inputs_x[i % N_SAMPLES], inputs_y[i % N_SAMPLES]);
    t1 = timer_now();
    double us_libm = timer_elapsed_us(t0, t1);

    row->fn_html     = "hypot_fast8";
    row->metric_note = "qf piecewise-linear; timed vs hypotf reference";
    row->kind        = SWEEP_REL;
    row->ms_qf       = us_qf / 1000.0;
    row->ms_libm     = us_libm / 1000.0;
}

static void bench_hypot_fast8(void)
{
    sweep_row_t w;
    meas_hypot_fast8(&w);
    printf("  hypot_f8   %10.2e  %10.2e  %10.2e  %8.1f  %8.1f  %5.1fx\n",
           w.e.max_abs, err_mean(&w.e), err_rms(&w.e),
           w.ms_qf, w.ms_libm, bench_speed_ratio(w.ms_libm, w.ms_qf));
}

static void emit_html_table(FILE *out)
{
    time_t     now = time(NULL);
    struct tm *utc = gmtime(&now);
    char       tbuf[64];

    strftime(tbuf, sizeof tbuf, "%Y-%m-%d %H:%M:%SZ", utc);
    fprintf(out,
            "<p class=\"qf-meta\"><small>Generated UTC <strong>%s</strong> — %d-sample grids, "
            "%d outer × 100-inner timing loops; error vs C <code>double</code>; libm = matching "
            "<code>*f</code> call. ",
            tbuf, N_SAMPLES, N_ITER);
#ifdef __VERSION__
    fprintf(out, "Compiler: <code>%s</code>.</small></p>\n", __VERSION__);
#else
    fprintf(out, "</small></p>\n");
#endif

    fprintf(out, "<h3>Accuracy sweep vs libm (<code>float</code>)</h3>\n");
    fprintf(out, "<table class=\"qf-table\"><thead><tr>"
                 "<th scope=\"col\">Function</th>"
                 "<th scope=\"col\">Bench note</th>"
                 "<th scope=\"col\">Max</th>"
                 "<th scope=\"col\">Mean</th>"
                 "<th scope=\"col\"><code>qf</code> ms</th>"
                 "<th scope=\"col\">libm ms</th>"
                 "<th scope=\"col\">libm ÷ qf</th>"
                 "</tr></thead>\n<tbody>\n");

    sweep_row_t rows[16];
    int         n = 0;
    meas_sin(&rows[n++]);
    meas_cos(&rows[n++]);
    meas_tan(&rows[n++]);
    meas_acos(&rows[n++]);
    meas_atan2(&rows[n++]);
    meas_log2(&rows[n++]);
    meas_ln(&rows[n++]);
    meas_pow2(&rows[n++]);
    meas_exp(&rows[n++]);
    meas_sqrt(&rows[n++]);
    meas_hypot(&rows[n++]);
    meas_hypot_fast8(&rows[n++]);

    for (int i = 0; i < n; i++) {
        char   mc[96], mn[96];
        double rat = bench_speed_ratio(rows[i].ms_libm, rows[i].ms_qf);
        fmt_err_cells(rows[i].kind, &rows[i].e, mc, sizeof mc, mn, sizeof mn);
        fprintf(out,
                "<tr><td><code>qf_%s()</code></td><td>%s</td><td>%s</td><td>%s</td>"
                "<td>%.2f</td><td>%.2f</td><td><strong>%.2f</strong>×</td></tr>\n",
                rows[i].fn_html, rows[i].metric_note, mc, mn, rows[i].ms_qf,
                rows[i].ms_libm, rat);
    }

    fprintf(out, "</tbody></table>\n");
    fprintf(out,
            "<p class=\"qf-footnote\"><small><strong>Error column:</strong> "
            "<em>amp</em> = %% of ±1 output (sin/cos/tan). "
            "<em>rel</em> = max relative error vs double × 100. "
            "<em>of pi</em> = absolute radian error as %% of π (acos, atan2). "
            "<em>abs</em> = absolute Δ vs double on the grid (log2, ln). "
            "<strong>libm ÷ qf</strong> uses total loop time; "
            "&gt; 1 means qf_math was faster on this host (microbenchmark only; profile on your MCU).</small></p>\n");
}

/*=======================================================
 * Main
 */

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--html-table") == 0) {
        timer_init();
        emit_html_table(stdout);
        return 0;
    }

    timer_init();

    printf("qf_math Speed & Accuracy Benchmark\n");
    printf("================================\n");
    printf("Platform: %s, %lu-bit pointers\n",
#ifdef __aarch64__
           "arm64"
#elif defined(__x86_64__)
           "x86_64"
#else
           "unknown"
#endif
           , sizeof(void*) * 8UL);
    printf("Iterations: %d x 100 calls per function\n", N_ITER);
    printf("Accuracy samples: %d\n\n", N_SAMPLES);

    printf("               -------- Error --------  --- Time (ms) ---\n");
    printf("  Function     Max Abs    Mean Abs   RMS         qf_math    libm  Ratio\n");
    printf("  -----------  ---------  ---------  ---------  -------  -------  -----\n");

    bench_sin();
    bench_cos();
    bench_tan();
    bench_acos();
    bench_atan2();
    bench_log2();
    bench_ln();
    bench_pow2();
    bench_exp();
    bench_sqrt();
    bench_hypot();
    bench_hypot_fast8();

    printf("\n");
    printf("Notes:\n");
    printf("  - Trig error is absolute (output range [-1,1]).\n");
    printf("  - exp/pow2/sqrt/hypot error is relative.\n");
    printf("  - tan excludes inputs within 0.05 rad of poles.\n");
    printf("  - Ratio > 1 means qf_math is faster than libm.\n");
    printf("  - This is a desktop benchmark. On MCUs without HW divider,\n");
    printf("    qf_math's table lookups gain more advantage over libm's\n");
    printf("    polynomial approximations.\n");

    return 0;
}
