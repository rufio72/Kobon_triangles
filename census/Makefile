# Censimento dei tipi di arrangiamenti ottimali di Kobon.
CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -D_POSIX_C_SOURCE=200809L \
         -DCARTELLA_TMP='"$(CURDIR)/tmp"'
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

census-kobon: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

src/%.o: src/%.c src/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f census-kobon src/*.o

.PHONY: clean
