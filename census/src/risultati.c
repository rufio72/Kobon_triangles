/* Scrittura dei risultati: un file per tipo (o un JSON unico) e README.
 * Il README è in inglese, come quelli già pubblicati in k14-k18. */
#include "risultati.h"
#include "costanti.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static FILE *apri(const char *dir, const char *nome)
{
    char p[2048];
    snprintf(p, sizeof p, "%s/%s", dir, nome);
    FILE *f = fopen(p, "w");
    if (!f) {
        fprintf(stderr, "errore: impossibile scrivere %s\n", p);
        exit(2);
    }
    return f;
}

/* Rimuove i risultati di un'esecuzione precedente. */
static void pulisci(const char *dir)
{
    char p[2048];
    for (int i = 1;; i++) {
        snprintf(p, sizeof p, "%s/tipo%d.txt", dir, i);
        if (unlink(p) != 0)
            break;
    }
    snprintf(p, sizeof p, "%s/soluzioni.json", dir);
    unlink(p);
}

static void scrivi_tipi(const char *dir, int n, int T, const Tabella *tipi,
                        int ntipi)
{
    char nome[64];
    for (int i = 0; i < ntipi; i++) {
        snprintf(nome, sizeof nome, "tipo%d.txt", i + 1);
        FILE *f = apri(dir, nome);
        fprintf(f, "# n=%d T=%d tipo %d di %d\n", n, T, i + 1, ntipi);
        fprintf(f, "# metodo: modello CNF di Savchuk + kissat, forma "
                   "canonica (minimo lessicografico dell'orbita)\n");
        fprintf(f, "# riga r: rette incontrate lungo la retta r, in ordine\n");
        tab_scrivi(f, &tipi[i]);
        fclose(f);
    }
}

static void scrivi_json(const char *dir, const Tabella *tipi, int ntipi)
{
    FILE *f = apri(dir, "soluzioni.json");
    fputs("[\n", f);
    for (int i = 0; i < ntipi; i++) {
        fputs("  ", f);
        tab_stampa_lista(f, &tipi[i]);
        fputs(i + 1 < ntipi ? ",\n" : "\n", f);
    }
    fputs("]\n", f);
    fclose(f);
}

static void scrivi_readme(const char *dir, int n, int T, int ntipi,
                          int scartate, const char *tetto, int json)
{
    FILE *f = apri(dir, "README.md");
    int m = segmenti_mancanti(n, T), m1 = segmenti_mancanti(n, T + 1);
    fprintf(f, "# k%d — census of optimal Kobon arrangements, n = %d\n\n", n, n);
    fprintf(f, "- Lines: **%d**\n", n);
    fprintf(f, "- Maximum number of triangles (OEIS A006066): **%d**\n", T);
    fprintf(f, "- Number of isomorphism types: **%d**\n", ntipi);
    fprintf(f, "- Upper bound check at %d triangles: ", T + 1);
    if (m1 < 0)
        fprintf(f, "impossible by segment counting (%d·%d − 3·%d < 0)\n",
                n, n - 2, T + 1);
    else if (tetto[0] == 'U')
        fprintf(f, "**UNSAT** (SAT model with at most %d segments not on a "
                   "triangle, kissat)\n", m1);
    else
        fprintf(f, "FAILED (%s)\n", tetto);
    fprintf(f, "\n## Files\n\n");
    if (ntipi == 0)
        fprintf(f, "No table files: no simple pseudoline arrangement of %d "
                   "lines has %d triangles (the SAT model is already UNSAT "
                   "at %d). The known optimum %d is therefore reached only "
                   "by non-simple arrangements (triple points or parallel "
                   "lines), outside the scope of this census.\n",
                n, T, T, T);
    else if (json)
        fprintf(f, "`soluzioni.json`: array of the %d canonical tables.\n",
                ntipi);
    else
        fprintf(f, "`tipo1.txt` … `tipo%d.txt`: one canonical table per "
                   "type.\n", ntipi);
    fprintf(f,
        "\nA table has %d rows; row r lists, in order along pseudoline r, "
        "the %d other pseudolines it crosses (lines numbered 1..%d by the "
        "circular order of directions, as in a wiring diagram sorted by "
        "slope). A triple {a,b,c} bounds an empty triangle iff on each of "
        "the three rows the other two appear in adjacent positions.\n",
        n, n - 1, n);
    fprintf(f,
        "\n## Method\n\n"
        "- CNF model of P. Savchuk (arXiv:2507.07951, "
        "https://github.com/zegalur/kobon-cnf), reimplemented from scratch "
        "in C, generalized with free \"missing segment\" variables and a "
        "Sinz cardinality constraint (at most %d finite segments not on a "
        "triangle, i.e. at least %d triangles).\n"
        "- SAT solver: kissat 4.0.4, called repeatedly; after every "
        "solution the whole isomorphism orbit (line relabeling, orientation "
        "reversal, reflection) is blocked, until UNSAT.\n"
        "- Every solution is validated by greedy reconstruction of a wiring "
        "diagram (it must be a genuine pseudoline arrangement) and its "
        "triangle count is checked. Solutions rejected by the validation: "
        "%d.\n"
        "- Canonical form: lexicographic minimum of the orbit.\n",
        m, T, scartate);
    fprintf(f,
        "\n## Scope\n\n"
        "This census covers **simple** arrangements only (no triple points, "
        "no parallel lines) at the **pseudoline** level. Realization with "
        "straight lines in rational coordinates (as in k14–k18) and "
        "degenerate arrangements (triple points, parallels) are future "
        "extensions and are not covered here.\n\n"
        "Produced by `census/census-kobon` in this repository.\n");
    fclose(f);
}

void scrivi_risultati(const char *dir, int n, int T, const Tabella *tipi,
                      int ntipi, int scartate, const char *tetto)
{
    if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "errore: impossibile creare %s\n", dir);
        exit(2);
    }
    pulisci(dir);
    int json = ntipi > SOGLIA_JSON;
    if (json)
        scrivi_json(dir, tipi, ntipi);
    else
        scrivi_tipi(dir, n, T, tipi, ntipi);
    scrivi_readme(dir, n, T, ntipi, scartate, tetto, json);
}
