FROM --platform=linux/amd64 ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Cross-compilers for qf_math ROM/code-size reports (same stack as fr_math / xelp).
# Build: docker build -t qf-math-sizes docker/

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gcc clang tcc \
    libc6-dev-i386 \
    lib32gcc-s1 \
    gcc-aarch64-linux-gnu \
    gcc-arm-none-eabi \
    gcc-msp430 \
    gcc-avr \
    gcc-m68hc1x \
    gcc-m68k-linux-gnu \
    gcc-powerpc-linux-gnu \
    gcc-riscv64-linux-gnu \
    gcc-riscv64-unknown-elf \
    gcc-xtensa-lx106 \
    picolibc-xtensa-lx106-elf \
    gcc-mipsel-linux-gnu \
    sdcc \
    binutils \
    wget ca-certificates xz-utils bzip2 \
    && rm -rf /var/lib/apt/lists/*

# Espressif unified Xtensa toolchain (ESP32 LX6, ESP32-S2/S3 LX7).
RUN wget -q https://github.com/espressif/crosstool-NG/releases/download/esp-15.2.0_20251204/xtensa-esp-elf-15.2.0_20251204-x86_64-linux-gnu.tar.xz \
    -O /tmp/xtensa-esp.tar.xz \
    && tar -xJf /tmp/xtensa-esp.tar.xz -C /opt \
    && for f in /opt/xtensa-esp-elf/bin/xtensa-esp*; do ln -sf "$f" /usr/local/bin/; done \
    && rm /tmp/xtensa-esp.tar.xz

WORKDIR /src
