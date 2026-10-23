/* Modello CNF degenere (compito 02), scritto da zero.
 *
 * Variabili (rette 0-based, indici distinti):
 *   X(r,i,j)  r incrocia i e j, l'evento di j è strettamente dopo quello
 *             di i lungo r;
 *   T(a,b,c)  a, b, c per un punto comune (indici ordinati);
 *   P(i,j)    i e j parallele (indici ordinati);
 *   adj(r,i,j) eventi di i e j consecutivi lungo r (i<j);
 *   tri(a,b,c) la terna è un triangolo;
 *   b1/b2(r,i,j,d) d strettamente fra i e j lungo r (i<j), nei due versi;
 *   ausiliarie dei contatori di Sinz, dopo tutte le altre.
 * Numerazione densa per blocchi; le combinazioni con indici ripetuti o non
 * ordinati restano variabili inutilizzate. */
#include "dcnf.h"

#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

static int n3(int n) { return n * n * n; }

int dv_X(int n, int r, int i, int j) { return 1 + (r * n + i) * n + j; }

static void ordina3(int *a, int *b, int *c)
{
    int t;
    if (*a > *b) { t = *a; *a = *b; *b = t; }
    if (*b > *c) { t = *b; *b = *c; *c = t; }
    if (*a > *b) { t = *a; *a = *b; *b = t; }
}

int dv_T(int n, int a, int b, int c)
{
    ordina3(&a, &b, &c);
    return 1 + n3(n) + (a * n + b) * n + c;
}

int dv_P(int n, int i, int j)
{
    if (i > j) { int t = i; i = j; j = t; }
    return 1 + 2 * n3(n) + i * n + j;
}

int dv_adj(int n, int r, int i, int j)
{
    if (i > j) { int t = i; i = j; j = t; }
    return 1 + 2 * n3(n) + n * n + (r * n + i) * n + j;
}

int dv_tri(int n, int a, int b, int c)
{
    ordina3(&a, &b, &c);
    return 1 + 3 * n3(n) + n * n + (a * n + b) * n + c;
}

int dv_b1(int n, int r, int i, int j, int d)
{
    return 1 + 4 * n3(n) + n * n + ((r * n + i) * n + j) * n + d;
}

int dv_b2(int n, int r, int i, int j, int d)
{
    return 1 + 4 * n3(n) + n * n + n3(n) * n + ((r * n + i) * n + j) * n + d;
}

static int nvar_base(int n) { return 4 * n3(n) + n * n + 2 * n3(n) * n; }

/* ---------- scrittura delle clausole ---------- */

static void *cresci(void *p, size_t elem, size_t nuova)
{
    p = realloc(p, elem * nuova);
    if (!p) {
        fprintf(stderr, "errore: memoria esaurita\n");
        exit(2);
    }
    return p;
}

static int gruppo_corrente;

static void lit(DCnf *c, int l)
{
    if (c->len == c->cap) {
        c->cap = c->cap ? 2 * c->cap : 1 << 16;
        c->lit = cresci(c->lit, sizeof *c->lit, c->cap);
    }
    c->lit[c->len++] = l;
}

static void chiudi(DCnf *c)
{
    lit(c, 0);
    if (c->nclausole == c->capg) {
        c->capg = c->capg ? 2 * c->capg : 1 << 14;
        c->gruppo = cresci(c->gruppo, 1, (size_t)c->capg);
    }
    c->gruppo[c->nclausole++] = (unsigned char)gruppo_corrente;
}

static void cl(DCnf *c, int k, const int *v)
{
    for (int q = 0; q < k; q++)
        lit(c, v[q]);
    chiudi(c);
}

#define CL(c, ...) do { int v_[] = {__VA_ARGS__}; \
        cl(c, (int)(sizeof v_ / sizeof *v_), v_); } while (0)

void dcnf_libera(DCnf *c)
{
    free(c->lit);
    free(c->gruppo);
    for (int s = 0; s < c->nsinz; s++)
        free(c->sinz[s].x);
    memset(c, 0, sizeof *c);
}

