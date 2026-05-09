/**
 * Raspberry Pi Pico 2 / Pico 2 W — shared compare-suite MCU benchmark.
 */

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

extern "C" {
#include "benchmark_core.h"
}

#ifndef QF_PICO_BENCH_ARCH
#define QF_PICO_BENCH_ARCH "unknown"
#endif

static void pico_bench_init(void *ctx)
{
    (void)ctx;
}

static uint64_t pico_bench_now(void *ctx)
{
    (void)ctx;
    return (uint64_t)micros();
}

static double pico_bench_elapsed_us(void *ctx, uint64_t start, uint64_t end)
{
    (void)ctx;
    return (double)(end - start);
}

static bench_timer_t s_pico_timer = {
    .init       = pico_bench_init,
    .now        = pico_bench_now,
    .elapsed_us = pico_bench_elapsed_us,
    .ctx        = nullptr,
};

static int serial_file_write(void *cookie, const char *buf, int len)
{
    (void)cookie;
    return (int)Serial.write((const uint8_t *)buf, (size_t)len);
}

void setup()
{
    Serial.begin(115200);
    delay(1500);

    static volatile float sink;
    static bench_results_t res;

    Serial.printf("\nqf_math bench — Raspberry Pi Pico 2 W (%s)\n\n", QF_PICO_BENCH_ARCH);

    bench_run_all(&s_pico_timer, &sink, &res);

    char device_line[160];
    snprintf(device_line, sizeof device_line,
             "Raspberry Pi Pico 2 W · Arduino-Pico 5.6.0 · RP2350 %s · 150 MHz",
             QF_PICO_BENCH_ARCH);

    FILE *serial_file = funopen(nullptr, nullptr, serial_file_write, nullptr, nullptr);
    Serial.println("\n:::: DOC_TABLE_START ::::");
    bench_emit_markdown_doc_snapshot(serial_file, &res, device_line);
    fflush(serial_file);
    Serial.println(":::: DOC_TABLE_END ::::");

    Serial.println("\n(Done — Serial @ 115200 baud)");
}

void loop()
{
    delay(60000);
}
