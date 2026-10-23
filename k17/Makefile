# Build of the verifiers (and of the internal extraction tool).
# Requirements: a C compiler and GMP (Debian/Ubuntu: apt install libgmp-dev).

CC ?= cc
CFLAGS = -O2 -Wall -Wextra -std=c11
LDLIBS = -lgmp

PROGS = verify_direct_exact classify compare_bader build/estrai

all: $(PROGS)

verify_direct_exact: verify_direct_exact.o json_min.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

classify: classify.o json_min.o geom.o canon.o sha256.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

compare_bader: compare_bader.o json_min.o geom.o canon.o sha256.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

build/estrai: build/estrai.o json_min.o geom.o canon.o sha256.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS) -lm

%.o: %.c
	$(CC) $(CFLAGS) -I. -c -o $@ $<

sha256.o: sha256.c sha256.h
json_min.o: json_min.c json_min.h
geom.o: geom.c geom.h
canon.o: canon.c canon.h geom.h sha256.h
verify_direct_exact.o: verify_direct_exact.c json_min.h
classify.o: classify.c canon.h geom.h json_min.h
compare_bader.o: compare_bader.c canon.h geom.h json_min.h
build/estrai.o: build/estrai.c canon.h geom.h json_min.h

check: verify_direct_exact classify compare_bader
	./verify_direct_exact
	./classify --self-test
	./compare_bader --self-test

clean:
	rm -f *.o build/*.o $(PROGS)

.PHONY: all check clean
