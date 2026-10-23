/* Enumerazione esaustiva indipendente dei wiring degeneri (n <= 8). */
#ifndef DBRUTO_H
#define DBRUTO_H

#include "dtab.h"

#define DBRUTO_N_MAX 8

typedef struct {
    long long stati;    /* stati visitati dalla DFS */
    long long foglie;   /* stati in cui è lecito fermarsi */
    int con_t;          /* tabelle distinte con T triangoli */
    int classi;         /* tabelle canoniche distinte fra queste */
    int semplici;       /* di cui semplici */
    TabellaD *canon;    /* tabelle canoniche, ordinate */
} EsitoDBruto;

/* DFS per n rette e T triangoli; 0 se ok, -1 se n non ammesso. */
int dbruto_esegui(int n, int T, EsitoDBruto *e);
void dbruto_libera(EsitoDBruto *e);

#endif
