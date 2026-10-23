# Verifica indipendente di non-isomorfismo degli arrangiamenti k18.
# Requisiti: un compilatore C e GMP (Debian/Ubuntu: apt install libgmp-dev).

CC ?= cc
CFLAGS = -O2 -Wall -Wextra -std=c11
LDLIBS = -lgmp

K17 ?= ../../../k17/k17_solutions.json

all: isocheck

isocheck: isocheck.o json_min.o geom.o canon.o sha256.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -I. -c -o $@ $<

sha256.o: sha256.c sha256.h
json_min.o: json_min.c json_min.h
geom.o: geom.c geom.h
canon.o: canon.c canon.h geom.h sha256.h
isocheck.o: isocheck.c canon.h geom.h json_min.h

check: isocheck
	./isocheck --k17 $(K17)
	./isocheck --self-test
	./isocheck

clean:
	rm -f *.o isocheck

.PHONY: all check clean