/* Contatore sequenziale di Sinz: al più m letterali veri fra x[0..V-1]. */
static void sinz(DCnf *c, const int *x, int V, int m)
{
    if (m < 0) {
        chiudi(c);                      /* clausola vuota */
        return;
    }
    if (m >= V)
        return;
    if (m == 0) {
        for (int v = 0; v < V; v++)
            CL(c, -x[v]);
        return;
    }
    SinzD *s = &c->sinz[c->nsinz++];
    s->base = c->nvar;
    s->V = V;
    s->m = m;
    s->x = malloc((size_t)V * sizeof *s->x);
    memcpy(s->x, x, (size_t)V * sizeof *x);
    int base = c->nvar;
#define S(v, k) (base + ((v) - 1) * m + (k))
    c->nvar = base + (V - 1) * m;
    CL(c, -x[0], S(1, 1));
    for (int k = 2; k <= m; k++)
        CL(c, -S(1, k));
    for (int v = 2; v <= V - 1; v++) {
        CL(c, -x[v - 1], S(v, 1));
        CL(c, -S(v - 1, 1), S(v, 1));
        for (int k = 2; k <= m; k++) {
            CL(c, -x[v - 1], -S(v - 1, k - 1), S(v, k));
            CL(c, -S(v - 1, k), S(v, k));
        }
        CL(c, -x[v - 1], -S(v - 1, m));
    }
    CL(c, -x[V - 1], -S(V - 1, m));
#undef S
}

/* Compagni della relazione (r,a,b) secondo l'ordine dei tre indici
 * (tabella dei sei casi, come nel compito 01). */
static void compagni(int r, int a, int b, int t1[3], int t2[3])
{
#define TT(v, x, y, z) (v[0] = (x), v[1] = (y), v[2] = (z))
    if (r < a && a < b)      { TT(t1, a, r, b); TT(t2, b, r, a); }
    else if (r < b && b < a) { TT(t1, b, a, r); TT(t2, a, b, r); }
    else if (a < r && r < b) { TT(t1, a, r, b); TT(t2, b, a, r); }
    else if (b < r && r < a) { TT(t1, b, a, r); TT(t2, a, r, b); }
    else if (a < b && b < r) { TT(t1, a, b, r); TT(t2, b, a, r); }
    else                     { TT(t1, b, r, a); TT(t2, a, r, b); }
#undef TT
}

/* D1-D6: parallele, ordine sulle righe, punti multipli. */
static void clausole_struttura(DCnf *c, int n)
{
    gruppo_corrente = G_D1;
    for (int i = 0; i < n; i++)
        for (int k = i + 1; k < n; k++)
            for (int j = k + 1; j < n; j++) {
                CL(c, -dv_P(n, i, j), dv_P(n, i, k));
                CL(c, -dv_P(n, i, j), dv_P(n, k, j));
            }
    for (int r = 0; r < n; r++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                if (r == i || r == j || i == j) continue;
                int x = dv_X(n, r, i, j);
                gruppo_corrente = G_D2;
                CL(c, -x, -dv_P(n, r, i));
                CL(c, -x, -dv_P(n, r, j));
                if (i < j) {
                    int y = dv_X(n, r, j, i), t = dv_T(n, r, i, j);
                    gruppo_corrente = G_D3;
                    CL(c, x, y, t, dv_P(n, r, i), dv_P(n, r, j));
                    CL(c, -x, -y);
                    CL(c, -x, -t);
                    CL(c, -y, -t);
                }
                gruppo_corrente = G_D5;
                for (int k = 0; k < n; k++) {
                    if (k == r || k == i || k == j) continue;
                    int xik = dv_X(n, r, i, k);
                    CL(c, -x, -dv_X(n, r, j, k), xik);
                    CL(c, -dv_T(n, r, i, j), -dv_X(n, r, j, k), xik);
                    CL(c, -x, -dv_T(n, r, j, k), xik);
                }
            }
    gruppo_corrente = G_D4;
    for (int a = 0; a < n; a++)
        for (int b = a + 1; b < n; b++)
            for (int d = b + 1; d < n; d++) {
                int t = dv_T(n, a, b, d);
                CL(c, -t, -dv_P(n, a, b));
                CL(c, -t, -dv_P(n, a, d));
                CL(c, -t, -dv_P(n, b, d));
            }
    /* D6: per ogni quaterna e ogni coppia condivisa {x,y} con gli altri
     * due u, v: T(x,y,u) e T(x,y,v) implicano T(x,u,v) e T(y,u,v). */
    gruppo_corrente = G_D6;
    for (int a = 0; a < n; a++)
        for (int b = a + 1; b < n; b++)
            for (int d = b + 1; d < n; d++)
                for (int e = d + 1; e < n; e++) {
                    int q[4] = {a, b, d, e};
                    for (int p1 = 0; p1 < 4; p1++)
                        for (int p2 = p1 + 1; p2 < 4; p2++) {
                            int o[2], no = 0;
                            for (int z = 0; z < 4; z++)
                                if (z != p1 && z != p2) o[no++] = q[z];
                            int x = q[p1], y = q[p2], u = o[0], v = o[1];
                            int t1 = dv_T(n, x, y, u), t2 = dv_T(n, x, y, v);
                            CL(c, -t1, -t2, dv_T(n, x, u, v));
                            CL(c, -t1, -t2, dv_T(n, y, u, v));
                        }
                }
}

