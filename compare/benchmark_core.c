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
#define BENCH_DEG2RAD (0.017453292519943295769f)
#define BENCH_BAM2RAD (0.00009587379924285257f)
#define BENCH_TAN_FIXED_MAX ((double)0x7fffffff / 65536.0)
#define BENCH_TAN_FIX16_SAT_FLOOR 64.0

void bench_results_init(bench_results_t *r)
{
    for (int f = 0; f < BENCH_NFUNC; f++) {
        for (int l = 0; l < BENCH_NLIB; l++) {
            r->acc[f][l] = NAN;
            r->mse[f][l] = NAN;
            r->time_us[f][l] = NAN;
        }
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
    double sum_sq;
    int count;
} err2_t;

static void err2_clear(err2_t *e)
{
    e->max_abs = 0.0;
    e->max_rel = 0.0;
    e->sum_sq = 0.0;
    e->count = 0;
}

static void err2_track_abs(err2_t *e, double abs_err)
{
    if (abs_err > e->max_abs)
        e->max_abs = abs_err;
    e->sum_sq += abs_err * abs_err;
    e->count++;
}

static void err2_track_rel(err2_t *e, double ref, double approx)
{
    double ax = fabs(ref);
    if (ax > 1e-18) {
        double rel = fabs((approx - ref) / ref);
        if (rel > e->max_rel)
            e->max_rel = rel;
        e->sum_sq += rel * rel;
        e->count++;
    }
}

static double err2_mse(const err2_t *e)
{
    return (e->count > 0) ? (e->sum_sq / (double)e->count) : NAN;
}

/* ---------- libfixmath float bridges ---------- */

static float fix16_sin_float(float rad)
{
    return fix16_to_float(fix16_sin(fix16_from_float(rad)));
}

static float fix16_sin_deg_float(float deg) { return fix16_sin_float(deg * BENCH_DEG2RAD); }
static float fix16_sin_bam_float(float bam) { return fix16_sin_float((float)((uint16_t)((int32_t)bam)) * BENCH_BAM2RAD); }

static float fix16_cos_float(float rad)
{
    return fix16_to_float(fix16_cos(fix16_from_float(rad)));
}

static float fix16_cos_deg_float(float deg) { return fix16_cos_float(deg * BENCH_DEG2RAD); }
static float fix16_cos_bam_float(float bam) { return fix16_cos_float((float)((uint16_t)((int32_t)bam)) * BENCH_BAM2RAD); }

static float fix16_tan_float(float rad)
{
    return fix16_to_float(fix16_tan(fix16_from_float(rad)));
}

static float fix16_tan_deg_float(float deg) { return fix16_tan_float(deg * BENCH_DEG2RAD); }
static float fix16_tan_bam_float(float bam) { return fix16_tan_float((float)((uint16_t)((int32_t)bam)) * BENCH_BAM2RAD); }

static float fix16_asin_float(float x)
{
    return fix16_to_float(fix16_asin(fix16_from_float(x)));
}

static float fix16_acos_float(float x)
{
    return fix16_to_float(fix16_acos(fix16_from_float(x)));
}

static float fix16_atan_float(float x)
{
    return fix16_to_float(fix16_atan(fix16_from_float(x)));
}

static float fix16_atan2_float(float y, float x)
{
    return fix16_to_float(fix16_atan2(fix16_from_float(y), fix16_from_float(x)));
}

static float fix16_sqrt_float(float x)
{
    return fix16_to_float(fix16_sqrt(fix16_from_float(x)));
}

static float fix16_ln_float(float x)
{
    return fix16_to_float(fix16_log(fix16_from_float(x)));
}

static float fix16_log2_float(float x)
{
    return fix16_to_float(fix16_log2(fix16_from_float(x)));
}

static float fix16_log10_float(float x)
{
    return fix16_to_float(fix16_mul(fix16_log2(fix16_from_float(x)), (fix16_t)19728));
}

static float fix16_pow2_float(float x)
{
    return fix16_to_float(fix16_exp(fix16_mul(fix16_from_float(x), (fix16_t)45426)));
}

static float fix16_exp_float(float x)
{
    return fix16_to_float(fix16_exp(fix16_from_float(x)));
}

static float fix16_pow10_float(float x)
{
    return fix16_to_float(fix16_exp(fix16_mul(fix16_from_float(x), (fix16_t)150902)));
}

static float fix16_pow_float(float x, float y)
{
    fix16_t fx = fix16_from_float(x);
    fix16_t fy = fix16_from_float(y);
    return fix16_to_float(fix16_exp(fix16_mul(fy, fix16_log(fx))));
}

/* ---------- fr_math float bridges ---------- */

static s32 fr_rad_to_q(float rad) { return (s32)(rad * FR_SCAL); }

static s32 fr_float_to_q(float x) { return (s32)(x * FR_SCAL); }

static float fr_q_to_float(s32 v) { return (float)v / FR_SCAL; }

static float fr_sin_f(float rad)
{
    return fr_q_to_float(fr_sin(fr_rad_to_q(rad), BENCH_FR_RX));
}

static float fr_sin_deg_f(float deg) { return fr_q_to_float(fr_sin_deg(fr_float_to_q(deg), BENCH_FR_RX)); }
static float fr_sin_bam_f(float bam) { return fr_q_to_float(fr_sin_bam((u16)((uint16_t)((int32_t)bam)))); }

static float fr_cos_f(float rad)
{
    return fr_q_to_float(fr_cos(fr_rad_to_q(rad), BENCH_FR_RX));
}

static float fr_cos_deg_f(float deg) { return fr_q_to_float(fr_cos_deg(fr_float_to_q(deg), BENCH_FR_RX)); }
static float fr_cos_bam_f(float bam) { return fr_q_to_float(fr_cos_bam((u16)((uint16_t)((int32_t)bam)))); }

static float fr_tan_f(float rad)
{
    return fr_q_to_float(fr_tan(fr_rad_to_q(rad), BENCH_FR_RX));
}

static float fr_tan_deg_f(float deg) { return fr_q_to_float(fr_tan_deg(fr_float_to_q(deg), BENCH_FR_RX)); }
static float fr_tan_bam_f(float bam) { return fr_q_to_float(fr_tan_bam((u16)((uint16_t)((int32_t)bam)))); }

static float fr_asin_f(float x)
{
    s32 r = FR_asin(fr_float_to_q(x), BENCH_FR_RX, BENCH_FR_RX);
    if (r == FR_DOMAIN_ERROR)
        return (float)NAN;
    return fr_q_to_float(r);
}

static float fr_acos_f(float x)
{
    s32 r = FR_acos(fr_float_to_q(x), BENCH_FR_RX, BENCH_FR_RX);
    if (r == FR_DOMAIN_ERROR)
        return (float)NAN;
    return fr_q_to_float(r);
}

static float fr_atan_f(float x)
{
    s32 r = FR_atan(fr_float_to_q(x), BENCH_FR_RX, BENCH_FR_RX);
    if (r == FR_DOMAIN_ERROR)
        return (float)NAN;
    return fr_q_to_float(r);
}

static float fr_atan2_f(float y, float x)
{
    s32 r = FR_atan2(fr_float_to_q(y), fr_float_to_q(x), BENCH_FR_RX);
    if (r == FR_DOMAIN_ERROR)
        return (float)NAN;
    return fr_q_to_float(r);
}

static float fr_sqrt_f(float x)
{
    return fr_q_to_float(FR_sqrt(fr_float_to_q(x), BENCH_FR_RX));
}

static float fr_hypot_f(float x, float y)
{
    return fr_q_to_float(FR_hypot(fr_float_to_q(x), fr_float_to_q(y), BENCH_FR_RX));
}

static float fr_hypot_fast_f(float x, float y)
{
    return fr_q_to_float(FR_hypot_fast8(fr_float_to_q(x), fr_float_to_q(y)));
}

static float fr_ln_f(float x)
{
    s32 r = FR_ln(fr_float_to_q(x), BENCH_FR_RX, BENCH_FR_RX);
    if (r == FR_DOMAIN_ERROR)
        return (float)NAN;
    return fr_q_to_float(r);
}

static float fr_log2_f(float x)
{
    s32 r = FR_log2(fr_float_to_q(x), BENCH_FR_RX, BENCH_FR_RX);
    if (r == FR_DOMAIN_ERROR)
        return (float)NAN;
    return fr_q_to_float(r);
}

static float fr_log10_f(float x)
{
    s32 r = FR_log10(fr_float_to_q(x), BENCH_FR_RX, BENCH_FR_RX);
    if (r == FR_DOMAIN_ERROR)
        return (float)NAN;
    return fr_q_to_float(r);
}

static float fr_pow2_f(float x)
{
    s32 r = FR_pow2(fr_float_to_q(x), BENCH_FR_RX);
    return fr_q_to_float(r);
}

static float fr_exp_f(float x)
{
    s32 r = FR_EXP(fr_float_to_q(x), BENCH_FR_RX);
    return fr_q_to_float(r);
}

static float fr_pow10_f(float x)
{
    s32 r = FR_POW10(fr_float_to_q(x), BENCH_FR_RX);
    return fr_q_to_float(r);
}

static float fr_pow_f(float x, float y)
{
    s32 lx = FR_log2(fr_float_to_q(x), BENCH_FR_RX, BENCH_FR_RX);
    if (lx == FR_DOMAIN_ERROR)
        return (float)NAN;
    s32 yy = fr_float_to_q(y);
    s32 e = (s32)(((s64)yy * (s64)lx) >> BENCH_FR_RX);
    return fr_q_to_float(FR_pow2(e, BENCH_FR_RX));
}

static double bench_tan_reference(double rad)
{
    double c = cos(rad);
    if (fabs(c) < 1e-6)
        return (double)QF_TAN_MAX;

    double t = tan(rad);
    if (t > (double)QF_TAN_MAX)
        return (double)QF_TAN_MAX;
    if (t < -(double)QF_TAN_MAX)
        return -(double)QF_TAN_MAX;
    return t;
}

static double bench_tan_limit_for_lib(int lib_idx, int func_idx)
{
    switch (lib_idx) {
    case BENCH_L_FX:
    case BENCH_L_FR:
        return BENCH_TAN_FIXED_MAX;
    default:
        return (double)QF_TAN_MAX;
    }
}

static double bench_tan_rad_from_input(int func_idx, float x)
{
    switch (func_idx) {
    case BENCH_F_TAN: return (double)x;
    case BENCH_F_TAN_DEG: return (double)x * (double)BENCH_DEG2RAD;
    case BENCH_F_TAN_BAM: return (double)((uint16_t)((int32_t)x)) * (double)BENCH_BAM2RAD;
    default: return NAN;
    }
}

static double bench_tan_rad_for_lib(int lib_idx, int func_idx, float x)
{
    if (lib_idx == BENCH_L_FX) {
        float rad;
        switch (func_idx) {
        case BENCH_F_TAN:
            rad = x;
            break;
        case BENCH_F_TAN_DEG:
            rad = x * BENCH_DEG2RAD;
            break;
        case BENCH_F_TAN_BAM:
            rad = (float)((uint16_t)((int32_t)x)) * BENCH_BAM2RAD;
            break;
        default:
            return NAN;
        }
        return (double)fix16_to_float(fix16_from_float(rad));
    }

    if (lib_idx == BENCH_L_FR) {
        switch (func_idx) {
        case BENCH_F_TAN:
            return (double)fr_q_to_float(fr_rad_to_q(x));
        case BENCH_F_TAN_DEG:
            return (double)fr_deg_to_bam(fr_float_to_q(x), BENCH_FR_RX) * (double)BENCH_BAM2RAD;
        case BENCH_F_TAN_BAM:
            return (double)((uint16_t)((int32_t)x)) * (double)BENCH_BAM2RAD;
        default:
            return NAN;
        }
    }

    return bench_tan_rad_from_input(func_idx, x);
}

static void bench_track_tan_metric(err2_t *e, int lib_idx, int func_idx, float x, double got)
{
    if (got != got)
        return;

    double raw_rad = bench_tan_rad_from_input(func_idx, x);
    double rad = bench_tan_rad_for_lib(lib_idx, func_idx, x);
    double limit = bench_tan_limit_for_lib(lib_idx, func_idx);
    double c = cos(raw_rad);
    double err;

    if (fabs(c) < 1e-6) {
        if (lib_idx == BENCH_L_FX)
            err = (fabs(got) >= 1.0) ? 0.0 : limit;
        else
            err = fabs(fabs(got) - limit);
    } else {
        double ref = tan(rad);
        if (lib_idx == BENCH_L_FX &&
            fabs(ref) > BENCH_TAN_FIX16_SAT_FLOOR &&
            fabs(got) >= 1.0) {
            err = ((ref < 0.0) == (got < 0.0)) ? 0.0 : fabs(got);
        } else if ((lib_idx == BENCH_L_FX || lib_idx == BENCH_L_FR) && fabs(ref) > limit) {
            err = fabs(fabs(got) - limit);
        } else {
            if (ref > limit)
                ref = limit;
            else if (ref < -limit)
                ref = -limit;
            err = fabs(ref - got);
        }
    }

    err2_track_abs(e, err);
}

const char *bench_func_name(int func_idx)
{
    static const char *names[BENCH_NFUNC] = {
        "sin_rad", "sin_deg", "sin_bam",
        "cos_rad", "cos_deg", "cos_bam",
        "tan_rad", "tan_deg", "tan_bam",
        "asin", "acos", "atan", "atan2",
        "sqrt", "hypot", "hypot_fast2", "hypot_fast",
        "log2", "ln", "log10", "pow2", "exp", "pow10", "pow",
    };
    return (func_idx >= 0 && func_idx < BENCH_NFUNC) ? names[func_idx] : "?";
}

const char *bench_func_metric(int func_idx)
{
    switch (func_idx) {
    case BENCH_F_SIN:
    case BENCH_F_SIN_DEG:
    case BENCH_F_SIN_BAM:
    case BENCH_F_COS:
    case BENCH_F_COS_DEG:
    case BENCH_F_COS_BAM:
        return "abs %FS";
    case BENCH_F_TAN:
    case BENCH_F_TAN_DEG:
    case BENCH_F_TAN_BAM:
        return "abs";
    case BENCH_F_ASIN:
    case BENCH_F_ACOS:
    case BENCH_F_ATAN:
    case BENCH_F_ATAN2:
        return "abs rad";
    case BENCH_F_SQRT:
    case BENCH_F_HYPOT:
    case BENCH_F_HYPOT_FAST2:
    case BENCH_F_HYPOT_FAST:
    case BENCH_F_LOG2:
    case BENCH_F_LN:
    case BENCH_F_LOG10:
    case BENCH_F_POW2:
    case BENCH_F_EXP:
    case BENCH_F_POW10:
    case BENCH_F_POW:
        return "rel %";
    default:
        return "?";
    }
}

const char *bench_lib_name(int lib_idx)
{
    static const char *names[BENCH_NLIB] = {
        "**qf_math**",
        "libm",
        "**libfixmath** (float bridge)",
        "**fr_math** (float bridge)",
        "**FastTrig**",
        "**ESP-DSP**",
        "**espp/math**",
    };
    return (lib_idx >= 0 && lib_idx < BENCH_NLIB) ? names[lib_idx] : "?";
}

static float s_in_a[BENCH_N_SAMPLES];
static float s_in_b[BENCH_N_SAMPLES];

static void bench_fill_inputs(int func_idx)
{
    for (int i = 0; i < BENCH_N_SAMPLES; i++) {
        float t = (float)i / (float)(BENCH_N_SAMPLES - 1);
        switch (func_idx) {
        case BENCH_F_SIN:
        case BENCH_F_COS:
            s_in_a[i] = -6.2831853f + 12.5663706f * ((float)i / (float)BENCH_N_SAMPLES);
            break;
        case BENCH_F_SIN_DEG:
        case BENCH_F_COS_DEG:
            s_in_a[i] = -360.0f + 720.0f * ((float)i / (float)BENCH_N_SAMPLES);
            break;
        case BENCH_F_SIN_BAM:
        case BENCH_F_COS_BAM:
            s_in_a[i] = 65536.0f * ((float)i / (float)BENCH_N_SAMPLES);
            break;
        case BENCH_F_TAN:
            s_in_a[i] = -6.2831853f + 12.5663706f * ((float)i / (float)BENCH_N_SAMPLES);
            break;
        case BENCH_F_TAN_DEG:
            s_in_a[i] = -360.0f + 720.0f * ((float)i / (float)BENCH_N_SAMPLES);
            break;
        case BENCH_F_TAN_BAM:
            s_in_a[i] = 65536.0f * ((float)i / (float)BENCH_N_SAMPLES);
            break;
        case BENCH_F_ASIN:
        case BENCH_F_ACOS:
            s_in_a[i] = -0.9995f + 1.9990f * t;
            break;
        case BENCH_F_ATAN:
            s_in_a[i] = -8.0f + 16.0f * t;
            break;
        case BENCH_F_ATAN2:
        {
            float u = (float)((i * 37) % BENCH_N_SAMPLES) / (float)(BENCH_N_SAMPLES - 1);
            s_in_a[i] = -4.0f + 8.0f * t;
            s_in_b[i] = -4.0f + 8.0f * u;
            if (fabsf(s_in_a[i]) < 0.0001f && fabsf(s_in_b[i]) < 0.0001f)
                s_in_b[i] = 0.0001f;
            break;
        }
        case BENCH_F_SQRT:
            s_in_a[i] = 0.0005f + 8000.0f * t;
            break;
        case BENCH_F_HYPOT:
        case BENCH_F_HYPOT_FAST2:
        case BENCH_F_HYPOT_FAST:
            s_in_a[i] = -4000.0f + 8000.0f * t;
            s_in_b[i] = 4000.0f - 6000.0f * t;
            break;
        case BENCH_F_LOG2:
        case BENCH_F_LN:
        case BENCH_F_LOG10:
            s_in_a[i] = 0.05f + 120.0f * t;
            break;
        case BENCH_F_POW2:
        case BENCH_F_EXP:
        case BENCH_F_POW10:
            s_in_a[i] = -6.0f + 12.0f * t;
            break;
        case BENCH_F_POW:
            s_in_a[i] = 0.05f + 120.0f * t;
            s_in_b[i] = -2.0f + 4.0f * t;
            break;
        default:
            s_in_a[i] = t;
            s_in_b[i] = t;
            break;
        }
    }
}

static double bench_ref_unary(int func_idx, float x)
{
    switch (func_idx) {
    case BENCH_F_SIN: return sin((double)x);
    case BENCH_F_SIN_DEG: return sin((double)x * (double)BENCH_DEG2RAD);
    case BENCH_F_SIN_BAM: return sin((double)((uint16_t)((int32_t)x)) * (double)BENCH_BAM2RAD);
    case BENCH_F_COS: return cos((double)x);
    case BENCH_F_COS_DEG: return cos((double)x * (double)BENCH_DEG2RAD);
    case BENCH_F_COS_BAM: return cos((double)((uint16_t)((int32_t)x)) * (double)BENCH_BAM2RAD);
    case BENCH_F_TAN: return bench_tan_reference((double)x);
    case BENCH_F_TAN_DEG: return bench_tan_reference((double)x * (double)BENCH_DEG2RAD);
    case BENCH_F_TAN_BAM: return bench_tan_reference((double)((uint16_t)((int32_t)x)) * (double)BENCH_BAM2RAD);
    case BENCH_F_ASIN: return asin((double)x);
    case BENCH_F_ACOS: return acos((double)x);
    case BENCH_F_ATAN: return atan((double)x);
    case BENCH_F_SQRT: return sqrt((double)x);
    case BENCH_F_LOG2: return log2((double)x);
    case BENCH_F_LN: return log((double)x);
    case BENCH_F_LOG10: return log10((double)x);
    case BENCH_F_POW2: return exp2((double)x);
    case BENCH_F_EXP: return exp((double)x);
    case BENCH_F_POW10: return pow(10.0, (double)x);
    default: return NAN;
    }
}

static double bench_ref_binary(int func_idx, float a, float b)
{
    switch (func_idx) {
    case BENCH_F_ATAN2: return atan2((double)a, (double)b);
    case BENCH_F_HYPOT: return hypot((double)a, (double)b);
    case BENCH_F_HYPOT_FAST2: return hypot((double)a, (double)b);
    case BENCH_F_HYPOT_FAST: return hypot((double)a, (double)b);
    case BENCH_F_POW: return pow((double)a, (double)b);
    default: return NAN;
    }
}

static void bench_track_metric(err2_t *e, int func_idx, double ref, double got)
{
    if (got != got || ref != ref)
        return;
    switch (func_idx) {
    case BENCH_F_SIN:
    case BENCH_F_SIN_DEG:
    case BENCH_F_SIN_BAM:
    case BENCH_F_COS:
    case BENCH_F_COS_DEG:
    case BENCH_F_COS_BAM:
        err2_track_abs(e, fabs(ref - got) * 100.0);
        break;
    case BENCH_F_SQRT:
    case BENCH_F_HYPOT:
    case BENCH_F_HYPOT_FAST2:
    case BENCH_F_HYPOT_FAST:
    case BENCH_F_LOG2:
    case BENCH_F_LN:
    case BENCH_F_LOG10:
    case BENCH_F_POW2:
    case BENCH_F_EXP:
    case BENCH_F_POW10:
    case BENCH_F_POW:
        err2_track_rel(e, ref, got);
        break;
    default:
        err2_track_abs(e, fabs(ref - got));
        break;
    }
}

void bench_run_unary_peer(const bench_timer_t *tm,
                          volatile float *sink,
                          bench_results_t *out,
                          int lib_idx,
                          int func_idx,
                          bench_unary_fn_t fn)
{
    if (!fn || lib_idx < 0 || lib_idx >= BENCH_NLIB || func_idx < 0 || func_idx >= BENCH_NFUNC)
        return;
    bench_fill_inputs(func_idx);
    err2_t e;
    err2_clear(&e);
    for (int i = 0; i < BENCH_N_SAMPLES; i++) {
        double got = (double)fn(s_in_a[i]);
        if (func_idx == BENCH_F_TAN || func_idx == BENCH_F_TAN_DEG || func_idx == BENCH_F_TAN_BAM)
            bench_track_tan_metric(&e, lib_idx, func_idx, s_in_a[i], got);
        else
            bench_track_metric(&e, func_idx, bench_ref_unary(func_idx, s_in_a[i]), got);
    }

    uint64_t t0 = tm->now(tm->ctx);
    int n_iter = (lib_idx == BENCH_L_QF || lib_idx == BENCH_L_LIBM || lib_idx == BENCH_L_FX || lib_idx == BENCH_L_FR)
                     ? BENCH_N_ITER
                     : BENCH_PEER_N_ITER;
    for (int r = 0; r < n_iter; r++)
        for (int i = 0; i < 64; i++)
            *sink = fn(s_in_a[i % BENCH_N_SAMPLES]);
    uint64_t t1 = tm->now(tm->ctx);

    if (lib_idx == BENCH_L_LIBM) {
        out->acc[func_idx][lib_idx] = 0.0;
        out->mse[func_idx][lib_idx] = 0.0;
    } else if (func_idx == BENCH_F_SQRT || func_idx == BENCH_F_HYPOT ||
               func_idx == BENCH_F_HYPOT_FAST2 ||
               func_idx == BENCH_F_HYPOT_FAST ||
               func_idx == BENCH_F_LOG2 || func_idx == BENCH_F_LN ||
               func_idx == BENCH_F_LOG10 || func_idx == BENCH_F_POW2 ||
               func_idx == BENCH_F_EXP || func_idx == BENCH_F_POW10 ||
               func_idx == BENCH_F_POW) {
        out->acc[func_idx][lib_idx] = e.max_rel * 100.0;
        out->mse[func_idx][lib_idx] = err2_mse(&e) * 10000.0;
    } else {
        out->acc[func_idx][lib_idx] = e.max_abs;
        out->mse[func_idx][lib_idx] = err2_mse(&e);
    }
    double elapsed_us = tm->elapsed_us(tm->ctx, t0, t1);
    if (n_iter != BENCH_N_ITER)
        elapsed_us *= (double)BENCH_N_ITER / (double)n_iter;
    out->time_us[func_idx][lib_idx] = elapsed_us;
}

void bench_run_binary_peer(const bench_timer_t *tm,
                           volatile float *sink,
                           bench_results_t *out,
                           int lib_idx,
                           int func_idx,
                           bench_binary_fn_t fn)
{
    if (!fn || lib_idx < 0 || lib_idx >= BENCH_NLIB || func_idx < 0 || func_idx >= BENCH_NFUNC)
        return;
    bench_fill_inputs(func_idx);
    err2_t e;
    err2_clear(&e);
    for (int i = 0; i < BENCH_N_SAMPLES; i++)
        bench_track_metric(&e, func_idx, bench_ref_binary(func_idx, s_in_a[i], s_in_b[i]),
                           (double)fn(s_in_a[i], s_in_b[i]));

    uint64_t t0 = tm->now(tm->ctx);
    int n_iter = (lib_idx == BENCH_L_QF || lib_idx == BENCH_L_LIBM || lib_idx == BENCH_L_FX || lib_idx == BENCH_L_FR)
                     ? BENCH_N_ITER
                     : BENCH_PEER_N_ITER;
    for (int r = 0; r < n_iter; r++)
        for (int i = 0; i < 64; i++) {
            int idx = i % BENCH_N_SAMPLES;
            *sink = fn(s_in_a[idx], s_in_b[idx]);
        }
    uint64_t t1 = tm->now(tm->ctx);

    if (lib_idx == BENCH_L_LIBM) {
        out->acc[func_idx][lib_idx] = 0.0;
        out->mse[func_idx][lib_idx] = 0.0;
    } else if (func_idx == BENCH_F_HYPOT || func_idx == BENCH_F_HYPOT_FAST2 ||
               func_idx == BENCH_F_HYPOT_FAST || func_idx == BENCH_F_POW) {
        out->acc[func_idx][lib_idx] = e.max_rel * 100.0;
        out->mse[func_idx][lib_idx] = err2_mse(&e) * 10000.0;
    } else {
        out->acc[func_idx][lib_idx] = e.max_abs;
        out->mse[func_idx][lib_idx] = err2_mse(&e);
    }
    double elapsed_us = tm->elapsed_us(tm->ctx, t0, t1);
    if (n_iter != BENCH_N_ITER)
        elapsed_us *= (double)BENCH_N_ITER / (double)n_iter;
    out->time_us[func_idx][lib_idx] = elapsed_us;
}

static void bench_sin(const bench_timer_t *tm, volatile float *sink, bench_results_t *out)
{
    float inputs[BENCH_N_SAMPLES];
    for (int i = 0; i < BENCH_N_SAMPLES; i++)
        inputs[i] = -6.2831853f + 12.5663706f * ((float)i / (float)BENCH_N_SAMPLES);

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

    out->acc[BENCH_F_SIN][BENCH_L_QF]   = eqf.max_abs * 100.0;
    out->acc[BENCH_F_SIN][BENCH_L_LIBM] = 0.0;
    out->acc[BENCH_F_SIN][BENCH_L_FX]   = efx.max_abs * 100.0;
    out->acc[BENCH_F_SIN][BENCH_L_FR]   = efr.max_abs * 100.0;
    out->mse[BENCH_F_SIN][BENCH_L_QF]   = err2_mse(&eqf) * 10000.0;
    out->mse[BENCH_F_SIN][BENCH_L_LIBM] = 0.0;
    out->mse[BENCH_F_SIN][BENCH_L_FX]   = err2_mse(&efx) * 10000.0;
    out->mse[BENCH_F_SIN][BENCH_L_FR]   = err2_mse(&efr) * 10000.0;

    out->time_us[BENCH_F_SIN][BENCH_L_QF]   = us_qf;
    out->time_us[BENCH_F_SIN][BENCH_L_LIBM] = us_libm;
    out->time_us[BENCH_F_SIN][BENCH_L_FX]   = us_fx;
    out->time_us[BENCH_F_SIN][BENCH_L_FR]   = us_fr;
    (void)us_poly;
}

static void bench_cos(const bench_timer_t *tm, volatile float *sink, bench_results_t *out)
{
    float inputs[BENCH_N_SAMPLES];
    for (int i = 0; i < BENCH_N_SAMPLES; i++)
        inputs[i] = -6.2831853f + 12.5663706f * ((float)i / (float)BENCH_N_SAMPLES);

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

    out->acc[BENCH_F_COS][BENCH_L_QF]   = eqf.max_abs * 100.0;
    out->acc[BENCH_F_COS][BENCH_L_LIBM] = 0.0;
    out->acc[BENCH_F_COS][BENCH_L_FX]   = efx.max_abs * 100.0;
    out->acc[BENCH_F_COS][BENCH_L_FR]   = efr.max_abs * 100.0;
    out->mse[BENCH_F_COS][BENCH_L_QF]   = err2_mse(&eqf) * 10000.0;
    out->mse[BENCH_F_COS][BENCH_L_LIBM] = 0.0;
    out->mse[BENCH_F_COS][BENCH_L_FX]   = err2_mse(&efx) * 10000.0;
    out->mse[BENCH_F_COS][BENCH_L_FR]   = err2_mse(&efr) * 10000.0;

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

    out->acc[BENCH_F_SQRT][BENCH_L_QF]   = eqf.max_rel * 100.0;
    out->acc[BENCH_F_SQRT][BENCH_L_LIBM] = 0.0;
    out->acc[BENCH_F_SQRT][BENCH_L_FX]   = efx.max_rel * 100.0;
    out->acc[BENCH_F_SQRT][BENCH_L_FR]   = efr.max_rel * 100.0;
    out->mse[BENCH_F_SQRT][BENCH_L_QF]   = err2_mse(&eqf) * 10000.0;
    out->mse[BENCH_F_SQRT][BENCH_L_LIBM] = 0.0;
    out->mse[BENCH_F_SQRT][BENCH_L_FX]   = err2_mse(&efx) * 10000.0;
    out->mse[BENCH_F_SQRT][BENCH_L_FR]   = err2_mse(&efr) * 10000.0;

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

    out->acc[BENCH_F_LN][BENCH_L_QF]   = eqf.max_rel * 100.0;
    out->acc[BENCH_F_LN][BENCH_L_LIBM] = 0.0;
    out->acc[BENCH_F_LN][BENCH_L_FX]   = efx.max_rel * 100.0;
    out->acc[BENCH_F_LN][BENCH_L_FR]   = efr.max_rel * 100.0;
    out->mse[BENCH_F_LN][BENCH_L_QF]   = err2_mse(&eqf) * 10000.0;
    out->mse[BENCH_F_LN][BENCH_L_LIBM] = 0.0;
    out->mse[BENCH_F_LN][BENCH_L_FX]   = err2_mse(&efx) * 10000.0;
    out->mse[BENCH_F_LN][BENCH_L_FR]   = err2_mse(&efr) * 10000.0;

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

    out->acc[BENCH_F_EXP][BENCH_L_QF]   = eqf.max_rel * 100.0;
    out->acc[BENCH_F_EXP][BENCH_L_LIBM] = 0.0;
    out->acc[BENCH_F_EXP][BENCH_L_FX]   = efx.max_rel * 100.0;
    out->acc[BENCH_F_EXP][BENCH_L_FR]   = efr.max_rel * 100.0;
    out->mse[BENCH_F_EXP][BENCH_L_QF]   = err2_mse(&eqf) * 10000.0;
    out->mse[BENCH_F_EXP][BENCH_L_LIBM] = 0.0;
    out->mse[BENCH_F_EXP][BENCH_L_FX]   = err2_mse(&efx) * 10000.0;
    out->mse[BENCH_F_EXP][BENCH_L_FR]   = err2_mse(&efr) * 10000.0;

    out->time_us[BENCH_F_EXP][BENCH_L_QF]   = us_qf;
    out->time_us[BENCH_F_EXP][BENCH_L_LIBM] = us_libm;
    out->time_us[BENCH_F_EXP][BENCH_L_FX]   = us_fx;
    out->time_us[BENCH_F_EXP][BENCH_L_FR]   = us_fr;
}

static float libm_tan_f(float x) { return tanf(x); }
static float libm_sin_deg_f(float x) { return sinf(x * BENCH_DEG2RAD); }
static float libm_cos_deg_f(float x) { return cosf(x * BENCH_DEG2RAD); }
static float libm_tan_deg_f(float x) { return tanf(x * BENCH_DEG2RAD); }
static float libm_sin_bam_f(float x) { return sinf((float)((uint16_t)((int32_t)x)) * BENCH_BAM2RAD); }
static float libm_cos_bam_f(float x) { return cosf((float)((uint16_t)((int32_t)x)) * BENCH_BAM2RAD); }
static float libm_tan_bam_f(float x) { return tanf((float)((uint16_t)((int32_t)x)) * BENCH_BAM2RAD); }
static float libm_asin_f(float x) { return asinf(x); }
static float libm_acos_f(float x) { return acosf(x); }
static float libm_atan_f(float x) { return atanf(x); }
static float libm_atan2_f(float y, float x) { return atan2f(y, x); }
static float libm_hypot_f(float x, float y) { return hypotf(x, y); }
static float libm_pow10_f(float x) { return powf(10.0f, x); }
static float libm_pow_f(float x, float y) { return powf(x, y); }
static float qf_sin_bam_f(float x) { return qf_sin_bam((uint16_t)((int32_t)x)); }
static float qf_cos_bam_f(float x) { return qf_cos_bam((uint16_t)((int32_t)x)); }
static float qf_tan_bam_f(float x) { return qf_tan_bam((uint16_t)((int32_t)x)); }

void bench_run_all(const bench_timer_t *timer, volatile float *sink, bench_results_t *out)
{
    bench_results_init(out);
    timer->init(timer->ctx);
    bench_sin(timer, sink, out);
    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_SIN_DEG, qf_sin_deg);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_SIN_DEG, libm_sin_deg_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_SIN_DEG, fix16_sin_deg_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_SIN_DEG, fr_sin_deg_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_SIN_BAM, qf_sin_bam_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_SIN_BAM, libm_sin_bam_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_SIN_BAM, fix16_sin_bam_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_SIN_BAM, fr_sin_bam_f);

    bench_cos(timer, sink, out);
    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_COS_DEG, qf_cos_deg);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_COS_DEG, libm_cos_deg_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_COS_DEG, fix16_cos_deg_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_COS_DEG, fr_cos_deg_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_COS_BAM, qf_cos_bam_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_COS_BAM, libm_cos_bam_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_COS_BAM, fix16_cos_bam_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_COS_BAM, fr_cos_bam_f);

    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_TAN, qf_tan);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_TAN, libm_tan_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_TAN, fix16_tan_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_TAN, fr_tan_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_TAN_DEG, qf_tan_deg);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_TAN_DEG, libm_tan_deg_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_TAN_DEG, fix16_tan_deg_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_TAN_DEG, fr_tan_deg_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_TAN_BAM, qf_tan_bam_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_TAN_BAM, libm_tan_bam_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_TAN_BAM, fix16_tan_bam_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_TAN_BAM, fr_tan_bam_f);

    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_ASIN, qf_asin);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_ASIN, libm_asin_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_ASIN, fix16_asin_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_ASIN, fr_asin_f);

    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_ACOS, qf_acos);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_ACOS, libm_acos_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_ACOS, fix16_acos_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_ACOS, fr_acos_f);

    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_ATAN, qf_atan);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_ATAN, libm_atan_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_ATAN, fix16_atan_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_ATAN, fr_atan_f);

    bench_run_binary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_ATAN2, qf_atan2);
    bench_run_binary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_ATAN2, libm_atan2_f);
    bench_run_binary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_ATAN2, fix16_atan2_float);
    bench_run_binary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_ATAN2, fr_atan2_f);

    bench_sqrt(timer, sink, out);
    bench_run_binary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_HYPOT, qf_hypot);
    bench_run_binary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_HYPOT, libm_hypot_f);
    bench_run_binary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_HYPOT, fr_hypot_f);
    bench_run_binary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_HYPOT_FAST2, qf_hypot_fast2);
    bench_run_binary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_HYPOT_FAST2, libm_hypot_f);
    bench_run_binary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_HYPOT_FAST, qf_hypot_fast8);
    bench_run_binary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_HYPOT_FAST, libm_hypot_f);
    bench_run_binary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_HYPOT_FAST, fr_hypot_fast_f);

    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_LOG2, qf_log2);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_LOG2, log2f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_LOG2, fix16_log2_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_LOG2, fr_log2_f);
    bench_ln(timer, sink, out);
    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_LOG10, qf_log10);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_LOG10, log10f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_LOG10, fix16_log10_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_LOG10, fr_log10_f);

    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_POW2, qf_pow2);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_POW2, exp2f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_POW2, fix16_pow2_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_POW2, fr_pow2_f);
    bench_exp(timer, sink, out);
    bench_run_unary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_POW10, qf_pow10);
    bench_run_unary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_POW10, libm_pow10_f);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_POW10, fix16_pow10_float);
    bench_run_unary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_POW10, fr_pow10_f);

    bench_run_binary_peer(timer, sink, out, BENCH_L_QF, BENCH_F_POW, qf_pow);
    bench_run_binary_peer(timer, sink, out, BENCH_L_LIBM, BENCH_F_POW, libm_pow_f);
    bench_run_binary_peer(timer, sink, out, BENCH_L_FX, BENCH_F_POW, fix16_pow_float);
    bench_run_binary_peer(timer, sink, out, BENCH_L_FR, BENCH_F_POW, fr_pow_f);
}

