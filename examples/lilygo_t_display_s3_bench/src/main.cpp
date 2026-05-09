/**
 * LilyGO T-Display-S3 — same compare kernels as compare/benchmark_core.c (Arduino / PlatformIO).
 */

#include <Arduino.h>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>

extern "C" {
#include "benchmark_core.h"
#include "esp_chip_info.h"
#include "esp_timer.h"
#include "qf_math.h"
}

#include <FastTrig.h>
#include "espp_fast_math_subset.hpp"

static void esp_bench_init(void *ctx)
{
    (void)ctx;
}

static uint64_t esp_bench_now(void *ctx)
{
    (void)ctx;
    return (uint64_t)esp_timer_get_time();
}

static double esp_bench_elapsed_us(void *ctx, uint64_t start, uint64_t end)
{
    (void)ctx;
    return (double)((int64_t)(end - start));
}

static bench_timer_t s_esp_timer = {
    .init       = esp_bench_init,
    .now        = esp_bench_now,
    .elapsed_us = esp_bench_elapsed_us,
    .ctx        = nullptr,
};

static float wrap_rad_0_2pi(float rad)
{
    if (rad >= 0.0f) {
        if (rad >= QF_TWO_PI) {
            rad -= (float)((int32_t)(rad * (1.0f / QF_TWO_PI))) * QF_TWO_PI;
            if (rad >= QF_TWO_PI)
                rad -= QF_TWO_PI;
        }
    } else {
        rad += (float)((int32_t)(-rad * (1.0f / QF_TWO_PI)) + 1) * QF_TWO_PI;
        if (rad >= QF_TWO_PI)
            rad -= QF_TWO_PI;
    }
    return rad;
}

static float fasttrig_sin_f(float rad) { return isin(rad * QF_RAD2DEG_K); }
static float fasttrig_sin_deg_f(float deg) { return isin(deg); }
static float fasttrig_sin_bam_f(float bam) { return isin(QF_BAM_TO_DEG((uint16_t)((int32_t)bam))); }
static float fasttrig_cos_f(float rad) { return icos(rad * QF_RAD2DEG_K); }
static float fasttrig_cos_deg_f(float deg) { return icos(deg); }
static float fasttrig_cos_bam_f(float bam) { return icos(QF_BAM_TO_DEG((uint16_t)((int32_t)bam))); }
static float fasttrig_tan_f(float rad) { return itan(rad * QF_RAD2DEG_K); }
static float fasttrig_tan_deg_f(float deg) { return itan(deg); }
static float fasttrig_tan_bam_f(float bam) { return itan(QF_BAM_TO_DEG((uint16_t)((int32_t)bam))); }
static float fasttrig_asin_f(float x) { return iasin(x) * QF_DEG2RAD_K; }
static float fasttrig_acos_f(float x) { return iacos(x) * QF_DEG2RAD_K; }
static float fasttrig_atan_f(float x) { return atanFast(x); }
static float fasttrig_atan2_f(float y, float x) { return atan2Fast(y, x); }

static float espdsp_sqrt_f(float x)
{
    /* ESP-DSP dsps_sqrtf_f32_ansi approximation, kept local to avoid building the full examples tree. */
    int32_t bits;
    std::memcpy(&bits, &x, sizeof(bits));
    bits = 0x1fbb4000 + (bits >> 1);

    float out;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

static float espp_sin_f(float rad) { return espp::fast_sin(wrap_rad_0_2pi(rad)); }
static float espp_sin_deg_f(float deg) { return espp_sin_f(QF_DEG_TO_RAD(deg)); }
static float espp_sin_bam_f(float bam) { return espp_sin_f(QF_BAM_TO_RAD((uint16_t)((int32_t)bam))); }
static float espp_cos_f(float rad) { return espp::fast_cos(wrap_rad_0_2pi(rad)); }
static float espp_cos_deg_f(float deg) { return espp_cos_f(QF_DEG_TO_RAD(deg)); }
static float espp_cos_bam_f(float bam) { return espp_cos_f(QF_BAM_TO_RAD((uint16_t)((int32_t)bam))); }
static float espp_sqrt_f(float x) { return x * espp::fast_inv_sqrt(x); }
static float espp_ln_f(float x) { return espp::fast_ln(x); }

void setup(void)
{
    Serial.begin(115200);
    delay(800);

    static volatile float sink;
    bench_results_t       res;

    esp_chip_info_t chip;
    esp_chip_info(&chip);

    Serial.printf("\nqf_math bench — LilyGO T-Display-S3 (model=%u cores=%u rev=%u)\n\n",
                  (unsigned)chip.model, (unsigned)chip.cores, (unsigned)chip.revision);

    bench_run_all(&s_esp_timer, &sink, &res);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_SIN, fasttrig_sin_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_SIN_DEG, fasttrig_sin_deg_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_SIN_BAM, fasttrig_sin_bam_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_COS, fasttrig_cos_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_COS_DEG, fasttrig_cos_deg_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_COS_BAM, fasttrig_cos_bam_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_TAN, fasttrig_tan_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_TAN_DEG, fasttrig_tan_deg_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_TAN_BAM, fasttrig_tan_bam_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_ASIN, fasttrig_asin_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_ACOS, fasttrig_acos_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_ATAN, fasttrig_atan_f);
    bench_run_binary_peer(&s_esp_timer, &sink, &res, BENCH_L_FASTTRIG, BENCH_F_ATAN2, fasttrig_atan2_f);

    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_ESPDSP, BENCH_F_SQRT, espdsp_sqrt_f);

    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_ESPP, BENCH_F_SIN, espp_sin_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_ESPP, BENCH_F_SIN_DEG, espp_sin_deg_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_ESPP, BENCH_F_SIN_BAM, espp_sin_bam_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_ESPP, BENCH_F_COS, espp_cos_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_ESPP, BENCH_F_COS_DEG, espp_cos_deg_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_ESPP, BENCH_F_COS_BAM, espp_cos_bam_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_ESPP, BENCH_F_SQRT, espp_sqrt_f);
    bench_run_unary_peer(&s_esp_timer, &sink, &res, BENCH_L_ESPP, BENCH_F_LN, espp_ln_f);
    bench_print_human(&res);

    char device_line[160];
    snprintf(device_line, sizeof device_line,
             "LilyGO T-Display-S3 · Arduino-ESP32 · esp_chip model=%u cores=%u revision=%u",
             (unsigned)chip.model, (unsigned)chip.cores, (unsigned)chip.revision);

    Serial.println("\n:::: DOC_TABLE_START ::::");
    bench_emit_markdown_doc_snapshot(stdout, &res, device_line);
    Serial.println(":::: DOC_TABLE_END ::::");

    Serial.println("\n(Copy between DOC_TABLE markers into compare/MCU_BENCHMARK_SNAPSHOT.md)");

    Serial.println("\n(Done — Serial @ 115200 baud)");
}

void loop(void)
{
    delay(60000);
}
