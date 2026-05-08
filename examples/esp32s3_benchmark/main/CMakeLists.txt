get_filename_component(QF_MATH_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
set(COMP_THIRD "${QF_MATH_REPO_ROOT}/build/compare/third_party")
set(LIBMATH_DIR "${COMP_THIRD}/libfixmath/libfixmath")
set(FR_MATH_DIR "${COMP_THIRD}/fr_math/src")

if(NOT EXISTS "${LIBMATH_DIR}/fix16.c")
  message(FATAL_ERROR
    "Missing libfixmath under ${LIBMATH_DIR}.\n"
    "From the qf_math repo root run:  make compare-deps")
endif()
if(NOT EXISTS "${FR_MATH_DIR}/FR_math.c")
  message(FATAL_ERROR
    "Missing fr_math under ${FR_MATH_DIR}.\n"
    "From the qf_math repo root run:  make compare-deps")
endif()

set(QF_MATH_SRC "${QF_MATH_REPO_ROOT}/src/qf_math.c")
set(BENCH_CORE "${QF_MATH_REPO_ROOT}/compare/benchmark_core.c")

idf_component_register(
  SRCS
    "app_main.c"
    "${BENCH_CORE}"
    "${QF_MATH_SRC}"
    "${LIBMATH_DIR}/fix16.c"
    "${LIBMATH_DIR}/fract32.c"
    "${LIBMATH_DIR}/uint32.c"
    "${LIBMATH_DIR}/fix16_trig.c"
    "${LIBMATH_DIR}/fix16_sqrt.c"
    "${LIBMATH_DIR}/fix16_exp.c"
    "${FR_MATH_DIR}/FR_math.c"
  INCLUDE_DIRS
    "."
    "${QF_MATH_REPO_ROOT}/compare"
    "${QF_MATH_REPO_ROOT}/src"
    "${LIBMATH_DIR}"
    "${FR_MATH_DIR}"
  REQUIRES esp_timer)

target_compile_definitions(${COMPONENT_LIB} PRIVATE FIXMATH_NO_CACHE FR_NO_PRINT)

target_compile_options(${COMPONENT_LIB} PRIVATE
  -Wno-unused-parameter
  -Wno-sign-conversion
  -Wno-conversion)
