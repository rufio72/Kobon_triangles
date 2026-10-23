/* Costruzione del modello CNF (Savchuk, arXiv:2507.07951), scritto da zero.
 *
 * Variabili, indici 0-based con r, i, j rette distinte e k colonna:
 *   A(r,i,k)  la retta i è la k-esima incontrata lungo r;
 *   G(r,i,j)  j viene subito dopo i lungo r (segmento finito (i,j));
 *   X(r,i,j)  j viene dopo i lungo r;
 *   M(r,i,j)  il segmento (i,j) di r non è lato di un triangolo (solo m>0).
 * La numerazione è densa per blocchi; le combinazioni con indici ripetuti
 * restano variabili inutilizzate, ammesse dal formato DIMACS. */
#include "cnf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int blocco_A(int n) { return n * n * (n - 1); }
static int blocco_3(int n) { return n * n * n; }

int var_A(int n, int r, int i, int k)
{
    return 1 + (r * n + i) * (n - 1) + k;
}

int var_G(int n, int r, int i, int j)
{
    return 1 + blocco_A(n) + (r * n + i) * n + j;
}

int var_X(int n, int r, int i, int j)
{
    return 1 + blocco_A(n) + blocco_3(n) + (r * n + i) * n + j;
}

int var_M(int n, int r, int i, int j)
{
    return 1 + blocco_A(n) + 2 * blocco_3(n) + (r * n + i) * n + j;
}

static void lit(Cnf *c, int l)
{
    if (c->len == c->cap) {
        c->cap = c->cap ? 2 * c->cap : 1 << 16;
        c->lit = realloc(c->lit, c->cap * sizeof *c->lit);
        if (!c->lit) {
            fprintf(stderr, "errore: memoria esaurita\n");
            exit(2);
        }
    }
    c->lit[c->len++] = l;
}

static void chiudi(Cnf *c)
{
    lit(c, 0);
    c->nclausole++;
}

static void cl2(Cnf *c, int a, int b)
{
    lit(c, a); lit(c, b); chiudi(c);
}

static void cl3(Cnf *c, int a, int b, int d)
{
    lit(c, a); lit(c, b); lit(c, d); chiudi(c);
}

void cnf_libera(Cnf *c)
{
    free(c->lit);
    memset(c, 0, sizeof *c);
}

/* Struttura delle righe: clausole 1-10 del brief. */
static void clausole_righe(Cnf *c, int n)
{
    for (int r = 0; r < n; r++)
        for (int i = 0; i < n; i++) {
            if (i == r) continue;
            /* 1. ogni retta compare nella riga; 2. in una sola colonna */
            for (int k = 0; k < n - 1; k++)
                lit(c, var_A(n, r, i, k));
            chiudi(c);
            for (int k = 0; k < n - 1; k++)
                for (int k2 = 0; k2 < n - 1; k2++)
                    if (k2 != k)
                        cl2(c, -var_A(n, r, i, k), -var_A(n, r, i, k2));
            for (int j = 0; j < n; j++) {
                if (j == r || j == i) continue;
                /* 3. legame tra A e G */
                for (int k = 0; k < n - 2; k++) {
                    int a = var_A(n, r, i, k), b = var_A(n, r, j, k + 1);
                    int g = var_G(n, r, i, j);
                    cl3(c, -a, -b, g);
                    cl3(c, -a, -g, b);
                    cl3(c, -g, -b, a);
                }
                /* 4. successore unico */
                for (int k = 0; k < n; k++)
                    if (k != r && k != i && k != j)
                        cl2(c, -var_G(n, r, i, j), -var_G(n, r, i, k));
                /* 7. X totale e antisimmetrica */
                if (i < j) {
                    cl2(c, var_X(n, r, i, j), var_X(n, r, j, i));
                    cl2(c, -var_X(n, r, i, j), -var_X(n, r, j, i));
                }
                /* 8. transitività */
                for (int k = 0; k < n; k++)
                    if (k != r && k != i && k != j)
                        cl3(c, -var_X(n, r, i, j), -var_X(n, r, j, k),
                            var_X(n, r, i, k));
                /* 10. G implica X */
                cl2(c, -var_G(n, r, i, j), var_X(n, r, i, j));
            }
            /* 5. primo; 6. ultimo; 9. primo e ultimo tramite X */
            int a0 = var_A(n, r, i, 0), au = var_A(n, r, i, n - 2);
            for (int j = 0; j < n; j++) {
                if (j == r || j == i) continue;
                cl2(c, -a0, -var_G(n, r, j, i));
                cl2(c, -au, -var_G(n, r, i, j));
                cl2(c, -a0, var_X(n, r, i, j));
                cl2(c, -au, var_X(n, r, j, i));
            }
            lit(c, a0);
            for (int j = 0; j < n; j++)
                if (j != r && j != i) lit(c, var_G(n, r, j, i));
            chiudi(c);
            lit(c, au);
            for (int j = 0; j < n; j++)
                if (j != r && j != i) lit(c, var_G(n, r, i, j));
            chiudi(c);
            for (int j = 0; j < n; j++)
                if (j != r && j != i) lit(c, -var_X(n, r, i, j));
            lit(c, a0);
            chiudi(c);
            for (int j = 0; j < n; j++)
                if (j != r && j != i) lit(c, -var_X(n, r, j, i));
            lit(c, au);
            chiudi(c);
        }
}

