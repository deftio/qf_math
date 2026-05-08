# qf_math — Quick Float Math Library
# @author M A Chatterjee <deftio [at] deftio [dot] com>

CC      ?= gcc
CFLAGS   = -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror -Os -std=c99
CFLAGS_RELAX = -Wall -Wextra -Os -std=c99 -Wno-unused-parameter -Wno-conversion
LDFLAGS  = -lm

# Defined only for test binaries so internal helpers can be exercised (coverage).
CFLAGS_TEST = -DQF_MATH_COVERAGE

SRCDIR   = src
TESTDIR  = test
TOOLSDIR = tools
BUILD    = build

SRC      = $(SRCDIR)/qf_math.c
HDR      = $(SRCDIR)/qf_math.h
TEST_SRC = $(TESTDIR)/qf_math_test.c

# --- compare/ harness (deps under build/compare/third_party/) ---
COMP_DIR    = $(BUILD)/compare
COMPARE_DIR = compare
COMP_OBJ    = $(COMP_DIR)/obj
COMP_THIRD  = $(COMP_DIR)/third_party
LIBFIXMATH_ROOT = $(COMP_THIRD)/libfixmath/libfixmath
FR_MATH_ROOT    = $(COMP_THIRD)/fr_math/src
COMPARE_STAMP   = $(COMP_DIR)/.deps-ready

LIBFIXMATH_SRCS = fix16.c fract32.c uint32.c fix16_trig.c fix16_sqrt.c fix16_exp.c
LIBFIXMATH_OBJS = $(addprefix $(COMP_OBJ)/lf_,$(LIBFIXMATH_SRCS:.c=.o))
FR_MATH_OBJ      = $(COMP_OBJ)/fr_math.o

.PHONY: help
help:
	@echo "qf_math — Quick Float Math Library"
	@echo ""
	@echo "Usage: make <target>"
	@echo ""
	@echo "Build targets:"
	@echo "  lib            Build qf_math object file → $(BUILD)/"
	@echo "  test           Build and run unit tests"
	@echo "  bench          Build and run qf_math vs libm benchmark"
	@echo "  docs-pages     Inject fresh bench sweep into docs/pages/index.html"
	@echo ""
	@echo "Comparison (see compare/README.md):"
	@echo "  compare-deps   Fetch libfixmath + fr_math into $(COMP_THIRD)/"
	@echo "  compare        Build/run $(COMP_DIR)/benchmark_suite"
	@echo "  compare-report Print Markdown size table (needs: lib + compare-deps)"
	@echo "  compare-github-report Regenerate compare/BENCHMARK_REPORT.md for GitHub"
	@echo "  compare-matrix Alias for compare"
	@echo "  compare-tests  Run libfixmath upstream tests_ro64 (CMake)"
	@echo "  compare-fr-tests Run fr_math upstream makefile test suite"
	@echo "  mcu-benchmark-snapshot  Flash LilyGO bench → compare/MCU_BENCHMARK_SNAPSHOT.md (USB)"
	@echo "  benchmark-crossplatform Merge host + MCU snapshots → compare/BENCHMARK_CROSSPLATFORM.md"
	@echo ""
	@echo "Analysis:"
	@echo "  size           Show compiled object size"
	@echo "  coverage       Build with coverage and run tests (prints gcov summary)"
	@echo "  coverage-check Build coverage and fail if src/qf_math.c is not 100% lines"
	@echo ""
	@echo "Docker (cross-compile sizes — see docker/README.md):"
	@echo "  docker-sizes          docker/run.sh → build/docker_size_table.md"
	@echo "  docker-sizes-rebuild  Rebuild toolchain image, then run report"
	@echo ""
	@echo "Maintenance:"
	@echo "  clean          Remove $(BUILD) and coverage artifacts"

.PHONY: dirs
dirs:
	@mkdir -p $(BUILD)

.PHONY: lib
lib: dirs $(BUILD)/qf_math.o

$(BUILD)/qf_math.o: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -c $(SRC) -o $@

.PHONY: test
test: dirs $(BUILD)/qf_math_test
	@echo "Running qf_math tests..."
	@echo ""
	@$(BUILD)/qf_math_test