void bench_print_human(const bench_results_t *r)
{
    printf("qf_math comparison benchmark suite\n");
    printf("===================================\n");
    printf("Samples: %d  outer iterations: %d (x64 inner calls)\n", BENCH_N_SAMPLES, BENCH_N_ITER);
    printf("Use --markdown-body or make compare-github-report for generated tables.\n\n");
    printf("Quick speed ratios vs libm:\n");
    for (int f = 0; f < BENCH_NFUNC; f++) {
        printf("  %-6s  qf_math %.3fx  libfixmath %.3fx  fr_math %.3fx\n",
               bench_func_name(f),
               bench_ratio_vs_libm(r->time_us[f][BENCH_L_LIBM], r->time_us[f][BENCH_L_QF]),
               bench_ratio_vs_libm(r->time_us[f][BENCH_L_LIBM], r->time_us[f][BENCH_L_FX]),
               bench_ratio_vs_libm(r->time_us[f][BENCH_L_LIBM], r->time_us[f][BENCH_L_FR]));
    }
    printf("\n");
}

static void bench_md_fixed6_cell_bold(FILE *out, double v, int bold)
{
    if (isnan(v)) {
        fprintf(out, " --- |");
        return;
    }

    if (bold)
        fprintf(out, " **%.6f** |", v);
    else
        fprintf(out, " %.6f |", v);
}

