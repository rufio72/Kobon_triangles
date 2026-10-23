/* Modello CNF degenere: punti multipli e classi di parallele. */
#ifndef DCNF_H
#define DCNF_H

#include <stddef.h>
#include <stdio.h>
#include "dtab.h"

/* Modi di costruzione. */
enum {
    DCNF_CENSIMENTO,    /* esattamente T triangoli, almeno una degenerazione */
    DCNF_CON_SEMPLICI,  /* esattamente T triangoli, semplici ammessi */
    DCNF_UNSAT,         /* almeno T+1 triangoli (T è già T+1), senza D11 */
    DCNF_CONTROLLA      /* come DCNF_CON_SEMPLICI, per --controlla */
};

/* Gruppi di clausole (per la diagnostica di --controlla). */
enum { G_D1 = 1, G_D2, G_D3, G_D4, G_D5, G_D6, G_D7, G_D8, G_D9, G_D10,
       G_D11, G_D12, G_NUM };

typedef struct {
    int base, V, m;     /* ausiliarie s(v,k) = base + (v-1)*m + k */
    int *x;             /* letterali contati (V) */
} SinzD;

/* Formula in memoria: letterali in sequenza, ogni clausola chiusa da 0;
 * gruppo[c] = gruppo della clausola c. */
typedef struct {
    int *lit;
    size_t len, cap;
    unsigned char *gruppo;
    long nclausole, capg;
    int nvar, n;
    int nsinz;
    SinzD sinz[4];
} DCnf;

/* Numerazione delle variabili (rette 0-based, risultato 1-based). */
int dv_X(int n, int r, int i, int j);
int dv_T(int n, int a, int b, int c);
int dv_P(int n, int i, int j);
int dv_adj(int n, int r, int i, int j);
int dv_tri(int n, int a, int b, int c);
int dv_b1(int n, int r, int i, int j, int d);
int dv_b2(int n, int r, int i, int j, int d);

void dcnf_costruisci(DCnf *c, int n, int T, int modo);
void dcnf_libera(DCnf *c);
/* Restrizione a una parte dello spazio: il numero di terne per un punto
 * comune (variabili T vere) sta in [qmin, qmax] (qmax < 0: senza tetto).
 * È un invariante del tipo: parti diverse hanno tipi disgiunti. */
void dcnf_parte(DCnf *c, int qmin, int qmax);
/* Clausola D12 che esclude la presentazione t. */
void dcnf_blocca(DCnf *c, const TabellaD *t);
int dcnf_scrivi(const DCnf *c, const char *percorso);
/* Tabella dal modello; 0 se ok, -1 se il modello è incoerente. */
int dcnf_estrai(int n, const signed char *val, TabellaD *t);
/* Costruisce il modello DCNF_CONTROLLA con T triangoli e verifica che
 * l'assegnamento indotto da t soddisfi ogni clausola; stampa su rapporto
 * (se non NULL) le violate. Restituisce il numero di clausole violate. */
long dcnf_controlla(const TabellaD *t, int T, FILE *rapporto);

/* Come sat_risolvi, con tetto di tempo in secondi (0 = nessuno):
 * 10 SAT, 20 UNSAT, 0 tempo scaduto, -1 errore. */
int dcnf_risolvi(const char *kissat, const char *cnf_path,
                 const char *out_path, int nvar, signed char *val,
                 long limite);

#endif
