/**
 *  @file qf_math_test.c - Tests for the qf_math quick float math library
 *
 *  Tests all function groups: macros, trig, inverse trig, log/exp,
 *  sqrt/hypot, waves, ADSR, and qf<->fixed-radix conversions.
 *
 *  Uses libm as reference. Error tolerances match table-based precision.
 *
 *  Copyright (c) 2002-2026, M. A. Chatterjee
 *  All rights reserved. BSD-2-Clause — see qf_math.h for full text.
 */

#include <stdio.h>
#include <math.h>
#include <float.h>
#include "qf_math.h"

/*=======================================================
 * Test infrastructure
 */

static int g_test_count = 0;
static int g_fail_count = 0;

#define TEST_PASS 0
#define TEST_FAIL 1

#define RUN_TEST(fn) do { \
    printf("  %-36s ", #fn); \
    g_test_count++; \
    if ((fn)() == TEST_PASS) { printf("PASS\n"); } \
    else { printf("FAIL\n"); g_fail_count++; } \
} while(0)

/* Check that |a - b| <= tol.  Returns 1 on pass, 0 on fail. */
static int approx_eq(qf a, qf b, qf tol)
{
    qf d = a - b;
    if (d < 0.0f) d = -d;
    return d <= tol;
}

static double dabs_d(double x)
{
    return x < 0.0 ? -x : x;
}

/* Compare float implementation vs libm double reference. */
static int approx_vs_double_abs(float got, double ref, double tol_abs)
{
    return dabs_d((double)got - ref) <= tol_abs;
}

static int approx_vs_double_rel(float got, double ref, double tol_rel)
{
    double g = (double)got;
    if (dabs_d(ref) < 1e-300)
        return dabs_d(g - ref) <= tol_rel;
    return dabs_d((g - ref) / ref) <= tol_rel;
}

/* Tolerance constants */
#define TRIG_TOL   5.0e-4f   /* table-based trig: ~129 entries + lerp     */
#define LOG_TOL    1.0e-3f   /* polynomial log2(1+t) on mantissa         */
#define EXP_TOL    1.0e-3f   /* table-based pow2: 65 entries + lerp       */
#define SQRT_TOL   1.0e-4f   /* 2 Newton-Raphson iterations               */
#define HYPOT_TOL  2.0e-3f   /* piecewise-linear fast8: ~0.1% peak error  */
#define ATRIG_TOL  2.0e-3f   /* inverse trig via table binary search      */
#define CONV_TOL   1.0e-4f   /* f32<->fixed-radix round-trip              */

/* Double-reference sweeps (libm in double). */
#define SWEEP_SINCOS_ABS   6.5e-4
#define SWEEP_TAN_ABS      1.2e-3
#define SWEEP_LOG_REL      4.5e-3
#define SWEEP_EXP_REL      2.6e-3
#define SWEEP_SQRT_REL     3e-4
#define SWEEP_ATAN2_ABS    3e-3f

/*=======================================================
 * Test: macros and constants
 */

