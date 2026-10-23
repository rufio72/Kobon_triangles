/* Modello CNF di Savchuk per gli arrangiamenti di Kobon (pseudorette). */
#ifndef CNF_H
#define CNF_H

#include <stddef.h>
#include "tabella.h"

/* Formula in memoria: letterali in sequenza, ogni clausola chiusa da 0. */
typedef struct {
    int *lit;
    size_t len, cap;
    long nclausole;
    int nvar;
    int n;
} Cnf;

/* Numerazione delle variabili (indici 0-based, risultato 1-based). */
int var_A(int n, int r, int i, int k);
int var_G(int n, int r, int i, int j);
int var_X(int n, int r, int i, int j);
int var_M(int n, int r, int i, int j);

void cnf_libera(Cnf *c);
/* Costruisce il modello per n rette con al più m segmenti mancanti
 * (m = 0: arrangiamento "perfetto", senza variabili Miss). */
void cnf_kobon(Cnf *c, int n, int m);
/* Clausola bloccante che esclude esattamente la tabella t. */
void cnf_blocca(Cnf *c, const Tabella *t);
/* Scrive in DIMACS; 0 se ok, -1 se errore di scrittura. */
int cnf_scrivi(const Cnf *c, const char *percorso);

#endif
