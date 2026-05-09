/**
 * ESP32-S3 — runs compare/benchmark_core.c with esp_timer_get_time().
 */

#include <stdio.h>

#include "benchmark_core.h"
#include "esp_chip_info.h"
#include "esp_timer.h"

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
    .ctx        = NULL,
};

void app_main(void)
{
    static volatile float sink;
    bench_results_t       res;

    esp_chip_info_t chip;
    esp_chip_info(&chip);
    printf("\nqf_math ESP32-S3 bench — chip model=%u cores=%u revision=%u\n\n",
           (unsigned)chip.model, (unsigned)chip.cores, (unsigned)chip.revision);

    bench_run_all(&s_esp_timer, &sink, &res);
    bench_print_human(&res);

    char device_line[160];
    snprintf(device_line, sizeof device_line,
             "ESP-IDF · ESP32-S3 · esp_chip model=%u cores=%u revision=%u",
             (unsigned)chip.model, (unsigned)chip.cores, (unsigned)chip.revision);

    printf("\n:::: DOC_TABLE_START ::::\n");
    bench_emit_markdown_doc_snapshot(stdout, &res, device_line);
    printf(":::: DOC_TABLE_END ::::\n");

    printf("\n(Copy between DOC_TABLE markers into compare/MCU_BENCHMARK_SNAPSHOT_ESP32S3.md)\n\n");

    printf("(Same kernels as host `make compare` — see examples/esp32s3_benchmark/README.md)\n\n");
}
