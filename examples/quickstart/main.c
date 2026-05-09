/**
 *  qf_math quick start example
 *
 *  Demonstrates basic usage of the qf_math library: trig, log/exp,
 *  sqrt/hypot, degree/BAM conversions, and waveform generation.
 *
 *  Build:   make
 *  Run:     ./quickstart
 */

#include <stdio.h>
#include "qf_math.h"

/* ── helper: print a labeled result ─────────────────────────────── */

static void show(const char *label, qf value)
{
    printf("  %-28s = %12.6f\n", label, (double)value);
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void)
{
    printf("qf_math quick start  (version %s)\n\n", QF_MATH_VERSION);

    /* --- Trigonometry (radians) --- */
    printf("Trig (radians):\n");
    show("qf_sin(1.0)",          qf_sin(1.0f));
    show("qf_cos(1.0)",          qf_cos(1.0f));
    show("qf_tan(0.5)",          qf_tan(0.5f));
    show("qf_atan2(3.0, 4.0)",   qf_atan2(3.0f, 4.0f));

    /* --- Trig (degrees) --- */
    printf("\nTrig (degrees):\n");
    show("qf_sin_deg(45)",       qf_sin_deg(45.0f));
    show("qf_cos_deg(60)",       qf_cos_deg(60.0f));

    /* --- Trig (BAM — Binary Angular Measure) --- */
    printf("\nTrig (BAM, 16384 = 90 deg):\n");
    show("qf_sin_bam(16384)",    qf_sin_bam(16384));   /* sin(90 deg) */
    show("qf_cos_bam(0)",        qf_cos_bam(0));       /* cos(0 deg)  */

    /* --- Inverse trig --- */
    printf("\nInverse trig:\n");
    show("qf_asin(0.5)",         qf_asin(0.5f));
    show("qf_acos(0.5)",         qf_acos(0.5f));
    show("qf_atan(1.0)",         qf_atan(1.0f));

    /* --- Logarithms and exponentials --- */
    printf("\nLog / exp:\n");
    show("qf_log2(8.0)",         qf_log2(8.0f));
    show("qf_ln(2.71828)",       qf_ln(2.71828f));
    show("qf_log10(1000.0)",     qf_log10(1000.0f));
    show("qf_pow2(3.0)",         qf_pow2(3.0f));
    show("qf_exp(1.0)",          qf_exp(1.0f));
    show("qf_pow(2.0, 10.0)",    qf_pow(2.0f, 10.0f));

    /* --- Sqrt and hypot --- */
    printf("\nSqrt / hypot:\n");
    show("qf_sqrt(2.0)",         qf_sqrt(2.0f));
    show("qf_hypot(3.0, 4.0)",   qf_hypot(3.0f, 4.0f));
    show("qf_hypot_fast8(3, 4)", qf_hypot_fast8(3.0f, 4.0f));

    /* --- Utility macros --- */
    printf("\nUtility macros:\n");
    show("QF_DEG_TO_RAD(180)",    QF_DEG_TO_RAD(180.0f));
    show("QF_RAD_TO_DEG(QF_PI)",  QF_RAD_TO_DEG(QF_PI));
    show("QF_CLAMP(5, 0, 3)",    QF_CLAMP(5.0f, 0.0f, 3.0f));
    show("QF_INTERP(0, 10, 0.25)", QF_INTERP(0.0f, 10.0f, 0.25f));

    /* --- Waveform generators --- */
    printf("\nWaveforms (phase 0-65535 = one cycle):\n");
    show("qf_wave_sqr(8192)",    qf_wave_sqr(8192));
    show("qf_wave_tri(16384)",   qf_wave_tri(16384));
    show("qf_wave_saw(32768)",   qf_wave_saw(32768));

    uint32_t noise_state = 12345;
    show("qf_wave_noise()",      qf_wave_noise(&noise_state));

    /* --- Fixed-radix bridge --- */
    printf("\nFixed-radix bridge (Q16.16):\n");
    int32_t fr_val = QF_TO_FR(1.234f, 16);
    printf("  QF_TO_FR(1.234, 16)          = %d\n", (int)fr_val);
    show("FR_TO_QF(80871, 16)",  FR_TO_QF(80871, 16));

    printf("\ndone.\n");
    return 0;
}
