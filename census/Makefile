# Censimento dei tipi di arrangiamenti ottimali di Kobon.
# census-kobon: arrangiamenti semplici (compito 01);
# census-degeneri: punti multipli e parallele (compito 02, richiede GMP).
CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -D_POSIX_C_SOURCE=200809L \
         -DCARTELLA_TMP='"$(CURDIR)/tmp"' -DCARTELLA_CENSUS='"$(CURDIR)"'

OBJ_KOBON = src/tabella.o src/cnf.o src/sat.o src/wiring.o src/bruto.o \
            src/risultati.o src/main.o
OBJ_DEGENERI = src/dmain.o src/dtab.o src/dorbita.o src/dcnf.o \
               src/dbruto.o src/dauto.o src/drisultati.o src/tabella.o \
               src/geom.o src/canon.o src/json_min.o src/sha256.o

all: census-kobon census-degeneri

census-kobon: $(OBJ_KOBON)
	$(CC) $(CFLAGS) -o $@ $(OBJ_KOBON)

census-degeneri: $(OBJ_DEGENERI)
	$(CC) $(CFLAGS) -o $@ $(OBJ_DEGENERI) -lgmp

src/%.o: src/%.c src/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f census-kobon census-degeneri src/*.o

.PHONY: all clean
