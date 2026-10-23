/* DFS sui wiring diagram degeneri. Stato = permutazione + coppie già
 * incrociate (con l'indice dell'evento su ciascuna riga); mossa =
 * rovesciare un blocco ADIACENTE di m >= 2 rette a coppie non ancora
 * incrociate (un punto di molteplicità m). In ogni stato è lecito fermarsi
 * se la relazione "mai incrociate" è transitiva: le coppie mai incrociate
 * sono le parallele. Due mosse su blocchi disgiunti commutano: si scarta la
 * mossa il cui blocco sta tutto a sinistra del blocco precedente (le
 * parole così escluse hanno sempre una permutazione equivalente ammessa).
 * Le tabelle con T triangoli si deduplicano con una tabella hash, poi per
 * forma canonica. Codice indipendente dal modello CNF. */
#include "dbruto.h"
#include "dorbita.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    TabellaD *voci;
    int nvoci, capvoci;
    int *slot;          /* indice+1 in voci, 0 = vuoto */
    size_t nslot;       /* potenza di 2 */
} InsiemeD;

static int n_g, T_g, perm_g[N_MAX];
static long long stati_g, foglie_g;
static TabellaD cor_g;
static InsiemeD ins_g;

static uint64_t hash_tab(const TabellaD *t)
{
    uint64_t h = 1469598103934665603ULL;
    for (int r = 0; r < t->n; r++)
        for (int j = 0; j < t->n; j++) {
            h ^= t->pos[r][j];
            h *= 1099511628211ULL;
        }
    return h;
}

static int uguali(const TabellaD *a, const TabellaD *b)
{
    return memcmp(a, b, sizeof *a) == 0;
}

static void rialloca(InsiemeD *s, size_t nslot)
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

static void inserisci(InsiemeD *s, const TabellaD *t)
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
        rialloca(s, s->nslot * 2);
}

/* 1 se la relazione "mai incrociate" è transitiva. */
static int fermabile(void)
{
    int n = n_g;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            if (i == j || cor_g.pos[i][j] != DT_NO) continue;
            for (int k = 0; k < n; k++)
                if (k != i && k != j && cor_g.pos[j][k] == DT_NO &&
                    cor_g.pos[i][k] != DT_NO)
                    return 0;
        }
    return 1;
}

/* Triangoli con il criterio di adiacenza, direttamente sulle posizioni. */
static int triangoli(void)
{
    int n = n_g, c = 0;
    const unsigned char (*p)[N_MAX] = cor_g.pos;
    for (int x = 0; x < n; x++)
        for (int y = x + 1; y < n; y++) {
            if (p[x][y] == DT_NO) continue;
            for (int z = y + 1; z < n; z++) {
                if (p[x][z] == DT_NO || p[y][z] == DT_NO) continue;
                if (p[x][y] == p[x][z]) continue;
                if (abs(p[x][y] - p[x][z]) == 1 &&
                    abs(p[y][x] - p[y][z]) == 1 &&
                    abs(p[z][x] - p[z][y]) == 1)
                    c++;
            }
        }
    return c;
}

static void dfs(int prec)
{
    stati_g++;
    if (fermabile()) {
        foglie_g++;
        if (triangoli() == T_g)
            inserisci(&ins_g, &cor_g);
    }
    int n = n_g;
    for (int p = 0; p + 1 < n; p++)
        for (int m = 2; p + m <= n; m++) {
            /* la nuova retta perm[p+m-1] non deve aver incrociato le altre */
            int nuova = perm_g[p + m - 1], ok = 1;
            for (int q = p; q < p + m - 1 && ok; q++)
                ok = cor_g.pos[nuova][perm_g[q]] == DT_NO;
            if (!ok)
                break;
            if (prec >= 0 && p + m <= prec)
                continue;       /* blocco tutto a sinistra del precedente */
            int blocco[N_MAX];
            for (int q = 0; q < m; q++)
                blocco[q] = perm_g[p + q];
            for (int q = 0; q < m; q++) {
                int v = blocco[q], e = cor_g.nev[v]++;
                for (int w = 0; w < m; w++)
                    if (w != q)
                        cor_g.pos[v][blocco[w]] = (unsigned char)e;
            }
            for (int q = 0; q < m; q++)
                perm_g[p + q] = blocco[m - 1 - q];
            dfs(p);
            for (int q = 0; q < m; q++)
                perm_g[p + q] = blocco[q];
            for (int q = 0; q < m; q++) {
                int v = blocco[q];
                cor_g.nev[v]--;
                for (int w = 0; w < m; w++)
                    if (w != q)
                        cor_g.pos[v][blocco[w]] = DT_NO;
            }
        }
}

static int confronta_q(const void *a, const void *b)
{
    return dtab_confronta(a, b);
}

int dbruto_esegui(int n, int T, EsitoDBruto *e)
{
    memset(e, 0, sizeof *e);
    if (n < 3 || n > DBRUTO_N_MAX)
        return -1;
    n_g = n;
    T_g = T;
    stati_g = foglie_g = 0;
    dtab_vuota(&cor_g, n);
    for (int i = 0; i < n; i++)
        perm_g[i] = i;
    memset(&ins_g, 0, sizeof ins_g);
    rialloca(&ins_g, 1024);
    dfs(-1);

    e->stati = stati_g;
    e->foglie = foglie_g;
    e->con_t = ins_g.nvoci;
    e->canon = malloc((size_t)(ins_g.nvoci + 1) * sizeof *e->canon);
    for (int i = 0; i < ins_g.nvoci; i++) {
        TabellaD c;
        if (dorb_canonica(&ins_g.voci[i], &c) == 0) {
            fprintf(stderr, "errore: tabella della DFS senza wiring\n");
            exit(2);
        }
        int nuova = 1;
        for (int k = 0; k < e->classi && nuova; k++)
            nuova = dtab_confronta(&e->canon[k], &c) != 0;
        if (nuova) {
            e->canon[e->classi++] = c;
            e->semplici += dtab_semplice(&c);
        }
    }
    qsort(e->canon, (size_t)e->classi, sizeof *e->canon, confronta_q);
    free(ins_g.voci);
    free(ins_g.slot);
    memset(&ins_g, 0, sizeof ins_g);
    return 0;
}

void dbruto_libera(EsitoDBruto *e)
{
    free(e->canon);
    e->canon = NULL;
}
