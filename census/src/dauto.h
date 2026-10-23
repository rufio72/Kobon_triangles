/* Autotest e riscontro con dati geometrici razionali esatti. */
#ifndef DAUTO_H
#define DAUTO_H

#include "dtab.h"

/* Riscontro di un file JSON con lines_frac (formato k18): stampa triangoli,
 * punti multipli, parallele, canone-eventi; se le rette entrano in N_MAX
 * anche la tabella canonica. 0 se ok. */
int dauto_riscontro(const char *json);

/* I cinque autotest del compito 02; radice = cartella census. 0 se tutti
 * passano. */
int dauto_autotest(const char *radice);

/* Canone-eventi delle rette razionali di un file JSON; 0 se ok. */
int dauto_sha_json(const char *json, char out[65]);

/* Canone-eventi (canonical_sha256) di una tabella. */
void dauto_sha(const TabellaD *t, char out[65]);

#endif
