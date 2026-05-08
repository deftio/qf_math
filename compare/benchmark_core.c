/**
 * benchmark_core.c — Shared benchmark kernels (host + MCU).
 *
 * Copyright (c) 2002-2026, M. A. Chatterjee — qf_math portions BSD-2-Clause.
 */

#include "benchmark_core.h"

#include <math.h>
#include <stdio.h>

#include "FR_math.h"
#include "fix16.h"
#include "qf_math.h"

#define FR_SCAL ((float)(1u << BENCH_FR_RX))

void bench_results_init(bench_results_t *r)
{
    for (int f = 0; f < BENCH_NFUNC; f++) {
        for (int c = 0; c < 4; c++)
            r->acc_pct[f][c] = NAN;
        for (int l = 0; l < BENCH_NLIB; l++)
            r->time_us[f][l] = NAN;
    }
}

double bench_ratio_vs_libm(double us_libm, double us_impl)
{
    if (us_impl < 1e-9)
        return 0.0;
    return us_libm / us_impl;
}

/* ---------- naive polynomial baseline (pedagogical) ---------- */

static float poly_sin_taylor7_reduced(float x)
{
    float x2 = x * x;
    return x * (1.0f + x2 * (-1.0f / 6.0f + x2 * (1.0f / 120.0f + x2 * (-1.0f / 5040.0f))));
}

static float poly_sin_full(float rad)
{
    const float pi = QF_PI;
    float       x  = fmodf(rad + pi, 2.0f * pi) - pi;
    if (x > pi * 0.5f)
        x = pi - x;
    else if (x < -pi * 0.5f)
        x = -pi - x;
    return poly_sin_taylor7_reduced(x);
}

typedef struct {
    double max_abs;
    double max_rel;
} err2_t;

static void err2_clear(err2_t *e)
{
    e->max_abs = 0.0;
    e->max_rel = 0.0;
}

static void err2_track_abs(err2_t *e, double abs_err)
{
    if (abs_err > e->max_abs)
        e->max_abs = abs_err;
}

static void err2_track_rel(err2_t *e, double ref, double approx)
{
    double ax = fabs(ref);
    if (ax > 1e-18) {
        double rel = fabs((approx - ref) / ref);
        if (rel > e->max_rel)
            e->max_rel = rel;
    }
}

/* ---------- libfixmath float bridges ---------- */

static float fix16_sin_float(float rad)
{
    return fix16_to_float(fix16_sin(fix16_from_float(rad)));
}

static float fix16_cos_float(float rad)
{
    return fix16_to_float(fix16_cos(fix16_from_float(rad)));
}

static float fix16_sqrt_float(float x)
{
    return fix16_to_float(fix16_sqrt(fix16_from_float(x)));
}

static float fix16_ln_float(float x)
{
    return fix16_to_float(fix16_log(fix16_from_float(x)));
}

static float fix16_exp_float(float x)
{
    return fix16_to_float(fix16_exp(fix16_from_float(x)));
}

/* ---------- fr_math float bridges ---------- */

static s32 fr_rad_to_q(float rad) { return (s32)(rad * FR_SCAL); }

static s32 fr_float_to_q(float x) { return (s32)(x * FR_SCAL); }

static float fr_q_to_float(s32 v) { return (float)v / FR_SCAL; }

static float fr_sin_f(float rad)
{
    return fr_q_to_float(fr_sin(fr_rad_to_q(rad), BENCH_FR_RX));
}

static float fr_cos_f(float rad)
{
    return fr_q_to_float(fr_cos(fr_rad_to_q(rad), BENCH_FR_RX));
}

static float fr_sqrt_f(float x)
{
    return fr_q_to_float(FR_sqrt(fr_float_to_q(x), BENCH_FR_RX));
}

static float fr_ln_f(float x)
{
    s32 r = FR_ln(fr_float_to_q(x), BENCH_FR_RX, BENCH_FR_RX);
    if (r == FR_DOMAIN_ERROR)
        return (float)NAN;
    return fr_q_to_float(r);
}

static float fr_exp_f(float x)
{
    s32 r = FR_EXP(fr_float_to_q(x), BENCH_FR_RX);
    return fr_q_to_float(r);
}