/* D7: coerenza delle terne con la convenzione di numerazione, con guardia. */
static void clausole_terne(DCnf *c, int n)
{
    gruppo_corrente = G_D7;
    for (int r = 0; r < n; r++)
        for (int a = 0; a < n; a++)
            for (int b = 0; b < n; b++) {
                if (a == r || b == r || a == b) continue;
                int p[3], q[3];
                compagni(r, a, b, p, q);
                int x = dv_X(n, r, a, b);
                int x1 = dv_X(n, p[0], p[1], p[2]);
                int x2 = dv_X(n, q[0], q[1], q[2]);
                int g1 = dv_P(n, r, a), g2 = dv_P(n, r, b);
                int g3 = dv_P(n, a, b), g4 = dv_T(n, r, a, b);
                CL(c, -x, x1, g1, g2, g3, g4);
                CL(c, -x, x2, g1, g2, g3, g4);
                CL(c, x, -x1, -x2, g1, g2, g3, g4);
            }
}

/* D8-D9: adiacenza e triangoli. */
static void clausole_triangoli(DCnf *c, int n)
{
    for (int r = 0; r < n; r++)
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++) {
                if (i == r || j == r) continue;
                gruppo_corrente = G_D8;
                int ad = dv_adj(n, r, i, j);
                int xij = dv_X(n, r, i, j), xji = dv_X(n, r, j, i);
                CL(c, -ad, xij, xji);
                for (int d = 0; d < n; d++) {
                    if (d == r || d == i || d == j) continue;
                    int b1 = dv_b1(n, r, i, j, d), b2 = dv_b2(n, r, i, j, d);
                    int xid = dv_X(n, r, i, d), xdj = dv_X(n, r, d, j);
                    int xjd = dv_X(n, r, j, d), xdi = dv_X(n, r, d, i);
                    CL(c, -b1, xid);
                    CL(c, -b1, xdj);
                    CL(c, b1, -xid, -xdj);
                    CL(c, -b2, xjd);
                    CL(c, -b2, xdi);
                    CL(c, b2, -xjd, -xdi);
                    CL(c, -ad, -b1);
                    CL(c, -ad, -b2);
                }
                for (int verso = 0; verso < 2; verso++) {
                    lit(c, ad);
                    lit(c, verso ? -xji : -xij);
                    for (int d = 0; d < n; d++) {
                        if (d == r || d == i || d == j) continue;
                        lit(c, dv_b1(n, r, i, j, d));
                        lit(c, dv_b2(n, r, i, j, d));
                    }
                    chiudi(c);
                }
            }
    gruppo_corrente = G_D9;
    for (int a = 0; a < n; a++)
        for (int b = a + 1; b < n; b++)
            for (int d = b + 1; d < n; d++) {
                int t = dv_tri(n, a, b, d);
                int a1 = dv_adj(n, a, b, d), a2 = dv_adj(n, b, a, d);
                int a3 = dv_adj(n, d, a, b);
                CL(c, -t, a1);
                CL(c, -t, a2);
                CL(c, -t, a3);
                CL(c, t, -a1, -a2, -a3);
            }
}