static void bench_md_fixed2_cell_bold(FILE *out, double v, int bold)
{
    if (isnan(v)) {
        fprintf(out, " --- |");
        return;
    }

    if (bold)
        fprintf(out, " **%.2f** |", v);
    else
        fprintf(out, " %.2f |", v);
}

static int bench_best_low_cell(const double row[BENCH_NLIB], int include_libm)
{
    int best = -1;
    for (int l = 0; l < BENCH_NLIB; l++) {
        if (!include_libm && l == BENCH_L_LIBM)
            continue;
        if (isnan(row[l]))
            continue;
        if (best < 0 || row[l] < row[best])
            best = l;
    }
    return best;
}

static int bench_best_speed_cell(const bench_results_t *r, int func_idx)
{
    int best = -1;
    for (int l = 0; l < BENCH_NLIB; l++) {
        if (isnan(r->time_us[func_idx][l]))
            continue;
        if (best < 0 || r->time_us[func_idx][l] < r->time_us[func_idx][best])
            best = l;
    }
    return best;
}

static int bench_lib_display_idx(int pos)
{
    static const int order[BENCH_NLIB] = {
        BENCH_L_LIBM,
        BENCH_L_QF,
        BENCH_L_FX,
        BENCH_L_FR,
        BENCH_L_FASTTRIG,
        BENCH_L_ESPDSP,
        BENCH_L_ESPP,
    };
    return (pos >= 0 && pos < BENCH_NLIB) ? order[pos] : pos;
}

