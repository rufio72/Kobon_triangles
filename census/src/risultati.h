/* Scrittura dei risultati nella cartella k{n}. */
#ifndef RISULTATI_H
#define RISULTATI_H

#include "tabella.h"

/* Oltre questa soglia i tipi vanno in un unico soluzioni.json. */
#define SOGLIA_JSON 20

/* Scrive tipoN.txt (o soluzioni.json) e README.md in dir, creandola.
 * tetto: esito della verifica a T+1 ("aritmetico", "UNSAT", ...). */
void scrivi_risultati(const char *dir, int n, int T, const Tabella *tipi,
                      int ntipi, int scartate, const char *tetto);

#endif
