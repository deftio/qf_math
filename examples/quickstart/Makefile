# qf_math quick start example
#
# Build:  make
# Run:    ./quickstart
# Clean:  make clean

CC      ?= gcc
CFLAGS   = -std=c99 -Os -Wall -Wextra -Wpedantic
QF_SRC   = ../../src

.PHONY: all clean run

all: quickstart

quickstart: main.c $(QF_SRC)/qf_math.c $(QF_SRC)/qf_math.h
	$(CC) $(CFLAGS) -I$(QF_SRC) main.c $(QF_SRC)/qf_math.c -o $@

run: quickstart
	./quickstart

clean:
	rm -f quickstart
