/**
 * benchmark_suite.c — Host benchmark: qf_math vs libm vs libfixmath vs fr_math.
 *
 * Shared kernels live in benchmark_core.c (also built for `examples/esp32s3_benchmark` and `examples/lilygo_t_display_s3_bench`).
 *
 * Flags:
 *   (none)           — human-readable report on stdout
 *   --markdown-body  — Markdown only (matrices for compare/gen_github_report.sh)
 *
 * Copyright (c) 2002-2026, M. A. Chatterjee — qf_math portions BSD-2-Clause.
 */

#include "benchmark_core.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if !defined(_WIN32) && (defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__) || defined(__unix__))
#include <sys/utsname.h>
#define QF_HAVE_UTSNAME 1
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#include "qf_math.h"

static volatile float g_sink;
static int            g_human   = 1;
static int            g_md_body = 0;

#ifdef __APPLE__
static double           timer_ns_per_tick;
static void host_timer_init(void *ctx)
{
    (void)ctx;
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    timer_ns_per_tick = (double)info.numer / (double)info.denom;
}
static uint64_t host_timer_now(void *ctx)
{
    (void)ctx;
    return mach_absolute_time();
}
static double host_timer_elapsed_us(void *ctx, uint64_t start, uint64_t end)
{
    (void)ctx;
    return (double)(end - start) * timer_ns_per_tick / 1000.0;
}
#else
static void host_timer_init(void *ctx)
{
    (void)ctx;
}
static uint64_t host_timer_now(void *ctx)
{
    (void)ctx;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}
static double host_timer_elapsed_us(void *ctx, uint64_t start, uint64_t end)
{
    (void)ctx;
    return (double)(end - start) / 1000.0;
}
#endif

static bench_timer_t g_host_timer = {
    .init       = host_timer_init,
    .now        = host_timer_now,
    .elapsed_us = host_timer_elapsed_us,
    .ctx        = NULL,
};

static void md_host_meta(FILE *out)
{
    time_t     now = time(NULL);
    struct tm *utc = gmtime(&now);
    char       tbuf[64];
    strftime(tbuf, sizeof tbuf, "%Y-%m-%d %H:%M:%SZ", utc);

    fprintf(out, "## Automated measurements\n\n");
    fprintf(out, "### Host metadata\n\n");
    fprintf(out, "| Field | Value |\n");
    fprintf(out, "| --- | --- |\n");
    fprintf(out, "| UTC time | %s |\n", tbuf);
#ifdef QF_HAVE_UTSNAME
    struct utsname u;
    if (uname(&u) == 0)
        fprintf(out, "| uname | %s %s %s |\n", u.sysname, u.machine, u.release);
#endif
#ifdef __VERSION__
    fprintf(out, "| Compiler | %s |\n", __VERSION__);
#endif
    fprintf(out, "| Pointer size | %lu-bit |\n", (unsigned long)(sizeof(void *) * 8UL));
    fprintf(out, "| qf_math | %s (`QF_MATH_VERSION_HEX`=%#x) |\n", QF_MATH_VERSION,
            QF_MATH_VERSION_HEX);
    fprintf(out, "| Loop shape | %d sample grid × %d outer × 64 inner calls |\n\n", BENCH_N_SAMPLES,
            BENCH_N_ITER);
}

static int parse_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--markdown-body") == 0) {
            g_human   = 0;
            g_md_body = 1;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (parse_args(argc, argv))
        return 1;

    bench_results_t res;
    bench_run_all(&g_host_timer, &g_sink, &res);

    FILE *md = g_md_body ? stdout : NULL;

    if (g_human)
        bench_print_human(&res);

    if (g_md_body && md) {
        md_host_meta(md);
        bench_emit_markdown_tables(md, &res, BENCH_MD_HOST);
    }

    if (g_human) {
        printf("Notes:\n");
        printf("  - Timings are desktop-oriented; rankings change on MCUs with/without FPU.\n");
        printf("  - Float<->fixed bridges dominate libfixmath/fr_math times here; ship native fixed types for real firmware.\n");
        printf("  - Markdown report: `make compare-github-report`\n");
        printf("  - Docs: compare/FUNCTION_MATRIX.md, compare/LIBRARIES.md, compare/PEERS.md\n");
        printf("  - Sizes: `make compare-report`\n");
        printf("  - Upstream libfixmath tests: `make compare-tests`; fr_math: `make compare-fr-tests`\n");
        printf("  - On-device (ESP32-S3): examples/lilygo_t_display_s3_bench/ (PlatformIO) or examples/esp32s3_benchmark/ (ESP-IDF)\n");
    }

    (void)md;
    return 0;
}
