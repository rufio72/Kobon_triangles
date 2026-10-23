/* File dei tipi degeneri e sezione dei README (in inglese, come il resto
 * dei README di k<n>). I file esistenti del compito 01 non si toccano. */
#include "drisultati.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void percorso(char *p, size_t len, const char *dir, const char *nome)
{
    snprintf(p, len, "%s/%s", dir, nome);
}

void dris_scrivi_tipi(const char *dir, int n, int T, const TabellaD *tipi,
                      char (*sha)[65], int ntipi)
{
    char p[2048], nome[64];
    for (int i = 1;; i++) {
        snprintf(nome, sizeof nome, "degenere%d.txt", i);
        percorso(p, sizeof p, dir, nome);
        if (unlink(p) != 0)
            break;
    }
    for (int i = 0; i < ntipi; i++) {
        snprintf(nome, sizeof nome, "degenere%d.txt", i + 1);
        percorso(p, sizeof p, dir, nome);
        FILE *f = fopen(p, "w");
        if (!f) {
            fprintf(stderr, "errore: impossibile scrivere %s\n", p);
            exit(2);
        }
        fprintf(f, "# n=%d T=%d degenere %d di %d\n", n, T, i + 1, ntipi);
        fputs("# punti multipli (rette 1-based): ", f);
        dtab_scrivi_multipli(f, &tipi[i]);
        fputs("\n# classi parallele: ", f);
        dtab_scrivi_parallele(f, &tipi[i]);
        fprintf(f, "\n# metodo: modello CNF degenere + kissat 4.0.4, "
                   "validazione con wiring generalizzato, tabella canonica "
                   "(minimo delle presentazioni valide)\n");
        fprintf(f, "# canone-eventi (canonical_sha256): %s\n", sha[i]);
        fputs("# riga r: eventi lungo la retta r; fra parentesi le rette di "
              "un punto multiplo; le parallele a r non compaiono\n", f);
        dtab_scrivi(f, &tipi[i]);
        fclose(f);
    }
}

#define TITOLO "## Degenerate arrangements"

void dris_readme(const char *dir, int n, int T, int ntipi, int scartate,
                 const char *tetto, const char *nota, int parziale)
{
    char p[2048];
    percorso(p, sizeof p, dir, "README.md");
    FILE *f = fopen(p, "r");
    if (!f) {
        fprintf(stderr, "errore: %s non esiste\n", p);
        exit(2);
    }
    static char buf[1 << 20];
    size_t l = fread(buf, 1, sizeof buf - 1, f);
    fclose(f);
    buf[l] = '\0';
    /* Una sezione già scritta da un'esecuzione precedente si sostituisce. */
    char *vecchia = strstr(buf, "\n\n" TITOLO);
    if (vecchia)
        vecchia[1] = '\0';
    f = fopen(p, "w");
    if (!f) {
        fprintf(stderr, "errore: impossibile scrivere %s\n", p);
        exit(2);
    }
    fputs(buf, f);
    size_t lb = strlen(buf);
    if (lb && buf[lb - 1] != '\n')
        fputc('\n', f);
    char data[16];
    time_t ora = time(NULL);
    strftime(data, sizeof data, "%Y-%m-%d", localtime(&ora));
    int s = n <= 10 ? SEMPLICI_FASE1[n] : -1;
    fprintf(f, "\n" TITOLO " (census extension, %s)\n\n", data);
    fprintf(f,
        "Allowing multiple points and parallel lines (counting convention "
        "of the\npublished records: a line through a vertex only touches the "
        "triangle),\nthe census at %d triangles finds %s%d degenerate type%s",
        T, parziale ? "at least " : "", ntipi, ntipi == 1 ? "" : "s");
    if (ntipi == 1)
        fputs(" (`degenere1.txt`).\n", f);
    else if (ntipi > 1)
        fprintf(f, " (`degenere1.txt` … `degenere%d.txt`).\n", ntipi);
    else
        fputs(".\n", f);
    fprintf(f, "Total types, simple + degenerate: %d + %d = %d%s.\n", s,
            ntipi, s + ntipi, parziale ? " (lower bound, partial)" : "");
    fprintf(f, "Ceiling at %d with degeneracies: %s.\n", T + 1, tetto);
    fprintf(f,
        "Method: extended CNF model (see census/), kissat 4.0.4, validation "
        "by\ngeneralized wiring reconstruction");
    if (n == 8)
        fprintf(f, ", cross-check against the published\nrational solution "
                   "(ud1/kobon-solutions certificate,\n%s).\n", URL_K8);
    else
        fputs(".\n", f);
    fprintf(f,
        "Parallelism is transitive (classes of directions, as for straight "
        "lines);\ntypes are pseudoline arrangements up to relabeling, "
        "reversal and\nreflection. Solutions rejected by the wiring "
        "validation: %d.\n", scartate);
    if (nota)
        fprintf(f, "%s\n", nota);
    if (ntipi > 0)
        fputs("In a `degenere*.txt` table row r lists the events along line "
              "r; lines\nthrough a common point are in parentheses, lines "
              "parallel to r are absent.\n", f);
    fclose(f);
}