$(BUILD)/qf_math_test: $(TEST_SRC) $(SRC) $(HDR)
	$(CC) -Wall -Wextra -Os -std=c99 -I$(SRCDIR) $(CFLAGS_TEST) $(TEST_SRC) $(SRC) $(LDFLAGS) -o $@

$(BUILD)/qf_math_bench: $(TOOLSDIR)/qf_math_bench.c $(SRC) $(HDR)
	$(CC) -Wall -Wextra -Os -std=c99 -I$(SRCDIR) $(TOOLSDIR)/qf_math_bench.c $(SRC) $(LDFLAGS) -o $@

.PHONY: bench
bench: dirs $(BUILD)/qf_math_bench
	@$(BUILD)/qf_math_bench

.PHONY: docs-pages
docs-pages: dirs $(BUILD)/qf_math_bench
	python3 tools/inject_docs_bench_table.py

# --- compare suite ---
.PHONY: compare-deps
compare-deps: $(COMPARE_STAMP)

$(COMPARE_STAMP):
	@bash compare/fetch_deps.sh
	@mkdir -p $(dir $@)
	@touch $@

$(COMP_OBJ)/lf_%.o: $(LIBFIXMATH_ROOT)/%.c | $(COMPARE_STAMP)
	@mkdir -p $(COMP_OBJ)
	$(CC) $(CFLAGS_RELAX) -c $< -o $@ -I$(LIBFIXMATH_ROOT) -DFIXMATH_NO_CACHE

$(FR_MATH_OBJ): $(FR_MATH_ROOT)/FR_math.c | $(COMPARE_STAMP)
	@mkdir -p $(COMP_OBJ)
	$(CC) $(CFLAGS_RELAX) -c $< -o $@ -I$(FR_MATH_ROOT) -DFR_NO_PRINT

COMPARE_BIN = $(COMP_DIR)/benchmark_suite

$(COMPARE_BIN): compare/benchmark_suite.c compare/benchmark_core.c $(SRC) $(HDR) $(LIBFIXMATH_OBJS) $(FR_MATH_OBJ)
	$(CC) $(CFLAGS_RELAX) -I$(SRCDIR) -I$(COMPARE_DIR) -I$(LIBFIXMATH_ROOT) -I$(FR_MATH_ROOT) \
		compare/benchmark_suite.c compare/benchmark_core.c $(SRC) $(LIBFIXMATH_OBJS) $(FR_MATH_OBJ) $(LDFLAGS) -o $@

.PHONY: compare compare-matrix
compare compare-matrix: dirs compare-deps $(COMPARE_BIN)
	@echo ""
	@$(COMPARE_BIN)

.PHONY: compare-report
compare-report: lib compare-deps $(LIBFIXMATH_OBJS) $(FR_MATH_OBJ)
	@bash compare/report_sizes.sh "$(BUILD)/qf_math.o" "$(FR_MATH_OBJ)" $(LIBFIXMATH_OBJS)

.PHONY: compare-github-report
compare-github-report: lib compare-deps $(COMPARE_BIN) $(LIBFIXMATH_OBJS) $(FR_MATH_OBJ)
	@bash compare/gen_github_report.sh

.PHONY: compare-fr-tests
compare-fr-tests:
	@bash compare/run_fr_math_tests.sh

.PHONY: mcu-benchmark-snapshot
mcu-benchmark-snapshot: compare-deps
	python3 $(TOOLSDIR)/mcu_benchmark_snapshot.py

.PHONY: benchmark-crossplatform
benchmark-crossplatform:
	python3 $(TOOLSDIR)/gen_benchmark_crossplatform.py

.PHONY: compare-tests
compare-tests:
	@bash $(TOOLSDIR)/run_libfixmath_tests.sh

.PHONY: size
size: lib
	@echo "=== qf_math Object Size ==="
	@size $(BUILD)/qf_math.o

.PHONY: coverage
coverage:
	@bash tools/check_coverage.sh --report-only

.PHONY: coverage-check
coverage-check:
	@bash tools/check_coverage.sh

.PHONY: docker-sizes
docker-sizes:
	@bash docker/run.sh

.PHONY: docker-sizes-rebuild
docker-sizes-rebuild:
	@bash docker/run.sh --rebuild

.PHONY: clean
clean:
	rm -rf $(BUILD)
	find . \( -name '*.gcda' -o -name '*.gcno' -o -name '*.gcov' \) -delete 2>/dev/null || true