void bench_emit_markdown_tables(FILE *out, const bench_results_t *r, bench_md_style_t style)
{
    (void)style;
    fprintf(out, "### Accuracy — peak error\n\n");
    fprintf(out,
            "One generated matrix for all benchmarked functions over %d sample points. Sine/cosine "
            "and tangent rad/deg rows sweep signed `-rotation..+rotation` inputs; BAM rows sweep one "
            "unsigned cycle. Tangent pole samples "
            "use each implementation family’s saturation magnitude (`QF_TAN_MAX` for qf/float peers, "
            "fixed-point range for fixed-point peers), and fixed-point tangent peers are scored "
            "against the quantized bridge-domain angle. The `Metric` column defines each "
            "peak value: `abs %%FS` for sine/cosine output amplitude, `abs` for tangent, "
            "`abs rad` for inverse trig, and `rel %%` for sqrt/hypot/log/exp/pow. Bold marks the best "
            "non-`libm` approximation in each row.\n\n",
            BENCH_N_SAMPLES);
    fprintf(out, "| Function | Metric |");
    for (int pos = 0; pos < BENCH_NLIB; pos++) {
        int l = bench_lib_display_idx(pos);
        fprintf(out, " %s |", bench_lib_name(l));
    }
    fprintf(out, "\n| :--- | :--- |");
    for (int l = 0; l < BENCH_NLIB; l++)
        fprintf(out, " ---:|");
    fprintf(out, "\n");

    for (int f = 0; f < BENCH_NFUNC; f++) {
        int best = bench_best_low_cell(r->acc[f], 0);
        fprintf(out, "| `%s` | %s |", bench_func_name(f), bench_func_metric(f));
        for (int pos = 0; pos < BENCH_NLIB; pos++) {
            int l = bench_lib_display_idx(pos);
            bench_md_fixed6_cell_bold(out, r->acc[f][l], l == best);
        }
        fprintf(out, "\n");
    }

    fprintf(out, "\n### Accuracy — mean squared error\n\n");
    fprintf(out,
            "Mean squared error uses the same metric units as the peak-error table, squared. "
            "Bold marks the best non-`libm` approximation in each row.\n\n");
    fprintf(out, "| Function | Metric squared |");
    for (int pos = 0; pos < BENCH_NLIB; pos++) {
        int l = bench_lib_display_idx(pos);
        fprintf(out, " %s |", bench_lib_name(l));
    }
    fprintf(out, "\n| :--- | :--- |");
    for (int l = 0; l < BENCH_NLIB; l++)
        fprintf(out, " ---:|");
    fprintf(out, "\n");

    for (int f = 0; f < BENCH_NFUNC; f++) {
        int best = bench_best_low_cell(r->mse[f], 0);
        fprintf(out, "| `%s` | %s^2 |", bench_func_name(f), bench_func_metric(f));
        for (int pos = 0; pos < BENCH_NLIB; pos++) {
            int l = bench_lib_display_idx(pos);
            bench_md_fixed6_cell_bold(out, r->mse[f][l], l == best);
        }
        fprintf(out, "\n");
    }

    fprintf(out, "\n### Wall-clock time (microseconds)\n\n");
    fprintf(out,
            "Total microseconds normalized to the metadata loop shape. ESP32-only peer rows may be "
            "measured with a shorter loop and scaled to the same outer-iteration count. Unsupported cells are `---`.\n\n");
    fprintf(out, "| Function |");
    for (int pos = 0; pos < BENCH_NLIB; pos++) {
        int l = bench_lib_display_idx(pos);
        fprintf(out, " %s |", bench_lib_name(l));
    }
    fprintf(out, "\n| :--- |");
    for (int l = 0; l < BENCH_NLIB; l++)
        fprintf(out, " ---:|");
    fprintf(out, "\n");

    for (int f = 0; f < BENCH_NFUNC; f++) {
        int best = bench_best_speed_cell(r, f);
        fprintf(out, "| `%s` |", bench_func_name(f));
        for (int pos = 0; pos < BENCH_NLIB; pos++) {
            int l = bench_lib_display_idx(pos);
            bench_md_fixed6_cell_bold(out, r->time_us[f][l], l == best);
        }
        fprintf(out, "\n");
    }

    fprintf(out, "\n### Speed vs libm (ratio)\n\n");
    fprintf(out,
            "`libm` time ÷ implementation time for the same function on this platform. Above 1.0 means "
            "faster than the platform `libm` call in this microbenchmark. Ratios are rounded to two "
            "decimal places because smaller differences are usually noise.\n\n");
    fprintf(out,
            "**Fixed-point peer rows are bridged harness timings**: `float`→fixed→function→`float`. "
            "They keep the report comparable, but are not native `fix16_t` / `s32` pipeline timings.\n\n");
    fprintf(out, "| Function |");
    for (int pos = 0; pos < BENCH_NLIB; pos++) {
        int l = bench_lib_display_idx(pos);
        fprintf(out, " %s |", bench_lib_name(l));
    }
    fprintf(out, "\n| :--- |");
    for (int l = 0; l < BENCH_NLIB; l++)
        fprintf(out, " ---:|");
    fprintf(out, "\n");

    for (int f = 0; f < BENCH_NFUNC; f++) {
        int best = bench_best_speed_cell(r, f);
        fprintf(out, "| `%s` |", bench_func_name(f));
        for (int pos = 0; pos < BENCH_NLIB; pos++) {
            int l = bench_lib_display_idx(pos);
            if (l == BENCH_L_LIBM && !isnan(r->time_us[f][l]))
                bench_md_fixed2_cell_bold(out, 1.0, l == best);
            else if (isnan(r->time_us[f][l]) || isnan(r->time_us[f][BENCH_L_LIBM]))
                fprintf(out, " --- |");
            else
                bench_md_fixed2_cell_bold(out,
                                          bench_ratio_vs_libm(r->time_us[f][BENCH_L_LIBM],
                                                              r->time_us[f][l]),
                                          l == best);
        }
        fprintf(out, "\n");
    }

    fprintf(out, "\n");
}

void bench_emit_markdown_doc_snapshot(FILE *out, const bench_results_t *r, const char *device_line)
{
    fprintf(out,
            "<!-- MCU benchmark snapshot — paste into docs (`compare/MCU_BENCHMARK_SNAPSHOT_<TARGET>.md`). "
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
