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

#define EXTRA_NFUNC 5
#define EXTRA_N_ITER (BENCH_N_ITER / 10)

typedef float (*extra_fn_t)(float);

typedef struct extra_peer {
    const char *name;
    extra_fn_t funcs[EXTRA_NFUNC];
    double     acc_pct[EXTRA_NFUNC];
    double     time_us[EXTRA_NFUNC];
} extra_peer_t;

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
static float fasttrig_cos_f(float rad) { return icos(rad * QF_RAD2DEG_K); }

static float libm_sin_f(float x) { return sinf(x); }
static float libm_cos_f(float x) { return cosf(x); }
static float libm_sqrt_f(float x) { return sqrtf(x); }
static float libm_ln_f(float x) { return logf(x); }
static float libm_exp_f(float x) { return expf(x); }

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
static float espp_cos_f(float rad) { return espp::fast_cos(wrap_rad_0_2pi(rad)); }
static float espp_sqrt_f(float x) { return x * espp::fast_inv_sqrt(x); }
static float espp_ln_f(float x) { return espp::fast_ln(x); }

static void extra_clear(extra_peer_t *p)
{
    for (int f = 0; f < EXTRA_NFUNC; f++) {
        p->acc_pct[f] = NAN;
        p->time_us[f] = NAN;
    }
}

static void extra_track_abs(double *acc_pct, double ref, double got)
{
    double e = fabs(ref - got) * 100.0;
    if (e > *acc_pct || isnan(*acc_pct))
        *acc_pct = e;
}

static void extra_track_rel(double *acc_pct, double ref, double got)
{
    double ar = fabs(ref);
    if (ar <= 1e-18)
        return;
    double e = fabs((got - ref) / ref) * 100.0;
    if (e > *acc_pct || isnan(*acc_pct))
        *acc_pct = e;
}

static void extra_fill_inputs(float inputs[EXTRA_NFUNC][BENCH_N_SAMPLES])
{
    for (int i = 0; i < BENCH_N_SAMPLES; i++) {
        float t = (float)i / (float)(BENCH_N_SAMPLES - 1);
        inputs[BENCH_F_SIN][i] = -6.2831853f + 12.5663706f * t;
        inputs[BENCH_F_COS][i] = inputs[BENCH_F_SIN][i];
        inputs[BENCH_F_SQRT][i] = 0.0005f + 8000.0f * t;
        inputs[BENCH_F_LN][i] = 0.05f + 120.0f * t;
        inputs[BENCH_F_EXP][i] = -6.0f + 12.0f * t;
    }
}

static double extra_time_func(const bench_timer_t *tm,
                              volatile float *sink,
                              extra_fn_t fn,
                              const float *inputs)
{
    uint64_t t0 = tm->now(tm->ctx);
    for (int r = 0; r < EXTRA_N_ITER; r++) {
        for (int i = 0; i < 64; i++)
            *sink = fn(inputs[i % BENCH_N_SAMPLES]);
    }
    uint64_t t1 = tm->now(tm->ctx);
    return tm->elapsed_us(tm->ctx, t0, t1);
}

static void extra_bench_one(const bench_timer_t *tm,
                            volatile float *sink,
                            extra_peer_t *p,
                            float inputs[EXTRA_NFUNC][BENCH_N_SAMPLES])
{
    for (int f = 0; f < EXTRA_NFUNC; f++) {
        if (!p->funcs[f])
            continue;

        for (int i = 0; i < BENCH_N_SAMPLES; i++) {
            float x = inputs[f][i];
            float got = p->funcs[f](x);
            switch (f) {
            case BENCH_F_SIN:
                extra_track_abs(&p->acc_pct[f], sin((double)x), (double)got);
                break;
            case BENCH_F_COS:
                extra_track_abs(&p->acc_pct[f], cos((double)x), (double)got);
                break;
            case BENCH_F_SQRT:
                extra_track_rel(&p->acc_pct[f], sqrt((double)x), (double)got);
                break;
            case BENCH_F_LN:
                if (got == got)
                    extra_track_rel(&p->acc_pct[f], log((double)x), (double)got);
                break;
            case BENCH_F_EXP: {
                double ref = exp((double)x);
                if (ref <= 1e12 && ref >= 1e-12)
                    extra_track_rel(&p->acc_pct[f], ref, (double)got);
                break;
            }
            default:
                break;
            }
        }

        p->time_us[f] = extra_time_func(tm, sink, p->funcs[f], inputs[f]);
    }
}

static void print_cell(FILE *out, double v)
{
    if (isnan(v))
        fprintf(out, " — |");
    else
        fprintf(out, " %.6f |", v);
}

