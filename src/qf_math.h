/**
 *  @file qf_math.h - QF_MATH: Quick Float Math Library
 *
 *  Fast approximate math on IEEE 754 float32. Table-based trig, log, exp,
 *  sqrt, hypot, wave generators, and ADSR — all in ~5 KB of portable C99.
 *  No external dependencies (no libm required).
 *
 *  @author M A Chatterjee <deftio [at] deftio [dot] com>
 *
 *  Copyright (c) 2002-2026, M. A. Chatterjee
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __QF_MATH_H__
#define __QF_MATH_H__

#define QF_MATH_VERSION     "1.0.0"
#define QF_MATH_VERSION_HEX  0x010000

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef float qf;

/*===============================================
 * Conversion between qf and fixed-radix integers
 *
 * These macros bridge between the FR_math integer fixed-radix world
 * and float32. Use them at system boundaries (sensor input, DAC output,
 * protocol fields, etc.).
 *
 * QF_TO_FR(val, radix) — float to fixed-radix s32, truncating
 * QF_TO_FR_RND(val, radix) — float to fixed-radix s32, round-to-nearest
 * FR_TO_QF(val, radix) — fixed-radix s32 to float
 *
 * Example:
 *   qf angle = 1.234f;
 *   int32_t angle_r16 = QF_TO_FR(angle, 16);  // 80871
 *   qf back = FR_TO_QF(angle_r16, 16);        // ~1.234
 */
#define QF_TO_FR(val, radix)     ((int32_t)((val) * (qf)(1 << (radix))))
#define QF_TO_FR_RND(val, radix) ((int32_t)((val) * (qf)(1 << (radix)) + ((val) >= 0.0f ? 0.5f : -0.5f)))
#define FR_TO_QF(val, radix)     ((qf)(val) / (qf)(1 << (radix)))

/* Integer conversions */
#define QF_FROM_INT(x)  ((qf)(x))
#define QF_TO_INT(x)    ((int32_t)(x))
#define QF_ROUND(x)     ((int32_t)((x) >= 0.0f ? (x) + 0.5f : (x) - 0.5f))

/*===============================================
 * Basic operations
 */
#define QF_ABS(x)            (((x) < 0.0f) ? (-(x)) : (x))
#define QF_SGN(x)            (((x) > 0.0f) ? 1.0f : (((x) < 0.0f) ? -1.0f : 0.0f))
#define QF_MIN(a, b)         (((a) < (b)) ? (a) : (b))
#define QF_MAX(a, b)         (((a) > (b)) ? (a) : (b))
#define QF_CLAMP(x, lo, hi)  (QF_MIN(QF_MAX((x), (lo)), (hi)))

/*===============================================
 * Interpolation
 *
 * QF_INTERP(a, b, t) — linear interpolation, t in [0, 1]
 */
#define QF_INTERP(a, b, t)  ((a) + ((b) - (a)) * (t))

/*===============================================
 * Constants
 */
#define QF_PI          3.14159265f
#define QF_TWO_PI      6.28318530f
#define QF_HALF_PI     1.57079632f
#define QF_E           2.71828182f
#define QF_INV_E       0.36787944f
#define QF_INV_PI      0.31830988f
#define QF_DEG2RAD_K   0.01745329f
#define QF_RAD2DEG_K   57.2957795f
#define QF_LOG2E       1.44269504f
#define QF_LN2         0.69314718f
#define QF_LOG2_10     3.32192809f
#define QF_LOG10_2     0.30102999f
#define QF_SQRT2       1.41421356f
#define QF_INV_SQRT2   0.70710678f
#define QF_SQRT3       1.73205080f
#define QF_INV_SQRT3   0.57735026f
#define QF_SQRT5       2.23606797f
#define QF_SQRT10      3.16227766f

/*===============================================
 * Angular conversions
 */
#define QF_DEG_TO_RAD(x)  ((x) * QF_DEG2RAD_K)
#define QF_RAD_TO_DEG(x)  ((x) * QF_RAD2DEG_K)

/*===============================================
 * BAM (Binary Angular Measure) conversions
 *
 * BAM is a u16 where one full revolution = 65536.
 *   0     =   0 deg =     0 rad
 *   16384 =  90 deg = pi/2 rad
 *   32768 = 180 deg =   pi rad
 *   65536 wraps to 0 (u16 natural wraparound)
 */
#define QF_BAM_TO_RAD(bam)  ((qf)(bam) * (QF_TWO_PI / 65536.0f))
#define QF_RAD_TO_BAM(rad)  ((uint16_t)((rad) * (65536.0f / QF_TWO_PI)))
#define QF_BAM_TO_DEG(bam)  ((qf)(bam) * (360.0f / 65536.0f))
#define QF_DEG_TO_BAM(deg)  ((uint16_t)((deg) * (65536.0f / 360.0f)))

/*===============================================
 * Domain error sentinel
 */
#define QF_DOMAIN_ERROR (-1.0e30f)