static int test_macros(void)
{
    if (!approx_eq(QF_ABS(-3.14f),   3.14f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_ABS(2.71f),    2.71f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_SGN(-5.0f),   -1.0f,   1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_SGN(0.0f),     0.0f,   1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_SGN(5.0f),     1.0f,   1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_MIN(3.0f, 7.0f), 3.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_MAX(3.0f, 7.0f), 7.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_CLAMP(10.0f, 2.0f, 5.0f), 5.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_CLAMP(-1.0f, 2.0f, 5.0f), 2.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_CLAMP(3.0f, 2.0f, 5.0f),  3.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_INTERP(0.0f, 10.0f, 0.5f), 5.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_INTERP(0.0f, 10.0f, 0.0f), 0.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(QF_INTERP(0.0f, 10.0f, 1.0f), 10.0f, 1e-6f)) return TEST_FAIL;
    return TEST_PASS;
}

/*=======================================================
 * Test: qf <-> fixed-radix conversions
 */

static int test_conversions(void)
{
    /* qf -> fixed-radix -> qf round-trip */
    qf vals[] = { 0.0f, 1.0f, -1.0f, 1.5f, -2.75f, 3.14159f, 0.001f, 100.0f };
    int n = (int)(sizeof(vals) / sizeof(vals[0]));

    for (int i = 0; i < n; i++) {
        int32_t fr = QF_TO_FR(vals[i], 16);
        qf back = FR_TO_QF(fr, 16);
        if (!approx_eq(back, vals[i], CONV_TOL)) return TEST_FAIL;
    }

    /* Round-to-nearest conversion */
    int32_t rnd = QF_TO_FR_RND(1.5f, 8);
    qf rnd_back = FR_TO_QF(rnd, 8);
    if (!approx_eq(rnd_back, 1.5f, 0.01f)) return TEST_FAIL;

    /* Integer conversions */
    if (QF_TO_INT(3.7f) != 3) return TEST_FAIL;
    if (QF_ROUND(3.7f) != 4) return TEST_FAIL;
    if (QF_ROUND(-3.7f) != -4) return TEST_FAIL;
    if (!approx_eq(QF_FROM_INT(42), 42.0f, 1e-6f)) return TEST_FAIL;

    return TEST_PASS;
}

/*=======================================================
 * Test: angular conversion macros
 */

static int test_angular_conversions(void)
{
    if (!approx_eq(QF_DEG_TO_RAD(180.0f), QF_PI,  1e-4f)) return TEST_FAIL;
    if (!approx_eq(QF_RAD_TO_DEG(QF_PI), 180.0f,  0.01f)) return TEST_FAIL;
    if (!approx_eq(QF_DEG_TO_RAD(90.0f),  QF_HALF_PI, 1e-4f)) return TEST_FAIL;

    /* BAM conversions */
    if (!approx_eq(QF_BAM_TO_DEG(16384), 90.0f,  0.01f)) return TEST_FAIL;
    if (!approx_eq(QF_BAM_TO_DEG(32768), 180.0f, 0.01f)) return TEST_FAIL;
    if (!approx_eq(QF_BAM_TO_RAD(16384), QF_HALF_PI, 0.001f)) return TEST_FAIL;

    uint16_t bam90 = QF_DEG_TO_BAM(90.0f);
    if (bam90 < 16380 || bam90 > 16388) return TEST_FAIL;

    return TEST_PASS;
}

/*=======================================================
 * Test: sin — BAM, radian, and degree variants
 */

static int test_sin(void)
{
    /* Cardinal angles (exact) */
    if (!approx_eq(qf_sin(0.0f),         0.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_sin(QF_HALF_PI),  1.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_sin(QF_PI),       0.0f,  1e-4f)) return TEST_FAIL;
    if (!approx_eq(qf_sin(QF_PI * 1.5f), -1.0f, 1e-4f)) return TEST_FAIL;

    /* Negative input */
    if (!approx_eq(qf_sin(-QF_HALF_PI), -1.0f, 1e-6f)) return TEST_FAIL;

    /* Sweep 0..360 in 5-degree steps, compare to libm sinf */
    for (int deg = 0; deg <= 360; deg += 5) {
        qf rad = (qf)deg * QF_DEG2RAD_K;
        qf ref = sinf(rad);
        qf got = qf_sin(rad);
        if (!approx_eq(got, ref, TRIG_TOL)) return TEST_FAIL;
    }

    /* BAM-native */
    if (!approx_eq(qf_sin_bam(0),      0.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_sin_bam(16384),  1.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_sin_bam(32768),  0.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_sin_bam(49152), -1.0f,  1e-6f)) return TEST_FAIL;

    /* Degree */
    if (!approx_eq(qf_sin_deg(30.0f),  0.5f,   TRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_sin_deg(90.0f),  1.0f,   TRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_sin_deg(-30.0f), -0.5f,  TRIG_TOL)) return TEST_FAIL;

    return TEST_PASS;
}

/*=======================================================
 * Test: cos — BAM, radian, and degree variants
 */

static int test_cos(void)
{
    if (!approx_eq(qf_cos(0.0f),         1.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_cos(QF_HALF_PI),  0.0f,  1e-4f)) return TEST_FAIL;
    if (!approx_eq(qf_cos(QF_PI),      -1.0f,  1e-4f)) return TEST_FAIL;

    /* cos is even */
    if (!approx_eq(qf_cos(-QF_HALF_PI), 0.0f, 1e-4f)) return TEST_FAIL;

    /* Sweep and compare to libm */
    for (int deg = 0; deg <= 360; deg += 5) {
        qf rad = (qf)deg * QF_DEG2RAD_K;
        qf ref = cosf(rad);
        qf got = qf_cos(rad);
        if (!approx_eq(got, ref, TRIG_TOL)) return TEST_FAIL;
    }

    /* BAM-native */
    if (!approx_eq(qf_cos_bam(0),      1.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_cos_bam(16384),  0.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_cos_bam(32768), -1.0f,  1e-6f)) return TEST_FAIL;

    /* Degree */
    if (!approx_eq(qf_cos_deg(60.0f),   0.5f,  TRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_cos_deg(0.0f),    1.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_cos_deg(180.0f), -1.0f,  TRIG_TOL)) return TEST_FAIL;

    return TEST_PASS;
}

/*=======================================================
 * Test: tan — BAM, radian, and degree variants
 */

static int test_tan(void)
{
    if (!approx_eq(qf_tan(0.0f), 0.0f, 1e-6f)) return TEST_FAIL;

    /* tan(pi/4) = 1.0 */
    qf pi_4 = QF_PI * 0.25f;
    if (!approx_eq(qf_tan(pi_4), 1.0f, TRIG_TOL)) return TEST_FAIL;

    /* tan(-pi/4) = -1.0 */
    if (!approx_eq(qf_tan(-pi_4), -1.0f, TRIG_TOL)) return TEST_FAIL;

    /* Sweep and compare to libm (avoid near-pole angles) */
    for (int deg = 5; deg <= 80; deg += 5) {
        qf rad = (qf)deg * QF_DEG2RAD_K;
        qf ref = tanf(rad);
        qf got = qf_tan(rad);
        if (!approx_eq(got, ref, QF_ABS(ref) * 0.01f + 0.001f)) return TEST_FAIL;
    }

    /* Saturation near poles */
    qf near_pole = qf_tan(QF_HALF_PI - 0.0001f);
    if (near_pole < 1000.0f) return TEST_FAIL;

    /* 3pi/2 pole: offset large enough to use 1/ad (covers qf_tan second pole block). */
    {
        qf p = qf_tan(4.71238898f - 1e-4f);
        if (p < 5000.0f || p > QF_TAN_MAX) return TEST_FAIL;
    }

    /* BAM-native */
    if (!approx_eq(qf_tan_bam(0),     0.0f, 1e-6f)) return TEST_FAIL;
    if (qf_tan_bam(16384) < QF_TAN_MAX * 0.9f) return TEST_FAIL;

    /* Degree */
    if (!approx_eq(qf_tan_deg(45.0f),  1.0f,  TRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_tan_deg(0.0f),   0.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_tan_deg(-45.0f), -1.0f, TRIG_TOL)) return TEST_FAIL;

    return TEST_PASS;
}

/*=======================================================
 * Test: trig identity sin^2 + cos^2 = 1
 */

static int test_trig_identity(void)
{
    for (int deg = 0; deg <= 720; deg += 7) {
        qf rad = (qf)deg * QF_DEG2RAD_K;
        qf s = qf_sin(rad);
        qf c = qf_cos(rad);
        qf sum = s * s + c * c;
        if (!approx_eq(sum, 1.0f, 0.002f)) return TEST_FAIL;
    }
    return TEST_PASS;
}

/*=======================================================
 * Test: large angle reduction
 */

static int test_large_angles(void)
{
    /* sin and cos should handle angles >> 2*pi */
    qf s1 = qf_sin(0.5f);
    qf s2 = qf_sin(0.5f + QF_TWO_PI * 10.0f);
    if (!approx_eq(s1, s2, 0.002f)) return TEST_FAIL;

    qf c1 = qf_cos(1.0f);
    qf c2 = qf_cos(1.0f + QF_TWO_PI * 5.0f);
    if (!approx_eq(c1, c2, 0.002f)) return TEST_FAIL;

    /* Negative large angles */
    qf s3 = qf_sin(-0.5f);
    qf s4 = qf_sin(-0.5f - QF_TWO_PI * 10.0f);
    if (!approx_eq(s3, s4, 0.002f)) return TEST_FAIL;

    return TEST_PASS;
}

/*=======================================================
 * Test: inverse trig
 */

static int test_acos(void)
{
    if (!approx_eq(qf_acos(1.0f),   0.0f,      1e-6f))  return TEST_FAIL;
    if (!approx_eq(qf_acos(-1.0f),  QF_PI,    1e-6f))  return TEST_FAIL;
    if (!approx_eq(qf_acos(0.0f),   QF_HALF_PI, ATRIG_TOL)) return TEST_FAIL;

    /* Sweep and compare to libm */
    for (int i = -10; i <= 10; i++) {
        qf x = (qf)i * 0.1f;
        qf ref = acosf(x);
        qf got = qf_acos(x);
        if (!approx_eq(got, ref, ATRIG_TOL)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_asin(void)
{
    if (!approx_eq(qf_asin(0.0f),  0.0f,       1e-6f))  return TEST_FAIL;
    if (!approx_eq(qf_asin(1.0f),  QF_HALF_PI, ATRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_asin(-1.0f), -QF_HALF_PI, ATRIG_TOL)) return TEST_FAIL;

    for (int i = -10; i <= 10; i++) {
        qf x = (qf)i * 0.1f;
        qf ref = asinf(x);
        qf got = qf_asin(x);
        if (!approx_eq(got, ref, ATRIG_TOL)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_atan2(void)
{
    /* Axis cases */
    if (!approx_eq(qf_atan2(0.0f, 1.0f),   0.0f,       1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_atan2(0.0f, -1.0f),  QF_PI,      ATRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_atan2(1.0f, 0.0f),   QF_HALF_PI, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_atan2(-1.0f, 0.0f), -QF_HALF_PI, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_atan2(0.0f, 0.0f),   0.0f,        1e-6f)) return TEST_FAIL;

    /* 45-degree angles */
    if (!approx_eq(qf_atan2(1.0f, 1.0f),  QF_PI * 0.25f, ATRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_atan2(-1.0f, 1.0f), -QF_PI * 0.25f, ATRIG_TOL)) return TEST_FAIL;

    /* Sweep all quadrants */
    for (int deg = 0; deg < 360; deg += 15) {
        qf rad = (qf)deg * QF_DEG2RAD_K;
        qf x = cosf(rad);
        qf y = sinf(rad);
        qf ref = atan2f(y, x);
        qf got = qf_atan2(y, x);
        if (!approx_eq(got, ref, ATRIG_TOL)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_atan(void)
{
    if (!approx_eq(qf_atan(0.0f), 0.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_atan(1.0f), QF_PI * 0.25f, ATRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_atan(-1.0f), -QF_PI * 0.25f, ATRIG_TOL)) return TEST_FAIL;

    /* Very large input → saturates to ±pi/2 */
    if (!approx_eq(qf_atan(QF_TAN_MAX * 2.0f), QF_HALF_PI, 1e-4f)) return TEST_FAIL;
    if (!approx_eq(qf_atan(-QF_TAN_MAX * 2.0f), -QF_HALF_PI, 1e-4f)) return TEST_FAIL;

    return TEST_PASS;
}

/*=======================================================
 * Test: log2, ln, log10
 */

static int test_log2(void)
{
    /* Exact powers of 2 */
    if (!approx_eq(qf_log2(1.0f),   0.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_log2(2.0f),   1.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_log2(4.0f),   2.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_log2(8.0f),   3.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_log2(0.5f),  -1.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_log2(0.25f), -2.0f, 1e-6f)) return TEST_FAIL;

    /* Non-power-of-2 values */
    for (int i = 1; i <= 20; i++) {
        qf x = (qf)i * 0.5f;
        qf ref = log2f(x);
        qf got = qf_log2(x);
        if (!approx_eq(got, ref, LOG_TOL)) return TEST_FAIL;
    }

    /* Domain error */
    if (qf_log2(0.0f) != QF_DOMAIN_ERROR) return TEST_FAIL;
    if (qf_log2(-1.0f) != QF_DOMAIN_ERROR) return TEST_FAIL;

    return TEST_PASS;
}

static int test_ln(void)
{
    if (!approx_eq(qf_ln(1.0f), 0.0f,  1e-5f)) return TEST_FAIL;
    if (!approx_eq(qf_ln(QF_E), 1.0f, LOG_TOL)) return TEST_FAIL;

    for (int i = 1; i <= 10; i++) {
        qf x = (qf)i;
        qf ref = logf(x);
        qf got = qf_ln(x);
        if (!approx_eq(got, ref, LOG_TOL)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_log10(void)
{
    if (!approx_eq(qf_log10(1.0f),    0.0f, 1e-5f)) return TEST_FAIL;
    if (!approx_eq(qf_log10(10.0f),   1.0f, LOG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_log10(100.0f),  2.0f, LOG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_log10(1000.0f), 3.0f, LOG_TOL)) return TEST_FAIL;
    return TEST_PASS;
}

/*=======================================================
 * Test: pow2, exp, pow10
 */

static int test_pow2(void)
{
    /* Exact integer powers */
    if (!approx_eq(qf_pow2(0.0f),  1.0f,  1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_pow2(1.0f),  2.0f,  1e-4f)) return TEST_FAIL;
    if (!approx_eq(qf_pow2(2.0f),  4.0f,  1e-4f)) return TEST_FAIL;
    if (!approx_eq(qf_pow2(3.0f),  8.0f,  1e-3f)) return TEST_FAIL;
    if (!approx_eq(qf_pow2(-1.0f), 0.5f,  1e-4f)) return TEST_FAIL;
    if (!approx_eq(qf_pow2(-2.0f), 0.25f, 1e-4f)) return TEST_FAIL;

    /* Fractional powers */
    if (!approx_eq(qf_pow2(0.5f), QF_SQRT2, EXP_TOL)) return TEST_FAIL;

    /* log2/pow2 round-trip */
    qf vals[] = { 0.1f, 0.5f, 1.0f, 2.5f, 7.3f, 100.0f };
    int n = (int)(sizeof(vals) / sizeof(vals[0]));
    for (int i = 0; i < n; i++) {
        qf rt = qf_pow2(qf_log2(vals[i]));
        if (!approx_eq(rt, vals[i], vals[i] * 0.002f)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_exp(void)
{
    if (!approx_eq(qf_exp(0.0f), 1.0f,   1e-5f))  return TEST_FAIL;
    if (!approx_eq(qf_exp(1.0f), QF_E,  EXP_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_exp(-1.0f), QF_INV_E, EXP_TOL)) return TEST_FAIL;

    for (int i = -5; i <= 5; i++) {
        qf x = (qf)i;
        qf ref = expf(x);
        qf got = qf_exp(x);
        if (!approx_eq(got, ref, ref * 0.002f + 0.001f)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_pow10(void)
{
    if (!approx_eq(qf_pow10(0.0f),  1.0f,    1e-5f))  return TEST_FAIL;
    if (!approx_eq(qf_pow10(1.0f),  10.0f,   EXP_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_pow10(2.0f),  100.0f,  0.5f))   return TEST_FAIL;
    if (!approx_eq(qf_pow10(-1.0f), 0.1f,    EXP_TOL)) return TEST_FAIL;
    return TEST_PASS;
}

static int test_pow(void)
{
    if (!approx_eq(qf_pow(2.0f, 3.0f), 8.0f, EXP_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_pow(9.0f, 0.5f), 3.0f, EXP_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_pow(10.0f, -1.0f), 0.1f, EXP_TOL)) return TEST_FAIL;
    if (qf_pow(0.0f, 2.0f) != QF_DOMAIN_ERROR) return TEST_FAIL;
    if (qf_pow(-2.0f, 3.0f) != QF_DOMAIN_ERROR) return TEST_FAIL;
    return TEST_PASS;
}

/*=======================================================
 * Test: sqrt
 */

static int test_sqrt(void)
{
    if (!approx_eq(qf_sqrt(0.0f),  0.0f,       1e-6f))  return TEST_FAIL;
    if (!approx_eq(qf_sqrt(1.0f),  1.0f,       SQRT_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_sqrt(4.0f),  2.0f,       SQRT_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_sqrt(9.0f),  3.0f,       SQRT_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_sqrt(2.0f),  QF_SQRT2,  SQRT_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_sqrt(0.25f), 0.5f,       SQRT_TOL)) return TEST_FAIL;

    /* Domain error */
    if (qf_sqrt(-1.0f) != QF_DOMAIN_ERROR) return TEST_FAIL;

    /* Sweep */
    for (int i = 1; i <= 100; i++) {
        qf x = (qf)i;
        qf ref = sqrtf(x);
        qf got = qf_sqrt(x);
        if (!approx_eq(got, ref, SQRT_TOL)) return TEST_FAIL;
    }
    return TEST_PASS;
}

/*=======================================================
 * Test: hypot, hypot_fast2, hypot_fast8
 */

static int test_hypot(void)
{
    if (!approx_eq(qf_hypot(3.0f, 4.0f), 5.0f, SQRT_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_hypot(0.0f, 5.0f), 5.0f, SQRT_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_hypot(5.0f, 0.0f), 5.0f, SQRT_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_hypot(0.0f, 0.0f), 0.0f, 1e-6f))   return TEST_FAIL;
    if (!approx_eq(qf_hypot(-3.0f, -4.0f), 5.0f, SQRT_TOL)) return TEST_FAIL;
    return TEST_PASS;
}

static int test_hypot_fast2(void)
{
    /* 3-4-5 triangle */
    if (!approx_eq(qf_hypot_fast2(3.0f, 4.0f), 5.0f, 0.015f * 5.0f)) return TEST_FAIL;

    /* Axis-aligned */
    if (!approx_eq(qf_hypot_fast2(0.0f, 7.0f), 7.0f, 0.015f * 7.0f)) return TEST_FAIL;
    if (!approx_eq(qf_hypot_fast2(7.0f, 0.0f), 7.0f, 0.015f * 7.0f)) return TEST_FAIL;

    /* Symmetry */
    if (!approx_eq(qf_hypot_fast2(3.0f, 4.0f),
                   qf_hypot_fast2(-3.0f, -4.0f), 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_hypot_fast2(3.0f, 4.0f),
                   qf_hypot_fast2(4.0f, 3.0f), 1e-6f)) return TEST_FAIL;

    /* Zero */
    if (!approx_eq(qf_hypot_fast2(0.0f, 0.0f), 0.0f, 1e-6f)) return TEST_FAIL;

    /* Sweep 0..90 degrees, check error is within 1.5% of true hypot */
    for (int deg = 0; deg <= 90; deg += 3) {
        qf rad = (qf)deg * QF_DEG2RAD_K;
        qf x = cosf(rad) * 1000.0f;
        qf y = sinf(rad) * 1000.0f;
        qf ref = sqrtf(x * x + y * y);
        qf got = qf_hypot_fast2(x, y);
        qf err = QF_ABS(got - ref) / ref;
        if (err > 0.015f) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_hypot_fast8(void)
{
    /* 3-4-5 triangle */
    if (!approx_eq(qf_hypot_fast8(3.0f, 4.0f), 5.0f, HYPOT_TOL * 5.0f)) return TEST_FAIL;

    /* Axis-aligned */
    if (!approx_eq(qf_hypot_fast8(0.0f, 7.0f), 7.0f, HYPOT_TOL * 7.0f)) return TEST_FAIL;
    if (!approx_eq(qf_hypot_fast8(7.0f, 0.0f), 7.0f, HYPOT_TOL * 7.0f)) return TEST_FAIL;

    /* Symmetry */
    if (!approx_eq(qf_hypot_fast8(3.0f, 4.0f),
                   qf_hypot_fast8(-3.0f, -4.0f), 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_hypot_fast8(3.0f, 4.0f),
                   qf_hypot_fast8(4.0f, 3.0f), 1e-6f)) return TEST_FAIL;

    /* Zero */
    if (!approx_eq(qf_hypot_fast8(0.0f, 0.0f), 0.0f, 1e-6f)) return TEST_FAIL;

    /* Sweep 0..90 degrees, check error is within 0.2% of true hypot */
    for (int deg = 0; deg <= 90; deg += 3) {
        qf rad = (qf)deg * QF_DEG2RAD_K;
        qf x = cosf(rad) * 1000.0f;
        qf y = sinf(rad) * 1000.0f;
        qf ref = sqrtf(x * x + y * y);
        qf got = qf_hypot_fast8(x, y);
        qf err = QF_ABS(got - ref) / ref;
        if (err > 0.002f) return TEST_FAIL;
    }
    return TEST_PASS;
}

/*=======================================================
 * Test: wave generators
 */

static int test_wave_sqr(void)
{
    if (!approx_eq(qf_wave_sqr(0),       1.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_wave_sqr(0x7FFF),  1.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_wave_sqr(0x8000), -1.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_wave_sqr(0xFFFF), -1.0f, 1e-6f)) return TEST_FAIL;
    return TEST_PASS;
}

static int test_wave_pwm(void)
{
    /* 50% duty = square */
    if (!approx_eq(qf_wave_pwm(0, 0x8000),       1.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_wave_pwm(0x8000, 0x8000), -1.0f, 1e-6f)) return TEST_FAIL;
    /* 0% duty = always low */
    if (!approx_eq(qf_wave_pwm(0, 0), -1.0f, 1e-6f)) return TEST_FAIL;
    return TEST_PASS;
}

static int test_wave_saw(void)
{
    /* Midpoint is 0 */
    if (!approx_eq(qf_wave_saw(0x8000), 0.0f, 0.001f)) return TEST_FAIL;
    /* Endpoints */
    qf v0 = qf_wave_saw(0);
    if (v0 > -0.99f) return TEST_FAIL;
    qf vmax = qf_wave_saw(0xFFFF);
    if (vmax < 0.99f) return TEST_FAIL;
    return TEST_PASS;
}

static int test_wave_tri(void)
{
    /* Peak at 1/4 cycle */
    if (!approx_eq(qf_wave_tri(0x4000), 1.0f, 0.001f)) return TEST_FAIL;
    /* Zero at start and half */
    if (!approx_eq(qf_wave_tri(0), 0.0f, 0.001f)) return TEST_FAIL;
    /* Negative peak at 3/4 cycle */
    if (!approx_eq(qf_wave_tri(0xC000), -1.0f, 0.001f)) return TEST_FAIL;
    return TEST_PASS;
}

static int test_wave_tri_morph(void)
{
    /* Symmetric triangle at break_point = 0x8000 */
    if (!approx_eq(qf_wave_tri_morph(0, 0x8000),      0.0f, 0.001f)) return TEST_FAIL;
    if (!approx_eq(qf_wave_tri_morph(0x8000, 0x8000),  1.0f, 0.001f)) return TEST_FAIL;
    if (!approx_eq(qf_wave_tri_morph(0xFFFF, 0x8000),  0.0f, 0.001f)) return TEST_FAIL;

    /* Break at edge = 0 (degenerate) */
    qf v = qf_wave_tri_morph(0, 0);
    (void)v;  /* just ensure no crash */

    return TEST_PASS;
}

static int test_wave_noise(void)
{
    uint32_t state = 0xACE1u;
    qf sum = 0.0f;
    int n = 10000;

    for (int i = 0; i < n; i++) {
        qf s = qf_wave_noise(&state);
        if (s < -1.0f || s > 1.0f) return TEST_FAIL;
        sum += s;
    }

    /* Mean should be near 0 for white noise */
    qf mean = sum / (qf)n;
    if (QF_ABS(mean) > 0.05f) return TEST_FAIL;

    /* NULL state */
    if (qf_wave_noise(NULL) != 0.0f) return TEST_FAIL;

    return TEST_PASS;
}

/*=======================================================
 * Test: ADSR envelope
 */

static int test_adsr(void)
{
    qf_adsr_t env;

    /* Basic lifecycle */
    qf_adsr_init(&env, 4, 4, 0.5f, 4);
    qf_adsr_trigger(&env);

    /* Attack: 4 steps to reach 1.0 */
    qf v = 0.0f;
    for (int i = 0; i < 4; i++) v = qf_adsr_step(&env);
    if (!approx_eq(v, 1.0f, 0.01f)) return TEST_FAIL;

    /* Decay: 4 steps to reach sustain (0.5) */
    for (int i = 0; i < 4; i++) v = qf_adsr_step(&env);
    if (!approx_eq(v, 0.5f, 0.01f)) return TEST_FAIL;

    /* Sustain: stays at 0.5 */
    for (int i = 0; i < 10; i++) v = qf_adsr_step(&env);
    if (!approx_eq(v, 0.5f, 0.01f)) return TEST_FAIL;

    /* Release: decays to 0 */
    qf_adsr_release(&env);
    for (int i = 0; i < 10; i++) v = qf_adsr_step(&env);
    if (!approx_eq(v, 0.0f, 0.01f)) return TEST_FAIL;
    if (env.state != QF_ADSR_IDLE) return TEST_FAIL;

    /* Idle stays at 0 */
    v = qf_adsr_step(&env);
    if (!approx_eq(v, 0.0f, 1e-6f)) return TEST_FAIL;

    /* Sustain clamping */
    qf_adsr_init(&env, 1, 1, -0.5f, 1);
    if (env.sustain != 0.0f) return TEST_FAIL;
    qf_adsr_init(&env, 1, 1, 1.5f, 1);
    if (env.sustain != 1.0f) return TEST_FAIL;

    /* NULL safety */
    qf_adsr_init(NULL, 1, 1, 0.5f, 1);
    qf_adsr_trigger(NULL);
    qf_adsr_release(NULL);
    if (qf_adsr_step(NULL) != 0.0f) return TEST_FAIL;

    /* Zero-length attack (instant) */
    qf_adsr_init(&env, 0, 0, 0.5f, 0);
    qf_adsr_trigger(&env);
    v = qf_adsr_step(&env);
    if (!approx_eq(v, 1.0f, 0.01f)) return TEST_FAIL;

    return TEST_PASS;
}

/*=======================================================
 * Coverage hooks + tough branches (see QF_MATH_COVERAGE)
 */

static int test_cov_reduce_to_twopi(void)
{
    qf r;

    r = qf_cov_reduce_to_twopi(-QF_PI * 3.5f);
    if (r < 0.0f || r >= QF_TWO_PI) return TEST_FAIL;

    r = qf_cov_reduce_to_twopi(-0.01f);
    if (r < 0.0f || r >= QF_TWO_PI) return TEST_FAIL;

    r = qf_cov_reduce_to_twopi(QF_TWO_PI * 1000.5f);
    if (r < 0.0f || r >= QF_TWO_PI) return TEST_FAIL;

    return TEST_PASS;
}

static int test_trig_table_branches(void)
{
    if (!approx_eq(qf_sin_bam(1), sinf(QF_BAM_TO_RAD(1)), TRIG_TOL)) return TEST_FAIL;

    {
        uint16_t bam = (uint16_t)((1u << 14) | 5000u);
        qf got = qf_tan_bam(bam);
        qf ref = tanf(QF_BAM_TO_RAD(bam));
        if (!approx_eq(got, ref, QF_ABS(ref) * 0.02f + 0.002f)) return TEST_FAIL;
    }

    {
        qf got = qf_tan_bam(16380);
        if (got < 100.0f || got > QF_TAN_MAX) return TEST_FAIL;
    }

    (void)qf_wave_tri(0xA000);

    return TEST_PASS;
}

static int test_tan_pole_and_small_paths(void)
{
    if (!approx_eq(qf_tan(2e-6f), tanf(2e-6f), 1e-5f)) return TEST_FAIL;
    if (!approx_eq(qf_tan(QF_PI + 4e-6f), tanf(QF_PI + 4e-6f), 1e-5f)) return TEST_FAIL;
    if (!approx_eq(qf_tan(QF_TWO_PI - 3e-6f), tanf(QF_TWO_PI - 3e-6f), 1e-5f)) return TEST_FAIL;

    if (qf_tan(QF_HALF_PI - 5e-7f) < 1000.0f) return TEST_FAIL;
    if (qf_tan(4.71238898f - 5e-7f) < 1000.0f) return TEST_FAIL;

    if (!approx_eq(qf_tan(0.42f), tanf(0.42f), TRIG_TOL)) return TEST_FAIL;

    return TEST_PASS;
}

static int test_acos_special_bins(void)
{
    if (!approx_eq(qf_acos(0.99992f), acosf(0.99992f), ATRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_acos(-0.99992f), acosf(-0.99992f), ATRIG_TOL)) return TEST_FAIL;
    if (!approx_eq(qf_acos(0.993f), acosf(0.993f), ATRIG_TOL * 3)) return TEST_FAIL;

    return TEST_PASS;
}

static int test_pow2_make_pow2i_extremes(void)
{
    volatile qf a = qf_pow2(140.0f);
    volatile qf b = qf_pow2(-140.0f);
    (void)a;
    (void)b;
    if (a < 3e38f) return TEST_FAIL;
    if (b != 0.0f) return TEST_FAIL;

    if (!approx_eq(qf_pow2(-3.7f), powf(2.0f, -3.7f), EXP_TOL * 2)) return TEST_FAIL;

    /* NaN → NaN */
    {
        union { float f; uint32_t u; } nan_u;
        nan_u.u = 0x7FC00000u;  /* quiet NaN */
        volatile qf nan_r = qf_pow2(nan_u.f);
        (void)nan_r;
        /* pow2(NaN) should return NaN (unchanged) */
    }
    /* +Inf → +Inf, -Inf → 0 */
    {
        union { float f; uint32_t u; } inf_u;
        inf_u.u = 0x7F800000u;
        volatile qf pinf_r = qf_pow2(inf_u.f);
        (void)pinf_r;
        inf_u.u = 0xFF800000u;
        volatile qf ninf_r = qf_pow2(inf_u.f);
        if (ninf_r != 0.0f) return TEST_FAIL;
    }
    /* Large |x| that triggers fallback path (magic trick fails for |x| > ~2^22) */
    {
        volatile qf big = qf_pow2(4194305.0f);
        (void)big; /* result overflows; just exercising the path */
    }

    return TEST_PASS;
}

static int test_log2_pow2_frac_edge(void)
{
    union {
        float f;
        uint32_t u;
    } v;
    for (v.u = 0x3f800001u; v.u < 0x40000000u; v.u += 0x80000u) {
        volatile qf lg = qf_log2(v.f);
        volatile qf p2 = qf_pow2(qf_log2(v.f));
        (void)lg;
        (void)p2;
    }

    /* Subnormal input: exponent field = 0, exercises legacy log2 path */
    {
        v.u = 0x00000001u;  /* smallest positive subnormal (~1.4e-45) */
        qf lg = qf_log2(v.f);
        if (lg >= 0.0f) return TEST_FAIL;  /* log2 of tiny value must be very negative */
    }

    return TEST_PASS;
}

static int test_atan2_small_ratio_paths(void)
{
    if (!approx_eq(qf_atan2(3e-5f, 1.0f), atan2f(3e-5f, 1.0f), SWEEP_ATAN2_ABS)) return TEST_FAIL;
    if (!approx_eq(qf_atan2(1.0f, 3e-5f), atan2f(1.0f, 3e-5f), SWEEP_ATAN2_ABS)) return TEST_FAIL;
    return TEST_PASS;
}

static int test_cov_internal_helpers(void)
{
    /* make_pow2i normal path (n in [-126, 127]) */
    if (!approx_eq(qf_cov_make_pow2i(0), 1.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_cov_make_pow2i(1), 2.0f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_cov_make_pow2i(-1), 0.5f, 1e-6f)) return TEST_FAIL;
    if (!approx_eq(qf_cov_make_pow2i(10), 1024.0f, 1e-3f)) return TEST_FAIL;

    /* qf_ifloor */
    if (qf_cov_ifloor(2.7f) != 2) return TEST_FAIL;
    if (qf_cov_ifloor(-2.7f) != -3) return TEST_FAIL;
    if (qf_cov_ifloor(0.0f) != 0) return TEST_FAIL;
    if (qf_cov_ifloor(-0.1f) != -1) return TEST_FAIL;

    /* qf_exp2_frac_01: frac <= 0.5 branch */
    {
        qf r = qf_cov_exp2_frac_01(0.3f);
        if (!approx_eq(r, powf(2.0f, 0.3f), EXP_TOL)) return TEST_FAIL;
    }
    /* qf_exp2_frac_01: frac > 0.5 branch */
    {
        qf r = qf_cov_exp2_frac_01(0.7f);
        if (!approx_eq(r, powf(2.0f, 0.7f), EXP_TOL)) return TEST_FAIL;
    }

    /* asin_pos_kernel: ax >= 1.0 → HALF_PI (defensive) */
    if (!approx_eq(qf_cov_asin_pos_kernel(1.0f), QF_HALF_PI, 1e-6f)) return TEST_FAIL;

    /* asin_pos_kernel: small ax → ax (linear approx) */
    if (!approx_eq(qf_cov_asin_pos_kernel(0.01f), 0.01f, 1e-5f)) return TEST_FAIL;

    return TEST_PASS;
}

static int test_accuracy_double_near_zero(void)
{
    for (int i = 1; i <= 400; i++) {
        double xd = (double)i * 1e-12;
        float xf = (float)xd;
        if (!approx_vs_double_abs(qf_sin(xf), sin(xd), SWEEP_SINCOS_ABS)) return TEST_FAIL;
        if (!approx_vs_double_abs(qf_cos(xf), cos(xd), SWEEP_SINCOS_ABS)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_accuracy_double_near_pi(void)
{
    const double pi = 3.14159265358979323846;

    for (int i = -200; i <= 200; i++) {
        double d = (double)i * 1e-7;
        double near_pi = pi + d;
        float xf = (float)near_pi;
        if (!approx_vs_double_abs(qf_sin(xf), sin(near_pi), SWEEP_SINCOS_ABS)) return TEST_FAIL;
        if (!approx_vs_double_abs(qf_cos(xf), cos(near_pi), SWEEP_SINCOS_ABS)) return TEST_FAIL;

        double near_2pi = 2.0 * pi + d;
        xf = (float)near_2pi;
        if (!approx_vs_double_abs(qf_sin(xf), sin(near_2pi), SWEEP_SINCOS_ABS)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_accuracy_double_cos_near_quadrant(void)
{
    const double pi = 3.14159265358979323846;

    for (int i = -250; i <= 250; i++) {
        double d = (double)i * 1e-7;
        double x = (pi / 2.0) + d;
        float xf = (float)x;
        if (!approx_vs_double_abs(qf_cos(xf), cos(x), SWEEP_SINCOS_ABS)) return TEST_FAIL;

        x = 3.0 * (pi / 2.0) + d;
        xf = (float)x;
        if (!approx_vs_double_abs(qf_cos(xf), cos(x), SWEEP_SINCOS_ABS)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_accuracy_double_tan(void)
{
    const double pi = 3.14159265358979323846;

    /* Same float angle as qf_tan: host tanf is the stable oracle here (double tan on the
     * rounded float differs noticeably on steep slopes while still testing vs libm). */
    for (int t = 250; t < 8600; t++) {
        double deg = (double)t / 100.0;
        double rd = deg * (pi / 180.0);
        float rf = (float)rd;
        double ref = (double)tanf(rf);
        if (fabs(ref) > 120.0) continue;
        double got = (double)qf_tan(rf);
        double mag = fabs(ref);
        if (mag > 1.0) {
            if (!approx_vs_double_rel((float)got, ref, 3e-3)) return TEST_FAIL;
        } else {
            if (!approx_vs_double_abs((float)got, ref, SWEEP_TAN_ABS)) return TEST_FAIL;
        }
    }
    return TEST_PASS;
}

static int test_accuracy_double_log_family(void)
{
    /* Stay within float meaningful magnitude (avoid Inf / sterile comparisons). */
    for (double x = 1e-37; x < 1e34; x *= 1.7) {
        float xf = (float)x;
        if (!(xf > 0.0f) || xf != xf || xf > 3.4e38f) continue;
        if (!approx_vs_double_rel(qf_ln(xf), log((double)xf), SWEEP_LOG_REL)) return TEST_FAIL;
        if (!approx_vs_double_rel(qf_log10(xf), log10((double)xf), SWEEP_LOG_REL)) return TEST_FAIL;
        if (!approx_vs_double_rel(qf_log2(xf), log2((double)xf), SWEEP_LOG_REL)) return TEST_FAIL;
    }
    /* ln(x) ~ (x-1) here — use absolute tolerance vs double reference */
    for (int i = -80; i <= 80; i++) {
        double x = 1.0 + (double)i * 2e-7;
        float xf = (float)x;
        if (!approx_vs_double_abs(qf_ln(xf), log((double)xf), 3e-6)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_accuracy_double_exp_family(void)
{
    for (int i = -420; i <= 420; i++) {
        double y = (double)i * 0.1;
        float yf = (float)y;
        if (!approx_vs_double_rel(qf_exp(yf), exp((double)yf), SWEEP_EXP_REL)) return TEST_FAIL;
        if (!approx_vs_double_rel(qf_pow2(yf), pow(2.0, (double)yf), SWEEP_EXP_REL)) return TEST_FAIL;
    }
    return TEST_PASS;
}

static int test_accuracy_double_sqrt(void)
{
    for (int i = 1; i <= 5000; i++) {
        double x = (double)i * 1e-9;
        float xf = (float)x;
        if (!approx_vs_double_rel(qf_sqrt(xf), sqrt((double)xf), SWEEP_SQRT_REL)) return TEST_FAIL;
    }
    for (int i = 1; i <= 2000; i++) {
        double x = (double)i * 0.031;
        float xf = (float)x;
        if (!approx_vs_double_rel(qf_sqrt(xf), sqrt((double)xf), SWEEP_SQRT_REL)) return TEST_FAIL;
    }
    return TEST_PASS;
}

/*=======================================================
 * Test: version string
 */

static int test_version(void)
{
    const char *v = QF_MATH_VERSION;
    if (v[0] == '\0') return TEST_FAIL;
    if (QF_MATH_VERSION_HEX == 0) return TEST_FAIL;
    return TEST_PASS;
}

/*=======================================================
 * Test: domain errors
 */

static int test_domain_errors(void)
{
    if (qf_sqrt(-1.0f) != QF_DOMAIN_ERROR) return TEST_FAIL;
    if (qf_log2(0.0f)  != QF_DOMAIN_ERROR) return TEST_FAIL;
    if (qf_log2(-5.0f) != QF_DOMAIN_ERROR) return TEST_FAIL;
    if (qf_ln(0.0f)    != QF_DOMAIN_ERROR) return TEST_FAIL;
    if (qf_log10(-1.0f) != QF_DOMAIN_ERROR) return TEST_FAIL;
    return TEST_PASS;
}

/*=======================================================
 * Main
 */

int main(void)
{
    printf("qf_math test suite — quick float math\n");
    printf("================================================\n\n");

    printf("Macros & Constants:\n");
    RUN_TEST(test_macros);
    RUN_TEST(test_version);
    printf("\n");

    printf("Conversions:\n");
    RUN_TEST(test_conversions);
    RUN_TEST(test_angular_conversions);
    printf("\n");

    printf("Trig:\n");
    RUN_TEST(test_sin);
    RUN_TEST(test_cos);
    RUN_TEST(test_tan);
    RUN_TEST(test_trig_identity);
    RUN_TEST(test_large_angles);
    RUN_TEST(test_cov_reduce_to_twopi);
    RUN_TEST(test_trig_table_branches);
    RUN_TEST(test_tan_pole_and_small_paths);
    printf("\n");

    printf("Accuracy vs libm (double reference):\n");
    RUN_TEST(test_accuracy_double_near_zero);
    RUN_TEST(test_accuracy_double_near_pi);
    RUN_TEST(test_accuracy_double_cos_near_quadrant);
    RUN_TEST(test_accuracy_double_tan);
    RUN_TEST(test_accuracy_double_log_family);
    RUN_TEST(test_accuracy_double_exp_family);
    RUN_TEST(test_accuracy_double_sqrt);
    printf("\n");

    printf("Inverse Trig:\n");
    RUN_TEST(test_acos);
    RUN_TEST(test_asin);
    RUN_TEST(test_atan);
    RUN_TEST(test_atan2);
    RUN_TEST(test_acos_special_bins);
    RUN_TEST(test_atan2_small_ratio_paths);
    RUN_TEST(test_cov_internal_helpers);
    printf("\n");

    printf("Log / Exp:\n");
    RUN_TEST(test_log2);
    RUN_TEST(test_ln);
    RUN_TEST(test_log10);
    RUN_TEST(test_pow2);
    RUN_TEST(test_pow2_make_pow2i_extremes);
    RUN_TEST(test_log2_pow2_frac_edge);
    RUN_TEST(test_exp);
    RUN_TEST(test_pow10);
    RUN_TEST(test_pow);
    printf("\n");

    printf("Sqrt / Hypot:\n");
    RUN_TEST(test_sqrt);
    RUN_TEST(test_hypot);
    RUN_TEST(test_hypot_fast2);
    RUN_TEST(test_hypot_fast8);
    printf("\n");

    printf("Waves:\n");
    RUN_TEST(test_wave_sqr);
    RUN_TEST(test_wave_pwm);
    RUN_TEST(test_wave_saw);
    RUN_TEST(test_wave_tri);
    RUN_TEST(test_wave_tri_morph);
    RUN_TEST(test_wave_noise);
    printf("\n");

    printf("ADSR:\n");
    RUN_TEST(test_adsr);
    printf("\n");

    printf("Domain Errors:\n");
    RUN_TEST(test_domain_errors);
    printf("\n");

    printf("================================================\n");
    printf("Results: %d/%d passed", g_test_count - g_fail_count, g_test_count);
    if (g_fail_count > 0)
        printf("  (%d FAILED)", g_fail_count);
    printf("\n");

    return g_fail_count;
}
