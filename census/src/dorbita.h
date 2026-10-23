/* Wiring generalizzato (punti multipli, parallele), presentazioni e forma
 * canonica delle tabelle degli eventi. */
#ifndef DORBITA_H
#define DORBITA_H

#include "dtab.h"

/* Taglia massima dell'orbita: 2n estremi di partenza per 2 versi. */
#define DORB_MAX (4 * N_MAX)

/* Ricostruzione greedy del wiring generalizzato con permutazione iniziale
 * identità: a ogni passo un insieme di >= 2 rette adiacenti, ciascuna con
 * le altre come prossimo evento, si rovescia come blocco. 1 se tutte le
 * righe si consumano; in finale (se non NULL) la permutazione finale,
 * dall'alto in basso. */
int dorb_wiring(const TabellaD *t, int *finale);

/* Presentazione con partenza dall'estremo s (0..2n-1) del bordo e verso
 * d (+1/-1). Il bordo è: ingressi dall'alto in basso (posizioni 0..n-1),
 * poi uscite dal basso in alto (posizioni n..2n-1). Le rette si numerano
 * nell'ordine del primo estremo incontrato, ciascuna orientata dal suo
 * primo estremo. Richiede la permutazione finale del wiring di t. */
void dorb_presentazione(const TabellaD *t, const int *finale, int s, int d,
                        TabellaD *out);

/* 1 se la presentazione è valida: classi di parallele consecutive e
 * wiring ricostruibile. */
int dorb_valida(const TabellaD *t);

/* Presentazioni valide e distinte (orbita); 0 se t stessa non ha wiring. */
int dorb_orbita(const TabellaD *t, TabellaD *orb);

/* Tabella canonica: minimo delle presentazioni valide. Restituisce la
 * taglia dell'orbita (0 se t non ha wiring: out non è definita). */
int dorb_canonica(const TabellaD *t, TabellaD *out);

#endif
