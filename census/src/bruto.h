/* Enumerazione esaustiva indipendente dei wiring diagram (n <= 7). */
#ifndef BRUTO_H
#define BRUTO_H

#include "tabella.h"

#define BRUTO_N_MAX 7

typedef struct {
    long long parole;   /* foglie della DFS: parole ridotte di w0 */
    int distinte;       /* tabelle distinte */
    int con_tmax;       /* tabelle distinte con T triangoli */
    int classi;         /* forme canoniche distinte tra queste */
    int non_valide;     /* tabelle che non superano la ricostruzione greedy */
    Tabella *canon;     /* forme canoniche, ordinate */
} EsitoBruto;

/* Esegue la DFS per n rette e massimo T; 0 se ok, -1 se n non ammesso. */
int bruto_esegui(int n, int T, EsitoBruto *e);
void bruto_libera(EsitoBruto *e);

#endif