void dcnf_costruisci(DCnf *c, int n, int T, int modo)
{
    memset(c, 0, sizeof *c);
    c->n = n;
    c->nvar = nvar_base(n);
    clausole_struttura(c, n);
    clausole_terne(c, n);
    clausole_triangoli(c, n);
    /* D10: cardinalità sui tri. */
    gruppo_corrente = G_D10;
    int C3 = n * (n - 1) * (n - 2) / 6, V = 0;
    int *x = malloc((size_t)(C3 + 1) * sizeof *x);
    for (int a = 0; a < n; a++)
        for (int b = a + 1; b < n; b++)
            for (int d = b + 1; d < n; d++)
                x[V++] = dv_tri(n, a, b, d);
    if (modo != DCNF_UNSAT)
        sinz(c, x, V, T);
    for (int v = 0; v < V; v++)
        x[v] = -x[v];
    sinz(c, x, V, C3 - T);
    free(x);
    /* D11: almeno una degenerazione (solo nel censimento). */
    if (modo == DCNF_CENSIMENTO) {
        gruppo_corrente = G_D11;
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                lit(c, dv_P(n, i, j));
        for (int a = 0; a < n; a++)
            for (int b = a + 1; b < n; b++)
                for (int d = b + 1; d < n; d++)
                    lit(c, dv_T(n, a, b, d));
        chiudi(c);
    }
}

void dcnf_parte(DCnf *c, int qmin, int qmax)
{
    int n = c->n, C3 = n * (n - 1) * (n - 2) / 6, V = 0;
    int *x = malloc((size_t)(C3 + 1) * sizeof *x);
    gruppo_corrente = G_D10;
    for (int a = 0; a < n; a++)
        for (int b = a + 1; b < n; b++)
            for (int d = b + 1; d < n; d++)
                x[V++] = dv_T(n, a, b, d);
    if (qmax >= 0)
        sinz(c, x, V, qmax);
    for (int v = 0; v < V; v++)
        x[v] = -x[v];
    sinz(c, x, V, C3 - qmin);
    free(x);
}

