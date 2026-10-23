/* Scrittura dei risultati del censimento degenere: file degenereN.txt e
 * sezione aggiunta in coda al README di k<n>. */
#ifndef DRISULTATI_H
#define DRISULTATI_H

#include "dtab.h"

/* Tipi semplici al massimo noto trovati dal compito 01 (indice = n). */
static const int SEMPLICI_FASE1[11] = {0, 0, 0, 1, 1, 1, 2, 1, 0, 1, 36};

/* Certificato pubblicato della soluzione a 15 triangoli per n = 8. */
#define URL_K8 "https://raw.githubusercontent.com/ud1/kobon-solutions/HEAD/" \
               "gallery/certificates/8/8-1yvz24guzamh9.json"

/* Rimuove i degenereN.txt precedenti e scrive i nuovi (uno per tipo).
 * sha[i] = canone-eventi (SHA-256) del tipo i. */
void dris_scrivi_tipi(const char *dir, int n, int T, const TabellaD *tipi,
                      char (*sha)[65], int ntipi);

/* Sostituisce (o aggiunge) in coda a dir/README.md la sezione
 * "## Degenerate arrangements": tutto ciò che precede resta intatto.
 * tetto: testo inglese sull'esito a T+1 (es. "UNSAT (kissat, 12.3 s)");
 * nota: riga inglese facoltativa (NULL se nessuna); parziale: enumerazione
 * non completata (i conteggi sono minimi). */
void dris_readme(const char *dir, int n, int T, int ntipi, int scartate,
                 const char *tetto, const char *nota, int parziale);

#endif