static void bench_sin(const bench_timer_t *tm, volatile float *sink, bench_results_t *out)
{
    float inputs[BENCH_N_SAMPLES];
    for (int i = 0; i < BENCH_N_SAMPLES; i++)
        inputs[i] = -6.2831853f + 12.5663706f * ((float)i / (float)(BENCH_N_SAMPLES - 1));

    err2_t eqf, efx, efr, epoly;
    err2_clear(&eqf);
    err2_clear(&efx);
    err2_clear(&efr);
    err2_clear(&epoly);

    for (int i = 0; i < BENCH_N_SAMPLES; i++) {
        double ref = sin((double)inputs[i]);
        err2_track_abs(&eqf, fabs(ref - (double)qf_sin(inputs[i])));
        err2_track_abs(&efx, fabs(ref - (double)fix16_sin_float(inputs[i])));
        err2_track_abs(&efr, fabs(ref - (double)fr_sin_f(inputs[i])));
        err2_track_abs(&epoly, fabs(ref - (double)poly_sin_full(inputs[i])));
    }

    uint64_t t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = qf_sin(inputs[i % BENCH_N_SAMPLES]);
    uint64_t t1 = tm->now(tm->ctx);
    double   us_qf = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = sinf(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_libm = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fix16_sin_float(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fx = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fr_sin_f(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fr = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = poly_sin_full(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_poly = tm->elapsed_us(tm->ctx, t0, t1);

    out->acc_pct[BENCH_F_SIN][0] = eqf.max_abs * 100.0;
    out->acc_pct[BENCH_F_SIN][1] = efx.max_abs * 100.0;
    out->acc_pct[BENCH_F_SIN][2] = efr.max_abs * 100.0;
    out->acc_pct[BENCH_F_SIN][3] = epoly.max_abs * 100.0;

    out->time_us[BENCH_F_SIN][BENCH_L_QF]   = us_qf;
    out->time_us[BENCH_F_SIN][BENCH_L_LIBM] = us_libm;
    out->time_us[BENCH_F_SIN][BENCH_L_FX]   = us_fx;
    out->time_us[BENCH_F_SIN][BENCH_L_FR]   = us_fr;
    out->time_us[BENCH_F_SIN][BENCH_L_POLY] = us_poly;
}

static void bench_cos(const bench_timer_t *tm, volatile float *sink, bench_results_t *out)
{
    float inputs[BENCH_N_SAMPLES];
    for (int i = 0; i < BENCH_N_SAMPLES; i++)
        inputs[i] = -6.2831853f + 12.5663706f * ((float)i / (float)(BENCH_N_SAMPLES - 1));

    err2_t eqf, efx, efr;
    err2_clear(&eqf);
    err2_clear(&efx);
    err2_clear(&efr);

    for (int i = 0; i < BENCH_N_SAMPLES; i++) {
        double ref = cos((double)inputs[i]);
        err2_track_abs(&eqf, fabs(ref - (double)qf_cos(inputs[i])));
        err2_track_abs(&efx, fabs(ref - (double)fix16_cos_float(inputs[i])));
        err2_track_abs(&efr, fabs(ref - (double)fr_cos_f(inputs[i])));
    }

    uint64_t t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = qf_cos(inputs[i % BENCH_N_SAMPLES]);
    uint64_t t1 = tm->now(tm->ctx);
    double   us_qf = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = cosf(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_libm = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fix16_cos_float(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fx = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fr_cos_f(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fr = tm->elapsed_us(tm->ctx, t0, t1);

    out->acc_pct[BENCH_F_COS][0] = eqf.max_abs * 100.0;
    out->acc_pct[BENCH_F_COS][1] = efx.max_abs * 100.0;
    out->acc_pct[BENCH_F_COS][2] = efr.max_abs * 100.0;

    out->time_us[BENCH_F_COS][BENCH_L_QF]   = us_qf;
    out->time_us[BENCH_F_COS][BENCH_L_LIBM] = us_libm;
    out->time_us[BENCH_F_COS][BENCH_L_FX]   = us_fx;
    out->time_us[BENCH_F_COS][BENCH_L_FR]   = us_fr;
}

static void bench_sqrt(const bench_timer_t *tm, volatile float *sink, bench_results_t *out)
{
    float inputs[BENCH_N_SAMPLES];
    for (int i = 0; i < BENCH_N_SAMPLES; i++)
        inputs[i] = 0.0005f + 8000.0f * ((float)i / (float)(BENCH_N_SAMPLES - 1));

    err2_t eqf, efx, efr;
    err2_clear(&eqf);
    err2_clear(&efx);
    err2_clear(&efr);

    for (int i = 0; i < BENCH_N_SAMPLES; i++) {
        double ref = sqrt((double)inputs[i]);
        err2_track_rel(&eqf, ref, (double)qf_sqrt(inputs[i]));
        err2_track_rel(&efx, ref, (double)fix16_sqrt_float(inputs[i]));
        err2_track_rel(&efr, ref, (double)fr_sqrt_f(inputs[i]));
    }

    uint64_t t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = qf_sqrt(inputs[i % BENCH_N_SAMPLES]);
    uint64_t t1 = tm->now(tm->ctx);
    double   us_qf = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = sqrtf(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_libm = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fix16_sqrt_float(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fx = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fr_sqrt_f(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fr = tm->elapsed_us(tm->ctx, t0, t1);

    out->acc_pct[BENCH_F_SQRT][0] = eqf.max_rel * 100.0;
    out->acc_pct[BENCH_F_SQRT][1] = efx.max_rel * 100.0;
    out->acc_pct[BENCH_F_SQRT][2] = efr.max_rel * 100.0;

    out->time_us[BENCH_F_SQRT][BENCH_L_QF]   = us_qf;
    out->time_us[BENCH_F_SQRT][BENCH_L_LIBM] = us_libm;
    out->time_us[BENCH_F_SQRT][BENCH_L_FX]   = us_fx;
    out->time_us[BENCH_F_SQRT][BENCH_L_FR]   = us_fr;
}

static void bench_ln(const bench_timer_t *tm, volatile float *sink, bench_results_t *out)
{
    float inputs[BENCH_N_SAMPLES];
    for (int i = 0; i < BENCH_N_SAMPLES; i++)
        inputs[i] = 0.05f + 120.0f * ((float)i / (float)(BENCH_N_SAMPLES - 1));

    err2_t eqf, efx, efr;
    err2_clear(&eqf);
    err2_clear(&efx);
    err2_clear(&efr);

    for (int i = 0; i < BENCH_N_SAMPLES; i++) {
        double ref = log((double)inputs[i]);
        err2_track_rel(&eqf, ref, (double)qf_ln(inputs[i]));
        err2_track_rel(&efx, ref, (double)fix16_ln_float(inputs[i]));
        float g = fr_ln_f(inputs[i]);
        if (g == g)
            err2_track_rel(&efr, ref, (double)g);
    }

    uint64_t t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = qf_ln(inputs[i % BENCH_N_SAMPLES]);
    uint64_t t1 = tm->now(tm->ctx);
    double   us_qf = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = logf(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_libm = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fix16_ln_float(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fx = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fr_ln_f(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fr = tm->elapsed_us(tm->ctx, t0, t1);

    out->acc_pct[BENCH_F_LN][0] = eqf.max_rel * 100.0;
    out->acc_pct[BENCH_F_LN][1] = efx.max_rel * 100.0;
    out->acc_pct[BENCH_F_LN][2] = efr.max_rel * 100.0;

    out->time_us[BENCH_F_LN][BENCH_L_QF]   = us_qf;
    out->time_us[BENCH_F_LN][BENCH_L_LIBM] = us_libm;
    out->time_us[BENCH_F_LN][BENCH_L_FX]   = us_fx;
    out->time_us[BENCH_F_LN][BENCH_L_FR]   = us_fr;
}

static void bench_exp(const bench_timer_t *tm, volatile float *sink, bench_results_t *out)
{
    float inputs[BENCH_N_SAMPLES];
    for (int i = 0; i < BENCH_N_SAMPLES; i++)
        inputs[i] = -6.0f + 12.0f * ((float)i / (float)(BENCH_N_SAMPLES - 1));

    err2_t eqf, efx, efr;
    err2_clear(&eqf);
    err2_clear(&efx);
    err2_clear(&efr);

    for (int i = 0; i < BENCH_N_SAMPLES; i++) {
        double ref = exp((double)inputs[i]);
        if (ref > 1e12 || ref < 1e-12)
            continue;
        err2_track_rel(&eqf, ref, (double)qf_exp(inputs[i]));
        err2_track_rel(&efx, ref, (double)fix16_exp_float(inputs[i]));
        err2_track_rel(&efr, ref, (double)fr_exp_f(inputs[i]));
    }

    uint64_t t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = qf_exp(inputs[i % BENCH_N_SAMPLES]);
    uint64_t t1 = tm->now(tm->ctx);
    double   us_qf = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = expf(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_libm = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fix16_exp_float(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fx = tm->elapsed_us(tm->ctx, t0, t1);

    t0 = tm->now(tm->ctx);
    for (int r = 0; r < BENCH_N_ITER; r++)
        for (int i = 0; i < 64; i++)
            *sink = fr_exp_f(inputs[i % BENCH_N_SAMPLES]);
    t1       = tm->now(tm->ctx);
    double us_fr = tm->elapsed_us(tm->ctx, t0, t1);

    out->acc_pct[BENCH_F_EXP][0] = eqf.max_rel * 100.0;
    out->acc_pct[BENCH_F_EXP][1] = efx.max_rel * 100.0;
    out->acc_pct[BENCH_F_EXP][2] = efr.max_rel * 100.0;

    out->time_us[BENCH_F_EXP][BENCH_L_QF]   = us_qf;
    out->time_us[BENCH_F_EXP][BENCH_L_LIBM] = us_libm;
    out->time_us[BENCH_F_EXP][BENCH_L_FX]   = us_fx;
    out->time_us[BENCH_F_EXP][BENCH_L_FR]   = us_fr;
}

void bench_run_all(const bench_timer_t *timer, volatile float *sink, bench_results_t *out)
{
    bench_results_init(out);
    timer->init(timer->ctx);
    bench_sin(timer, sink, out);
    bench_cos(timer, sink, out);
    bench_sqrt(timer, sink, out);
    bench_ln(timer, sink, out);
    bench_exp(timer, sink, out);
}

void bench_print_human(const bench_results_t *r)
{
    printf("qf_math comparison benchmark suite\n");
    printf("===================================\n");
    printf("Samples: %d  outer iterations: %d (x64 inner calls)\n", BENCH_N_SAMPLES, BENCH_N_ITER);
    printf("Platform pointers: %lu-bit\n\n", (unsigned long)(sizeof(void *) * 8UL));

    printf("Implementations:\n");
    printf("  qf_math     — float tables + interpolation (this repo)\n");
    printf("  libm        — sinf/cosf/sqrtf/logf/expf reference\n");
    printf("  libfixmath  — Q16.16 fix16_t (float bridge in this bench)\n");
    printf("  fr_math     — fixed-radix integer math (float bridge, radix=%d)\n", BENCH_FR_RX);
    printf("  poly7+fmod — naive Taylor sin baseline\n\n");

    printf("--- Accuracy vs double reference & wall-clock ---\n");

    double us_qf, us_libm, us_fx, us_fr, us_poly;

    us_qf = r->time_us[BENCH_F_SIN][BENCH_L_QF];
    us_libm = r->time_us[BENCH_F_SIN][BENCH_L_LIBM];
    us_fx = r->time_us[BENCH_F_SIN][BENCH_L_FX];
    us_fr = r->time_us[BENCH_F_SIN][BENCH_L_FR];
    us_poly = r->time_us[BENCH_F_SIN][BENCH_L_POLY];
    printf("  sin       max abs err  qf_math %.3e  libfixmath %.3e  fr_math %.3e  poly %.3e\n",
           r->acc_pct[BENCH_F_SIN][0] / 100.0, r->acc_pct[BENCH_F_SIN][1] / 100.0,
           r->acc_pct[BENCH_F_SIN][2] / 100.0, r->acc_pct[BENCH_F_SIN][3] / 100.0);
    printf("            (pct of ±1 FS)  %.5g  %.5g  %.5g  %.5g\n", r->acc_pct[BENCH_F_SIN][0],
           r->acc_pct[BENCH_F_SIN][1], r->acc_pct[BENCH_F_SIN][2], r->acc_pct[BENCH_F_SIN][3]);
    printf("            time ms      qf_math %8.2f  libm %8.2f  libfixmath %8.2f  fr_math %8.2f  poly %8.2f\n",
           us_qf / 1000.0, us_libm / 1000.0, us_fx / 1000.0, us_fr / 1000.0, us_poly / 1000.0);
    printf("            vs libm      qf_math %.2fx  libfixmath %.2fx  fr_math %.2fx  poly %.2fx\n\n",
           bench_ratio_vs_libm(us_libm, us_qf), bench_ratio_vs_libm(us_libm, us_fx),
           bench_ratio_vs_libm(us_libm, us_fr), bench_ratio_vs_libm(us_libm, us_poly));

    us_qf = r->time_us[BENCH_F_COS][BENCH_L_QF];
    us_libm = r->time_us[BENCH_F_COS][BENCH_L_LIBM];
    us_fx = r->time_us[BENCH_F_COS][BENCH_L_FX];
    us_fr = r->time_us[BENCH_F_COS][BENCH_L_FR];
    printf("  cos       max abs err  qf_math %.3e  libfixmath %.3e  fr_math %.3e\n",
           r->acc_pct[BENCH_F_COS][0] / 100.0, r->acc_pct[BENCH_F_COS][1] / 100.0,
           r->acc_pct[BENCH_F_COS][2] / 100.0);
    printf("            time ms      qf_math %8.2f  libm %8.2f  libfixmath %8.2f  fr_math %8.2f\n",
           us_qf / 1000.0, us_libm / 1000.0, us_fx / 1000.0, us_fr / 1000.0);
    printf("            vs libm      qf_math %.2fx  libfixmath %.2fx  fr_math %.2fx\n\n",
           bench_ratio_vs_libm(us_libm, us_qf), bench_ratio_vs_libm(us_libm, us_fx),
           bench_ratio_vs_libm(us_libm, us_fr));

    us_qf = r->time_us[BENCH_F_SQRT][BENCH_L_QF];
    us_libm = r->time_us[BENCH_F_SQRT][BENCH_L_LIBM];
    us_fx = r->time_us[BENCH_F_SQRT][BENCH_L_FX];
    us_fr = r->time_us[BENCH_F_SQRT][BENCH_L_FR];
    printf("  sqrt      max rel err    qf_math %.3e  libfixmath %.3e  fr_math %.3e\n",
           r->acc_pct[BENCH_F_SQRT][0] / 100.0, r->acc_pct[BENCH_F_SQRT][1] / 100.0,
           r->acc_pct[BENCH_F_SQRT][2] / 100.0);
    printf("            time ms        qf_math %8.2f  libm %8.2f  libfixmath %8.2f  fr_math %8.2f\n",
           us_qf / 1000.0, us_libm / 1000.0, us_fx / 1000.0, us_fr / 1000.0);
    printf("            vs libm        qf_math %.2fx  libfixmath %.2fx  fr_math %.2fx\n\n",
           bench_ratio_vs_libm(us_libm, us_qf), bench_ratio_vs_libm(us_libm, us_fx),
           bench_ratio_vs_libm(us_libm, us_fr));

    us_qf = r->time_us[BENCH_F_LN][BENCH_L_QF];
    us_libm = r->time_us[BENCH_F_LN][BENCH_L_LIBM];
    us_fx = r->time_us[BENCH_F_LN][BENCH_L_FX];
    us_fr = r->time_us[BENCH_F_LN][BENCH_L_FR];
    printf("  ln        max rel err    qf_math %.3e  libfixmath %.3e  fr_math %.3e\n",
           r->acc_pct[BENCH_F_LN][0] / 100.0, r->acc_pct[BENCH_F_LN][1] / 100.0,
           r->acc_pct[BENCH_F_LN][2] / 100.0);
    printf("            time ms        qf_math %8.2f  libm %8.2f  libfixmath %8.2f  fr_math %8.2f\n",
           us_qf / 1000.0, us_libm / 1000.0, us_fx / 1000.0, us_fr / 1000.0);
    printf("            vs libm        qf_math %.2fx  libfixmath %.2fx  fr_math %.2fx\n\n",
           bench_ratio_vs_libm(us_libm, us_qf), bench_ratio_vs_libm(us_libm, us_fx),
           bench_ratio_vs_libm(us_libm, us_fr));

    us_qf = r->time_us[BENCH_F_EXP][BENCH_L_QF];
    us_libm = r->time_us[BENCH_F_EXP][BENCH_L_LIBM];
    us_fx = r->time_us[BENCH_F_EXP][BENCH_L_FX];
    us_fr = r->time_us[BENCH_F_EXP][BENCH_L_FR];
    printf("  exp       max rel err    qf_math %.3e  libfixmath %.3e  fr_math %.3e\n",
           r->acc_pct[BENCH_F_EXP][0] / 100.0, r->acc_pct[BENCH_F_EXP][1] / 100.0,
           r->acc_pct[BENCH_F_EXP][2] / 100.0);
    printf("            time ms        qf_math %8.2f  libm %8.2f  libfixmath %8.2f  fr_math %8.2f\n",
           us_qf / 1000.0, us_libm / 1000.0, us_fx / 1000.0, us_fr / 1000.0);
    printf("            vs libm        qf_math %.2fx  libfixmath %.2fx  fr_math %.2fx\n\n",
           bench_ratio_vs_libm(us_libm, us_qf), bench_ratio_vs_libm(us_libm, us_fx),
           bench_ratio_vs_libm(us_libm, us_fr));
}

static void bench_md_fixed6_cell(FILE *out, double v)
{
    if (isnan(v)) {
        fprintf(out, " — |");
        return;
    }

    fprintf(out, " %.6f |", v);
}

static void bench_md_pct_cell(FILE *out, double v)
{
    bench_md_fixed6_cell(out, v);
}

static void bench_md_us_cell(FILE *out, double us)
{
    bench_md_fixed6_cell(out, us);
}

static void bench_md_ratio_cell(FILE *out, double libm_us, double impl_us)
{
    if (isnan(impl_us) || isnan(libm_us))
        fprintf(out, " — |");
    else
        bench_md_fixed6_cell(out, bench_ratio_vs_libm(libm_us, impl_us));
}

void bench_emit_markdown_tables(FILE *out, const bench_results_t *r, bench_md_style_t style)
{
    fprintf(out, "### Accuracy — max percent error\n\n");
    fprintf(out,
            "**sin / cos:** maximum absolute error versus `sin`/`cos` in double, expressed as "
            "**percent of unit amplitude** (output in [-1, 1], so multiply absolute error by 100).\n\n");
    fprintf(out,
            "**sqrt / ln / exp:** maximum **relative** error versus the double reference on the "
            "same sample grids as the C code, expressed as **percent** "
            "(max |(approx − ref) / ref| × 100).\n\n");
    fprintf(out, "| Function | qf_math | libfixmath | fr_math |\n");
    fprintf(out, "| :--- | ---:| ---:| ---:|\n");

    static const char *fnames[BENCH_NFUNC] = {
        "`sin`",
        "`cos`",
        "`sqrt`",
        "`ln`",
        "`exp`",
    };

    for (int f = 0; f < BENCH_NFUNC; f++) {
        fprintf(out, "| %s |", fnames[f]);
        for (int c = 0; c < 3; c++)
            bench_md_pct_cell(out, r->acc_pct[f][c]);
        fprintf(out, "\n");
    }

    fprintf(out,
            "\nNumbers are printed with six digits after the decimal point. Current timed/error "
            "coverage is `sin`, `cos`, "
            "`sqrt`, `ln`, and `exp`; broader API accuracy (`tan`, inverse trig, `log2`, `log10`, "
            "`pow2`, `pow10`, `hypot`, etc.) belongs in a separate coverage table before it is "
            "mixed into these platform timing rows.\n\n");

    fprintf(out, "### Wall-clock time (microseconds)\n\n");
    if (style == BENCH_MD_MCU)
        fprintf(out,
                "Total microseconds for each benchmark loop (**MCU wall-clock**; timer is "
                "`esp_timer_get_time()` or host monotonic equivalent).\n\n");
    else
        fprintf(out,
                "Total microseconds for each benchmark loop on this host (see metadata).\n\n");

    fprintf(out, "| Library | sin | cos | sqrt | ln | exp |\n");
    fprintf(out, "| :--- | ---:| ---:| ---:| ---:| ---:|\n");

    static const char *libnames[BENCH_NLIB] = {
        "**qf_math**",
        "**libm** (`sinf` …)",
        "**libfixmath** (float bridge)",
        "**fr_math** (float bridge)",
        "**Taylor poly** (internal sin baseline)",
    };

    for (int l = 0; l < BENCH_L_POLY; l++) {
        fprintf(out, "| %s |", libnames[l]);
        for (int f = 0; f < BENCH_NFUNC; f++)
            bench_md_us_cell(out, r->time_us[f][l]);
        fprintf(out, "\n");
    }

    fprintf(out, "\n### Speed vs libm (ratio)\n\n");
    if (style == BENCH_MD_MCU)
        fprintf(out,
                "`libm` time ÷ implementation time for the same loop — **above 1.0** means faster than "
                "**`sinf`/`cosf`/… on this MCU** in this microbenchmark.\n\n");
    else
        fprintf(out,
                "`libm` time ÷ implementation time for the same loop — **above 1.0** means faster than "
                "host `sinf`/`cosf`/… **in this microbenchmark** (not representative of every MCU).\n\n");
    fprintf(out,
            "**Fixed-point peer rows are bridged harness timings**: `float`→fixed→function→`float`. "
            "They keep the report comparable, but are not native `fix16_t` / `s32` pipeline timings.\n\n");

    fprintf(out, "| Library | sin | cos | sqrt | ln | exp |\n");
    fprintf(out, "| :--- | ---:| ---:| ---:| ---:| ---:|\n");

    for (int l = 0; l < BENCH_NLIB; l++) {
        if (l == BENCH_L_LIBM) {
            fprintf(out, "| libm (reference) | 1.000000 | 1.000000 | 1.000000 | 1.000000 | 1.000000 |\n");
            continue;
        }
        if (l == BENCH_L_POLY)
            continue;
        fprintf(out, "| %s |", libnames[l]);
        for (int f = 0; f < BENCH_NFUNC; f++) {
            double libm_us = r->time_us[f][BENCH_L_LIBM];
            double impl_us = r->time_us[f][l];
            bench_md_ratio_cell(out, libm_us, impl_us);
        }
        fprintf(out, "\n");
    }

    fprintf(out, "\n");
}

void bench_emit_markdown_doc_snapshot(FILE *out, const bench_results_t *r, const char *device_line)
{
    fprintf(out,
            "<!-- MCU benchmark snapshot — paste into docs (`compare/MCU_BENCHMARK_SNAPSHOT.md`). "
            "Same grids & iteration counts as host `make compare`. -->\n\n");
    fprintf(out, "### MCU benchmark snapshot\n\n");
    fprintf(out, "| Field | Value |\n");
    fprintf(out, "| --- | --- |\n");
    fprintf(out, "| Device | %s |\n", (device_line && device_line[0]) ? device_line : "—");
    fprintf(out, "| qf_math | %s (`QF_MATH_VERSION_HEX`=%#x) |\n", QF_MATH_VERSION,
            QF_MATH_VERSION_HEX);
    fprintf(out, "| Loop shape | %d sample grid × %d outer × 64 inner calls |\n\n", BENCH_N_SAMPLES,
            BENCH_N_ITER);
    fprintf(out, "---\n\n");
    bench_emit_markdown_tables(out, r, BENCH_MD_MCU);
    fflush(out);
}
