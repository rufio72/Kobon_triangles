/* Tabella degli eventi di un arrangiamento di pseudorette con degenerazioni
 * (punti multipli e classi di rette parallele). */
#ifndef DTAB_H
#define DTAB_H

#include <stdio.h>
#include "costanti.h"
#include "geom.h"
#include "tabella.h"

/* Valore di pos per "la retta non compare nella riga" (r stessa o parallela). */
#define DT_NO 255

/* Rette 0-based. pos[r][j] è l'indice (0-based) dell'evento della riga r che
 * contiene j; due rette con lo stesso indice passano con r per un punto
 * comune. nev[r] è il numero di eventi della riga r. Le celle oltre n
 * restano a DT_NO, così due tabelle uguali sono uguali byte per byte. */
typedef struct {
    int n;
    unsigned char nev[N_MAX];
    unsigned char pos[N_MAX][N_MAX];
} TabellaD;

/* Lunghezza della chiave di confronto (vedi dtab_chiave). */
#define DT_CHIAVE (N_MAX * 2 * N_MAX)

void dtab_vuota(TabellaD *t, int n);
/* 1 se i e j (distinte) non si incrociano. */
int dtab_parallele(const TabellaD *t, int i, int j);
/* Forma: simmetria degli incroci, eventi 0..nev-1 non vuoti, punti multipli
 * coerenti fra le righe, parallelismo transitivo. 1 se ben formata. */
int dtab_ben_formata(const TabellaD *t);
/* 1 se ogni classe di parallele è un intervallo di numeri consecutivi. */
int dtab_classi_consecutive(const TabellaD *t);
/* 1 se semplice: nessuna parallela e nessun punto multiplo. */
int dtab_semplice(const TabellaD *t);
/* Membri (crescenti) dell'evento e della riga r; restituisce quanti. */
int dtab_evento(const TabellaD *t, int r, int e, int *mem);
/* Chiave di confronto: per ogni riga, per ogni evento i membri+1 e uno 0,
 * poi uno 0 di fine riga; il resto a zero. Il memcmp delle chiavi ordina
 * riga per riga, evento per evento, con il prefisso più corto minore. */
void dtab_chiave(const TabellaD *t, unsigned char *k);
int dtab_confronta(const TabellaD *a, const TabellaD *b);

/* Conversioni. */
void dtab_da_tabella(const Tabella *s, TabellaD *t);
void dtab_a_eventi(const TabellaD *t, events_t *ev);
/* 0 se ok, -1 se troppe rette o struttura incoerente. */
int dtab_da_eventi(const events_t *ev, TabellaD *t);

/* Triangoli con il criterio di adiacenza degli eventi (geom). */
int dtab_triangoli(const TabellaD *t);

/* Punti multipli: tuple crescenti (>= 3 rette) in ordine lessicografico;
 * restituisce quante (al più N_MAX*N_MAX). len[q] = taglia della tupla q. */
int dtab_punti_multipli(const TabellaD *t, int tup[][N_MAX], int *len);
/* Descrizione 1-based: "(1 3 7) (4 6 7)" oppure "nessuno". */
void dtab_scrivi_multipli(FILE *f, const TabellaD *t);
/* Classi di parallele 1-based: "{1 2} {5 6}" oppure "nessuna". */
void dtab_scrivi_parallele(FILE *f, const TabellaD *t);

/* Scrittura 1-based, una riga per retta: "2 8 4 6 (3 7) 5". */
void dtab_scrivi(FILE *f, const TabellaD *t);
/* Scrittura 1-based su una riga sola, righe separate da " ; ". */
void dtab_scrivi_compatta(FILE *f, const TabellaD *t);
/* Lettura del formato testuale (1-based): righe separate da '\n' o ';',
 * commenti '#' fino a fine riga, righe vuote ignorate. n = numero di righe.
 * 0 se ok, -1 se errore. */
int dtab_leggi(const char *s, TabellaD *t);
int dtab_leggi_file(const char *percorso, TabellaD *t);

#endif
