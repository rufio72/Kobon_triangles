/* Tabella degli ordini di un arrangiamento semplice di pseudorette. */
#ifndef TABELLA_H
#define TABELLA_H

#include <stdio.h>
#include "costanti.h"

/* La riga r (0-based) elenca, nell'ordine lungo la retta r+1, le n-1 rette
 * incontrate, numerate 1..n. Le celle non usate restano a zero. */
typedef struct {
    int n;
    unsigned char r[N_MAX][N_MAX - 1];
} Tabella;

void tab_vuota(Tabella *t, int n);
/* La retta f (1-based) diventa la retta 1. */
void tab_reindex(const Tabella *t, int f, Tabella *out);
/* Inversione del verso. */
void tab_inverti(const Tabella *t, Tabella *out);
/* Confronto lessicografico riga per riga: <0, 0, >0. */
int tab_confronta(const Tabella *a, const Tabella *b);
/* Orbita completa (chiusura BFS); restituisce la taglia. */
int tab_orbita(const Tabella *t, Tabella *orb);
/* Forma canonica: minimo lessicografico dell'orbita. */
void tab_canonica(const Tabella *t, Tabella *out);
/* Numero di triangoli vuoti (terne adiacenti su tutte e tre le righe). */
int tab_triangoli(const Tabella *t);
/* Controllo di forma: ogni riga è una permutazione delle altre rette. */
int tab_ben_formata(const Tabella *t);
/* Scrittura: n righe di n-1 interi. */
void tab_scrivi(FILE *f, const Tabella *t);
/* Scrittura compatta [[..],[..]] (anche formato JSON). */
void tab_stampa_lista(FILE *f, const Tabella *t);
/* Lettura di n*(n-1) interi da una stringa; 0 se ok, -1 se errore. */
int tab_leggi(const char *s, int n, Tabella *t);

#endif