/*===============================================
 * Trig — table-based fast approximations
 *
 * Uses full-cycle 512-entry sine and tangent tables with 7-bit BAM
 * sub-step interpolation. Radian and degree APIs convert to BAM units
 * with one float multiply before table lookup.
 *
 * sin/cos: input in radians, output in [-1.0, 1.0]
 * tan: input in radians, output saturated at +/- QF_TAN_MAX near poles
 *
 * Worst-case error: ~3e-5 (same as FR_math at s15.16).
 */
#define QF_TAN_MAX  32767.0f

qf qf_sin_bam(uint16_t bam);
qf qf_cos_bam(uint16_t bam);
qf qf_tan_bam(uint16_t bam);

qf qf_sin(qf rad);
qf qf_cos(qf rad);
qf qf_tan(qf rad);

qf qf_sin_deg(qf deg);
qf qf_cos_deg(qf deg);
qf qf_tan_deg(qf deg);

/*===============================================
 * Inverse trig
 *
 * acos: output in [0, pi]
 * asin: output in [-pi/2, pi/2]
 * atan: output in [-pi/2, pi/2]
 * atan2: output in [-pi, pi]
 *
 * Uses binary search on the sine quadrant table (same approach
 * as FR_acos / FR_asin / FR_atan2).
 */
qf qf_acos(qf x);
qf qf_asin(qf x);
qf qf_atan(qf x);
qf qf_atan2(qf y, qf x);

/*===============================================
 * Logarithms — table-based fast approximations
 *
 * Uses a 65-entry table for log2 mantissa lookup (same table as
 * FR_log2). Returns QF_DOMAIN_ERROR for x <= 0.
 *
 * ln and log10 are derived from log2 via constant multiplication.
 */
qf qf_log2(qf x);
qf qf_ln(qf x);
qf qf_log10(qf x);

/*===============================================
 * Exponentials — table-based fast approximations
 *
 * Uses a 65-entry table for 2^frac lookup (same table as FR_pow2).
 * exp and pow10 are derived from pow2 via base conversion.
 */
qf qf_pow2(qf x);
qf qf_exp(qf x);
qf qf_pow10(qf x);

/*===============================================
 * Square root and magnitude
 *
 * qf_sqrt: Newton-Raphson with IEEE 754 initial estimate.
 *           Returns QF_DOMAIN_ERROR for x < 0.
 * qf_hypot: sqrt(x*x + y*y) via qf_sqrt.
 * qf_hypot_fast8: 8-segment piecewise-linear magnitude approximation
 *           (~0.10% peak error, no division, same algorithm as
 *           FR_hypot_fast8). Based on US Patent 6,567,777 B1
 *           (Chatterjee, expired).
 */
qf qf_sqrt(qf x);
qf qf_hypot(qf x, qf y);
qf qf_hypot_fast8(qf x, qf y);

/*===============================================
 * Wave generators
 *
 * All take a u16 BAM phase in [0, 65535] (one full cycle) and
 * return qf in [-1.0, 1.0] (except tri_morph: [0.0, 1.0]).
 *
 * Use QF_HZ2BAM_INC to compute a per-sample phase increment:
 *   uint16_t phase = 0;
 *   uint16_t inc = QF_HZ2BAM_INC(440, 48000);
 *   for (...) { sample = qf_sin_bam(phase); phase += inc; }
 */
#define QF_HZ2BAM_INC(hz, sample_rate) \
    ((uint16_t)(((uint32_t)(hz) * 65536UL) / (uint32_t)(sample_rate)))

qf qf_wave_sqr(uint16_t phase);
qf qf_wave_pwm(uint16_t phase, uint16_t duty);
qf qf_wave_tri(uint16_t phase);
qf qf_wave_saw(uint16_t phase);
qf qf_wave_tri_morph(uint16_t phase, uint16_t break_point);
qf qf_wave_noise(uint32_t *state);

/*===============================================
 * ADSR envelope generator
 *
 * Same lifecycle as fr_adsr_t but all levels are qf in [0.0, 1.0].
 *
 * Lifecycle:
 *   qf_adsr_t env;
 *   qf_adsr_init(&env, atk_samples, dec_samples, 0.7f, rel_samples);
 *   qf_adsr_trigger(&env);
 *   for (...) sample = qf_adsr_step(&env);
 *   qf_adsr_release(&env);
 *   for (...) sample = qf_adsr_step(&env);
 */
#define QF_ADSR_IDLE    0
#define QF_ADSR_ATTACK  1
#define QF_ADSR_DECAY   2
#define QF_ADSR_SUSTAIN 3
#define QF_ADSR_RELEASE 4

typedef struct qf_adsr_s {
    uint8_t state;
    qf     level;
    qf     sustain;
    qf     attack_inc;
    qf     decay_dec;
    qf     release_dec;
} qf_adsr_t;

void qf_adsr_init(qf_adsr_t *env,
                   uint32_t attack_samples,
                   uint32_t decay_samples,
                   qf sustain_level,
                   uint32_t release_samples);
void qf_adsr_trigger(qf_adsr_t *env);
void qf_adsr_release(qf_adsr_t *env);
qf  qf_adsr_step(qf_adsr_t *env);

/* Built only when compiling tests/coverage (see Makefile `CFLAGS_TEST`). */
#if defined(QF_MATH_COVERAGE)
qf qf_cov_reduce_to_twopi(qf rad);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __QF_MATH_H__ */
