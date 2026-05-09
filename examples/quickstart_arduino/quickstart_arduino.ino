/**
 *  qf_math quick start — Arduino sketch
 *
 *  Demonstrates basic usage of the qf_math library: trig, log/exp,
 *  sqrt/hypot, degree/BAM conversions, and waveform generation.
 *
 *  Open the Serial Monitor at 115200 baud to see results.
 */

#include <qf_math.h>

static void show(const char *label, qf value)
{
    Serial.print("  ");
    Serial.print(label);
    /* pad to 30 chars */
    int pad = 30 - (int)strlen(label);
    while (pad-- > 0) Serial.print(' ');
    Serial.print("= ");
    Serial.println(value, 6);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { /* wait for USB-serial on boards that need it */ }

    Serial.print("qf_math quick start  (version ");
    Serial.print(QF_MATH_VERSION);
    Serial.println(")\n");

    /* Trig (radians) */
    Serial.println("Trig (radians):");
    show("qf_sin(1.0)",          qf_sin(1.0f));
    show("qf_cos(1.0)",          qf_cos(1.0f));
    show("qf_tan(0.5)",          qf_tan(0.5f));
    show("qf_atan2(3.0, 4.0)",   qf_atan2(3.0f, 4.0f));

    /* Trig (degrees) */
    Serial.println("\nTrig (degrees):");
    show("qf_sin_deg(45)",       qf_sin_deg(45.0f));
    show("qf_cos_deg(60)",       qf_cos_deg(60.0f));

    /* Trig (BAM) */
    Serial.println("\nTrig (BAM, 16384 = 90 deg):");
    show("qf_sin_bam(16384)",    qf_sin_bam(16384));
    show("qf_cos_bam(0)",        qf_cos_bam(0));

    /* Inverse trig */
    Serial.println("\nInverse trig:");
    show("qf_asin(0.5)",         qf_asin(0.5f));
    show("qf_acos(0.5)",         qf_acos(0.5f));
    show("qf_atan(1.0)",         qf_atan(1.0f));

    /* Log / exp */
    Serial.println("\nLog / exp:");
    show("qf_log2(8.0)",         qf_log2(8.0f));
    show("qf_ln(2.71828)",       qf_ln(2.71828f));
    show("qf_log10(1000.0)",     qf_log10(1000.0f));
    show("qf_pow2(3.0)",         qf_pow2(3.0f));
    show("qf_exp(1.0)",          qf_exp(1.0f));
    show("qf_pow(2.0, 10.0)",    qf_pow(2.0f, 10.0f));

    /* Sqrt / hypot */
    Serial.println("\nSqrt / hypot:");
    show("qf_sqrt(2.0)",         qf_sqrt(2.0f));
    show("qf_hypot(3.0, 4.0)",   qf_hypot(3.0f, 4.0f));
    show("qf_hypot_fast8(3, 4)", qf_hypot_fast8(3.0f, 4.0f));

    /* Utility macros */
    Serial.println("\nUtility macros:");
    show("QF_DEG_TO_RAD(180)",    QF_DEG_TO_RAD(180.0f));
    show("QF_RAD_TO_DEG(QF_PI)",  QF_RAD_TO_DEG(QF_PI));
    show("QF_CLAMP(5, 0, 3)",    QF_CLAMP(5.0f, 0.0f, 3.0f));
    show("QF_INTERP(0, 10, 0.25)", QF_INTERP(0.0f, 10.0f, 0.25f));

    /* Waveforms */
    Serial.println("\nWaveforms (phase 0-65535 = one cycle):");
    show("qf_wave_sqr(8192)",    qf_wave_sqr(8192));
    show("qf_wave_tri(16384)",   qf_wave_tri(16384));
    show("qf_wave_saw(32768)",   qf_wave_saw(32768));

    uint32_t noise_state = 12345;
    show("qf_wave_noise()",      qf_wave_noise(&noise_state));

    /* Fixed-radix bridge */
    Serial.println("\nFixed-radix bridge (Q16.16):");
    int32_t fr_val = QF_TO_FR(1.234f, 16);
    Serial.print("  QF_TO_FR(1.234, 16)          = ");
    Serial.println(fr_val);
    show("FR_TO_QF(80871, 16)",  FR_TO_QF(80871, 16));

    Serial.println("\ndone.");
}

void loop()
{
    /* nothing — one-shot demo */
}
