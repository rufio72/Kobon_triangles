/* DFS su tutte le parole ridotte della permutazione più lunga: ogni foglia
 * è un wiring diagram, cioè un arrangiamento semplice di pseudorette con le
 * rette numerate 1..n dall'alto in basso a sinistra. Le tabelle degli
 * ordini vengono deduplicate con una tabella hash. */
#include "bruto.h"
#include "wiring.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Tabella *voci;
    int nvoci, capvoci;
    int *slot;          /* indice+1 in voci, 0 = vuoto */
    size_t nslot;       /* potenza di 2 */
} Insieme;

static int n_g, totale_g, len_g[N_MAX + 1], perm_g[N_MAX];
static long long foglie_g;
static Tabella cor_g;
static Insieme ins_g;

static uint64_t hash_tab(const Tabella *t)
{
    const unsigned char *p = &t->r[0][0];
    size_t len = (size_t)t->n * (N_MAX - 1);
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= 1099511628211ULL;
    }
    return h;
}

static int uguali(const Tabella *a, const Tabella *b)
{
    return memcmp(a->r, b->r, (size_t)a->n * (N_MAX - 1)) == 0;
}

static void rialloca_slot(Insieme *s, size_t nslot)
{
    free(s->slot);
    s->nslot = nslot;
    s->slot = calloc(nslot, sizeof *s->slot);
    if (!s->slot) {
        fprintf(stderr, "errore: memoria esaurita\n");
        exit(2);
    }
    for (int i = 0; i < s->nvoci; i++) {
        size_t h = hash_tab(&s->voci[i]) & (nslot - 1);
        while (s->slot[h])
            h = (h + 1) & (nslot - 1);
        s->slot[h] = i + 1;
    }
}

static void inserisci(Insieme *s, const Tabella *t)
{
    size_t h = hash_tab(t) & (s->nslot - 1);
    while (s->slot[h]) {
        if (uguali(&s->voci[s->slot[h] - 1], t))
            return;
        h = (h + 1) & (s->nslot - 1);
    }
    if (s->nvoci == s->capvoci) {
        s->capvoci = s->capvoci ? 2 * s->capvoci : 1024;
        s->voci = realloc(s->voci, (size_t)s->capvoci * sizeof *s->voci);
        if (!s->voci) {
            fprintf(stderr, "errore: memoria esaurita\n");
            exit(2);
        }
    }
    s->voci[s->nvoci++] = *t;
    s->slot[h] = s->nvoci;
    if ((size_t)s->nvoci * 2 > s->nslot)
        rialloca_slot(s, s->nslot * 2);
}

static void dfs(int fatti)
{
    if (fatti == totale_g) {
        foglie_g++;
        inserisci(&ins_g, &cor_g);
        return;
    }
    for (int p = 0; p + 1 < n_g; p++) {
        int u = perm_g[p], v = perm_g[p + 1];
        if (u > v)
            continue;   /* coppia già incrociata */
        cor_g.r[u - 1][len_g[u]++] = (unsigned char)v;
        cor_g.r[v - 1][len_g[v]++] = (unsigned char)u;
        perm_g[p] = v;
        perm_g[p + 1] = u;
        dfs(fatti + 1);
        perm_g[p] = u;
        perm_g[p + 1] = v;
        len_g[u]--;
        len_g[v]--;
    }
}

static int confronta_q(const void *a, const void *b)
{
    return tab_confronta(a, b);
}

int bruto_esegui(int n, int T, EsitoBruto *e)
{
    memset(e, 0, sizeof *e);
    if (n < 3 || n > BRUTO_N_MAX)
        return -1;
    n_g = n;
    totale_g = n * (n - 1) / 2;
    foglie_g = 0;
    tab_vuota(&cor_g, n);
    for (int i = 0; i < n; i++) {
        perm_g[i] = i + 1;
        len_g[i + 1] = 0;
    }
    memset(&ins_g, 0, sizeof ins_g);
    rialloca_slot(&ins_g, 1024);
    dfs(0);

    e->parole = foglie_g;
    e->distinte = ins_g.nvoci;
    e->canon = malloc((size_t)(ins_g.nvoci + 1) * sizeof *e->canon);
    for (int i = 0; i < ins_g.nvoci; i++) {
        const Tabella *t = &ins_g.voci[i];
        if (!wiring_valida(t))
            e->non_valide++;
        if (tab_triangoli(t) != T)
            continue;
        e->con_tmax++;
        Tabella c;
        tab_canonica(t, &c);
        int nuovo = 1;
        for (int k = 0; k < e->classi && nuovo; k++)
            if (tab_confronta(&e->canon[k], &c) == 0)
                nuovo = 0;
        if (nuovo)
            e->canon[e->classi++] = c;
    }
    qsort(e->canon, (size_t)e->classi, sizeof *e->canon, confronta_q);
    free(ins_g.voci);
    free(ins_g.slot);
    memset(&ins_g, 0, sizeof ins_g);
    return 0;
}

void bruto_libera(EsitoBruto *e)
{
    free(e->canon);
    e->canon = NULL;
}