static void emit_extra_fastmath_markdown(FILE *out,
                                         const bench_timer_t *tm,
                                         volatile float *sink,
                                         const bench_results_t *base)
{
    (void)base;
    float inputs[EXTRA_NFUNC][BENCH_N_SAMPLES];
    extra_fill_inputs(inputs);

    double extra_libm_us[EXTRA_NFUNC] = {
        extra_time_func(tm, sink, libm_sin_f, inputs[BENCH_F_SIN]),
        extra_time_func(tm, sink, libm_cos_f, inputs[BENCH_F_COS]),
        extra_time_func(tm, sink, libm_sqrt_f, inputs[BENCH_F_SQRT]),
        extra_time_func(tm, sink, libm_ln_f, inputs[BENCH_F_LN]),
        extra_time_func(tm, sink, libm_exp_f, inputs[BENCH_F_EXP]),
    };

    extra_peer_t peers[] = {
        {"FastTrig", {fasttrig_sin_f, fasttrig_cos_f, nullptr, nullptr, nullptr}},
        {"ESP-DSP", {nullptr, nullptr, espdsp_sqrt_f, nullptr, nullptr}},
        {"espp/math", {espp_sin_f, espp_cos_f, espp_sqrt_f, espp_ln_f, nullptr}},
    };

    fprintf(out, "\n### ESP32-S3 fast math peers\n\n");
    fprintf(out,
            "Additional ESP32-S3 peer implementations run in the LilyGO PlatformIO build. "
            "Cells marked `—` mean that library does not expose a matching scalar function in this harness. "
            "Peer timings use an 8000 sample grid × %d outer × 64 inner calls with a local libm baseline.\n\n",
            EXTRA_N_ITER);

    int n = (int)(sizeof(peers) / sizeof(peers[0]));
    for (int i = 0; i < n; i++) {
        extra_clear(&peers[i]);
        extra_bench_one(tm, sink, &peers[i], inputs);
    }

    fprintf(out, "#### Accuracy — max percent error\n\n");
    fprintf(out, "| Library | sin | cos | sqrt | ln | exp |\n");
    fprintf(out, "| :--- | ---:| ---:| ---:| ---:| ---:|\n");
    fprintf(out, "| libm (reference) |");
    for (int f = 0; f < EXTRA_NFUNC; f++)
        print_cell(out, extra_libm_us[f]);
    fprintf(out, "\n");
    for (int i = 0; i < n; i++) {
        fprintf(out, "| **%s** |", peers[i].name);
        for (int f = 0; f < EXTRA_NFUNC; f++)
            print_cell(out, peers[i].acc_pct[f]);
        fprintf(out, "\n");
    }

    fprintf(out, "\n#### Wall-clock time (microseconds)\n\n");
    fprintf(out, "| Library | sin | cos | sqrt | ln | exp |\n");
    fprintf(out, "| :--- | ---:| ---:| ---:| ---:| ---:|\n");
    for (int i = 0; i < n; i++) {
        fprintf(out, "| **%s** |", peers[i].name);
        for (int f = 0; f < EXTRA_NFUNC; f++)
            print_cell(out, peers[i].time_us[f]);
        fprintf(out, "\n");
    }

    fprintf(out, "\n#### Speed vs libm (ratio)\n\n");
    fprintf(out, "`libm` time ÷ peer time using the peer-section timing loop.\n\n");
    fprintf(out, "| Library | sin | cos | sqrt | ln | exp |\n");
    fprintf(out, "| :--- | ---:| ---:| ---:| ---:| ---:|\n");
    for (int i = 0; i < n; i++) {
        fprintf(out, "| **%s** |", peers[i].name);
        for (int f = 0; f < EXTRA_NFUNC; f++) {
            if (isnan(peers[i].time_us[f]))
                fprintf(out, " — |");
            else
                print_cell(out, bench_ratio_vs_libm(extra_libm_us[f], peers[i].time_us[f]));
        }
        fprintf(out, "\n");
    }
    fprintf(out, "\n");
}

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
    bench_print_human(&res);

    char device_line[160];
    snprintf(device_line, sizeof device_line,
             "LilyGO T-Display-S3 · Arduino-ESP32 · esp_chip model=%u cores=%u revision=%u",
             (unsigned)chip.model, (unsigned)chip.cores, (unsigned)chip.revision);

    Serial.println("\n:::: DOC_TABLE_START ::::");
    bench_emit_markdown_doc_snapshot(stdout, &res, device_line);
    emit_extra_fastmath_markdown(stdout, &s_esp_timer, &sink, &res);
    Serial.println(":::: DOC_TABLE_END ::::");

    Serial.println("\n(Copy between DOC_TABLE markers into compare/MCU_BENCHMARK_SNAPSHOT.md)");

    Serial.println("\n(Done — Serial @ 115200 baud)");
}

void loop(void)
{
    delay(60000);
}