/* Valori delle variabili strutturali indotti dalla tabella t. */
static void assegna_struttura(const TabellaD *t, signed char *val)
{
    int n = t->n;
#define ASSEGNA(v, b) (val[v] = (b) ? 1 : -1)
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            ASSEGNA(dv_P(n, i, j), dtab_parallele(t, i, j));
    for (int a = 0; a < n; a++)
        for (int b = a + 1; b < n; b++)
            for (int d = b + 1; d < n; d++)
                ASSEGNA(dv_T(n, a, b, d), t->pos[a][b] != DT_NO &&
                  t->pos[a][d] != DT_NO && t->pos[b][d] != DT_NO &&
                  t->pos[a][b] == t->pos[a][d]);
    for (int r = 0; r < n; r++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (r != i && r != j && i != j)
                    ASSEGNA(dv_X(n, r, i, j), t->pos[r][i] != DT_NO &&
                      t->pos[r][j] != DT_NO && t->pos[r][i] < t->pos[r][j]);
}

void dcnf_blocca(DCnf *c, const TabellaD *t)
{
    int n = c->n;
    signed char *val = calloc((size_t)nvar_base(n) + 1, 1);
    assegna_struttura(t, val);
    gruppo_corrente = G_D12;
    for (int v = 1; v <= 2 * n3(n) + n * n; v++)
        if (val[v] > 0)
            lit(c, -v);
    chiudi(c);
    free(val);
}

int dcnf_scrivi(const DCnf *c, const char *percorso)
{
    FILE *f = fopen(percorso, "w");
    if (!f)
        return -1;
    fprintf(f, "c modello degenere di Kobon per %d rette (census-degeneri)\n",
            c->n);
    fprintf(f, "p cnf %d %ld\n", c->nvar, c->nclausole);
    for (size_t p = 0; p < c->len; p++) {
        if (c->lit[p] == 0)
            fputs("0\n", f);
        else
            fprintf(f, "%d ", c->lit[p]);
    }
    return fclose(f) == 0 ? 0 : -1;
}

int dcnf_estrai(int n, const signed char *val, TabellaD *t)
{
    dtab_vuota(t, n);
    for (int r = 0; r < n; r++) {
        int cnt[N_MAX], usato[N_MAX] = {0};
        for (int i = 0; i < n; i++) {
            cnt[i] = -1;
            if (i == r || val[dv_P(n, r, i)] > 0)
                continue;
            cnt[i] = 0;
            for (int j = 0; j < n; j++)
                if (j != r && j != i && val[dv_X(n, r, j, i)] > 0)
                    cnt[i]++;
            usato[cnt[i]] = 1;
        }
        /* Posizioni uguali = stesso evento; si comprimono in 0..nev-1. */
        int rango[N_MAX], ne = 0;
        for (int p = 0; p < n; p++)
            rango[p] = usato[p] ? ne++ : -1;
        t->nev[r] = (unsigned char)ne;
        for (int i = 0; i < n; i++)
            if (cnt[i] >= 0)
                t->pos[r][i] = (unsigned char)rango[cnt[i]];
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++) {
                if (cnt[i] < 0 || cnt[j] < 0) continue;
                if ((cnt[i] == cnt[j]) != (val[dv_T(n, r, i, j)] > 0))
                    return -1;
            }
    }
    return dtab_ben_formata(t) ? 0 : -1;
}

/* Nome leggibile di una variabile, per la diagnostica. */
static void nome_var(int n, int v, char *buf, size_t len)
{
    int N3 = n3(n), u = v - 1;
    const char *nomi[] = {"X", "T", "P", "adj", "tri"};
    int inizio[] = {0, N3, 2 * N3, 2 * N3 + n * n, 3 * N3 + n * n,
                    4 * N3 + n * n};
    for (int b = 0; b < 5; b++)
        if (u >= inizio[b] && u < inizio[b + 1]) {
            int w = u - inizio[b];
            if (b == 2)
                snprintf(buf, len, "P(%d,%d)", w / n, w % n);
            else
                snprintf(buf, len, "%s(%d,%d,%d)", nomi[b], w / (n * n),
                         w / n % n, w % n);
            return;
        }
    u -= 4 * N3 + n * n;
    if (u < 2 * N3 * n) {
        int w = u % (N3 * n);
        snprintf(buf, len, "b%d(%d,%d,%d,%d)", u < N3 * n ? 1 : 2,
                 w / N3, w / (n * n) % n, w / n % n, w % n);
        return;
    }
    snprintf(buf, len, "s%d", v);
}

long dcnf_controlla(const TabellaD *t, int T, FILE *rapporto)
{
    int n = t->n;
    DCnf c;
    dcnf_costruisci(&c, n, T, DCNF_CONTROLLA);
    signed char *val = calloc((size_t)c.nvar + 1, 1);
    assegna_struttura(t, val);
    for (int r = 0; r < n; r++)
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++) {
                if (r == i || r == j) continue;
                int pi = t->pos[r][i], pj = t->pos[r][j];
                int inc = pi != DT_NO && pj != DT_NO;
                ASSEGNA(dv_adj(n, r, i, j), inc && abs(pi - pj) == 1);
                for (int d = 0; d < n; d++) {
                    if (d == r || d == i || d == j) continue;
                    int pd = t->pos[r][d];
                    int okd = inc && pd != DT_NO;
                    ASSEGNA(dv_b1(n, r, i, j, d), okd && pi < pd && pd < pj);
                    ASSEGNA(dv_b2(n, r, i, j, d), okd && pj < pd && pd < pi);
                }
            }
    for (int a = 0; a < n; a++)
        for (int b = a + 1; b < n; b++)
            for (int d = b + 1; d < n; d++)
                ASSEGNA(dv_tri(n, a, b, d), val[dv_adj(n, a, b, d)] > 0 &&
                  val[dv_adj(n, b, a, d)] > 0 && val[dv_adj(n, d, a, b)] > 0);
    for (int s = 0; s < c.nsinz; s++) {
        const SinzD *z = &c.sinz[s];
        int cnt = 0;
        for (int v = 1; v <= z->V - 1; v++) {
            int l = z->x[v - 1];
            cnt += (l > 0 ? val[l] : -val[-l]) > 0;
            for (int k = 1; k <= z->m; k++)
                ASSEGNA(z->base + (v - 1) * z->m + k, cnt >= k);
        }
    }
