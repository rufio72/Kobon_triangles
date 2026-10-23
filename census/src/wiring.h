/* Validazione delle tabelle tramite wiring diagram ed estrazione dal modello. */
#ifndef WIRING_H
#define WIRING_H

#include "tabella.h"

/* Ricostruzione greedy del wiring diagram partendo dalla permutazione
 * identità: 1 se riesce (la tabella è un arrangiamento di pseudorette). */
int wiring_valida(const Tabella *t);
/* 1 se la ricostruzione riesce su almeno un elemento dell'orbita. */
int wiring_valida_orbita(const Tabella *t);
/* Estrae la tabella dai valori del modello SAT (val[v] = +1/-1).
 * Restituisce 0 se ok, -1 se il modello è incoerente. */
int wiring_estrai(int n, const signed char *val, Tabella *t);

#endif
