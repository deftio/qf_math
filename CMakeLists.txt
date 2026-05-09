# ESP-IDF component registration for qf_math.
# This file is only meaningful inside an ESP-IDF project/component resolution context.

cmake_minimum_required(VERSION 3.16)

if(NOT ESP_PLATFORM)
  message(FATAL_ERROR
    "qf_math: CMakeLists.txt is for Espressif ESP-IDF only. "
    "Use the Makefile on the host, or consume this directory as an ESP-IDF component.")
endif()

idf_component_register(
    SRCS "src/qf_math.c"
    INCLUDE_DIRS "src"
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wconversion
    -Werror
)