#undef ASSEGNA
    long violate = 0, pergruppo[G_NUM] = {0}, ic = 0;
    size_t inizio = 0;
    for (size_t p = 0; p < c.len; p++) {
        if (c.lit[p] != 0)
            continue;
        int sodd = 0;
        for (size_t q = inizio; q < p && !sodd; q++) {
            int l = c.lit[q];
            sodd = (l > 0 ? val[l] : -val[-l]) > 0;
        }
        if (!sodd) {
            violate++;
            pergruppo[c.gruppo[ic]]++;
            if (rapporto && violate <= 10) {
                fprintf(rapporto, "  violata (D%d):", c.gruppo[ic]);
                for (size_t q = inizio; q < p; q++) {
                    char nome[64];
                    int l = c.lit[q];
                    nome_var(n, abs(l), nome, sizeof nome);
                    fprintf(rapporto, " %s%s", l < 0 ? "-" : "", nome);
                }
                fputc('\n', rapporto);
            }
        }
        inizio = p + 1;
        ic++;
    }
    if (rapporto) {
        fprintf(rapporto, "controlla: n=%d T=%d, %ld clausole, %ld violate",
                n, T, c.nclausole, violate);
        for (int g = 1; g < G_NUM; g++)
            if (pergruppo[g])
                fprintf(rapporto, " [D%d: %ld]", g, pergruppo[g]);
        fputc('\n', rapporto);
    }
    free(val);
    dcnf_libera(&c);
    return violate;
}

int dcnf_risolvi(const char *kissat, const char *cnf_path,
                 const char *out_path, int nvar, signed char *val,
                 long limite)
{
    char cmd[4096], tempo[64] = "";
    if (limite > 0)
        snprintf(tempo, sizeof tempo, " --time=%ld", limite);
    snprintf(cmd, sizeof cmd, "'%s' -q%s '%s' > '%s' 2>&1", kissat, tempo,
             cnf_path, out_path);
    int st = system(cmd);
    if (st == -1 || !WIFEXITED(st)) {
        fprintf(stderr, "errore: impossibile eseguire kissat\n");
        return -1;
    }
    int codice = WEXITSTATUS(st);
    if (codice == 0 && limite > 0)
        return 0;
    if (codice != 10 && codice != 20) {
        fprintf(stderr, "errore: kissat è uscito con codice %d (vedi %s)\n",
                codice, out_path);
        return -1;
    }
    FILE *f = fopen(out_path, "r");
    if (!f)
        return -1;
    static char riga[1 << 16];
    memset(val, 0, (size_t)nvar + 1);
    int stato = 0;
    while (fgets(riga, sizeof riga, f)) {
        if (riga[0] == 's')
            stato = strstr(riga, "UNSATISFIABLE") ? 20 : 10;
        else if (riga[0] == 'v') {
            char *p = riga + 1, *fine;
            for (;;) {
                long v = strtol(p, &fine, 10);
                if (fine == p || v == 0)
                    break;
                long a = v < 0 ? -v : v;
                if (a <= nvar)
                    val[a] = v > 0 ? 1 : -1;
                p = fine;
            }
        }
    }
    fclose(f);
    return stato == codice ? stato : -1;
}