/* Compagni (r1,i1,j1) e (r2,i2,j2) della relazione (r,a,b): secondo
 * l'ordine dei tre indici, le coppie che chiudono il triangolo {r,a,b}
 * (tabella dei sei casi, prima colonna con i=a, j=b). */
static void compagni(int r, int a, int b, int t1[3], int t2[3])
{
#define T(v, x, y, z) (v[0] = (x), v[1] = (y), v[2] = (z))
    if (r < a && a < b)      { T(t1, a, r, b); T(t2, b, r, a); }
    else if (r < b && b < a) { T(t1, b, a, r); T(t2, a, b, r); }
    else if (a < r && r < b) { T(t1, a, r, b); T(t2, b, a, r); }
    else if (b < r && r < a) { T(t1, b, a, r); T(t2, a, r, b); }
    else if (a < b && b < r) { T(t1, a, b, r); T(t2, b, a, r); }
    else                     { T(t1, b, r, a); T(t2, a, r, b); }
#undef T
}

/* 11. Chiusura dei triangoli (G con via d'uscita M) e coerenza delle
 * terne (X, senza M). */
static void clausole_triangoli(Cnf *c, int n, int m)
{
    for (int r = 0; r < n; r++)
        for (int a = 0; a < n; a++)
            for (int b = 0; b < n; b++) {
                if (a == r || b == r || a == b) continue;
                int p[3], q[3];
                compagni(r, a, b, p, q);
                int g = var_G(n, r, a, b);
                int g1 = var_G(n, p[0], p[1], p[2]);
                int g2 = var_G(n, q[0], q[1], q[2]);
                int mm = m > 0 ? var_M(n, r, a, b) : 0;
                lit(c, -g); lit(c, g1); if (mm) lit(c, mm); chiudi(c);
                lit(c, -g); lit(c, g2); if (mm) lit(c, mm); chiudi(c);
                lit(c, g); lit(c, -g1); lit(c, -g2); if (mm) lit(c, mm);
                chiudi(c);
                int x = var_X(n, r, a, b);
                int x1 = var_X(n, p[0], p[1], p[2]);
                int x2 = var_X(n, q[0], q[1], q[2]);
                cl2(c, -x, x1);
                cl2(c, -x, x2);
                cl3(c, x, -x1, -x2);
            }
}

/* 12. M implica G; al più m variabili M vere (contatore sequenziale di
 * Sinz, variabili ausiliarie s(v,c) dopo tutte le altre). */
static void clausole_mancanti(Cnf *c, int n, int m)
{
    int V = n * (n - 1) * (n - 2);
    int *x = malloc((size_t)V * sizeof *x);
    int nv = 0;
    for (int r = 0; r < n; r++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                if (i == r || j == r || i == j) continue;
                cl2(c, -var_M(n, r, i, j), var_G(n, r, i, j));
                x[nv++] = var_M(n, r, i, j);
            }
    int base = c->nvar;
    /* s(v,k) con v = 1..V-1, k = 1..m */
#define S(v, k) (base + ((v) - 1) * m + (k))
    c->nvar = base + (V - 1) * m;
    cl2(c, -x[0], S(1, 1));
    for (int k = 2; k <= m; k++) {
        lit(c, -S(1, k));
        chiudi(c);
    }
    for (int v = 2; v <= V - 1; v++) {
        cl2(c, -x[v - 1], S(v, 1));
        cl2(c, -S(v - 1, 1), S(v, 1));
        for (int k = 2; k <= m; k++) {
            cl3(c, -x[v - 1], -S(v - 1, k - 1), S(v, k));
            cl2(c, -S(v - 1, k), S(v, k));
        }
        cl2(c, -x[v - 1], -S(v - 1, m));
    }
    cl2(c, -x[V - 1], -S(V - 1, m));
#undef S
    free(x);
}

void cnf_kobon(Cnf *c, int n, int m)
{
    memset(c, 0, sizeof *c);
    c->n = n;
    c->nvar = blocco_A(n) + 2 * blocco_3(n) + (m > 0 ? blocco_3(n) : 0);
    clausole_righe(c, n);
    clausole_triangoli(c, n, m);
    if (m > 0)
        clausole_mancanti(c, n, m);
}

void cnf_blocca(Cnf *c, const Tabella *t)
{
    int n = c->n;
    for (int r = 0; r < n; r++)
        for (int k = 0; k < n - 1; k++)
            lit(c, -var_A(n, r, t->r[r][k] - 1, k));
    chiudi(c);
}

int cnf_scrivi(const Cnf *c, const char *percorso)
{
    FILE *f = fopen(percorso, "w");
    if (!f)
        return -1;
    fprintf(f, "c modello di Kobon per %d rette (census-kobon)\n", c->n);
    fprintf(f, "p cnf %d %ld\n", c->nvar, c->nclausole);
    for (size_t p = 0; p < c->len; p++) {
        if (c->lit[p] == 0)
            fputs("0\n", f);
        else
            fprintf(f, "%d ", c->lit[p]);
    }
    return fclose(f) == 0 ? 0 : -1;
}
