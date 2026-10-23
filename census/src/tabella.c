/* Operazioni sulla tabella degli ordini: gruppo di isomorfismo, forma
 * canonica, conteggio dei triangoli, lettura e scrittura. */
#include "tabella.h"

#include <stdlib.h>
#include <string.h>

void tab_vuota(Tabella *t, int n)
{
    memset(t, 0, sizeof *t);
    t->n = n;
}

void tab_reindex(const Tabella *t, int f, Tabella *out)
{
    int n = t->n;
    Tabella tmp;
    tab_vuota(&tmp, n);
    /* Rinumerazione x -> (n + x - f) mod n + 1; le righe delle rette che
     * precedono f vengono rovesciate. */
    for (int l = 0; l < n; l++) {
        for (int k = 0; k < n - 1; k++) {
            int x = t->r[l][k];
            int y = (n + x - f) % n + 1;
            int kk = (l + 1 < f) ? (n - 2 - k) : k;
            tmp.r[l][kk] = (unsigned char)y;
        }
    }
    /* Rotazione delle righe: la riga f-1 diventa la prima. */
    tab_vuota(out, n);
    for (int q = 0; q < n; q++)
        memcpy(out->r[q], tmp.r[(q + f - 1) % n], (size_t)(n - 1));
}

/* Indice della retta l (1-based) dopo l'inversione del verso. */
static int idx_inverso(int n, int l)
{
    return l == 1 ? 1 : n - l + 2;
}

void tab_inverti(const Tabella *t, Tabella *out)
{
    int n = t->n;
    tab_vuota(out, n);
    for (int r = 0; r < n; r++) {
        if (r == 0) {
            for (int k = 0; k < n - 1; k++)
                out->r[0][k] = (unsigned char)idx_inverso(n, t->r[0][k]);
        } else {
            int src = idx_inverso(n, r + 1) - 1;
            for (int k = 0; k < n - 1; k++)
                out->r[r][k] =
                    (unsigned char)idx_inverso(n, t->r[src][n - 2 - k]);
        }
    }
}

int tab_confronta(const Tabella *a, const Tabella *b)
{
    int n = a->n;
    for (int r = 0; r < n; r++)
        for (int k = 0; k < n - 1; k++)
            if (a->r[r][k] != b->r[r][k])
                return (int)a->r[r][k] - (int)b->r[r][k];
    return 0;
}

static int presente(const Tabella *orb, int cnt, const Tabella *t)
{
    for (int i = 0; i < cnt; i++)
        if (tab_confronta(&orb[i], t) == 0)
            return 1;
    return 0;
}

static void aggiungi(Tabella *orb, int *cnt, const Tabella *t)
{
    if (presente(orb, *cnt, t))
        return;
    if (*cnt >= ORBITA_MAX) {
        fprintf(stderr, "errore: orbita oltre %d elementi\n", ORBITA_MAX);
        exit(2);
    }
    orb[(*cnt)++] = *t;
}

int tab_orbita(const Tabella *t, Tabella *orb)
{
    int n = t->n, cnt = 0;
    aggiungi(orb, &cnt, t);
    /* Chiusura BFS sotto i generatori: reindex(., f) per f = 1..n,
     * inversione del verso, riflessione reindex(reindex(., n), 2). */
    for (int q = 0; q < cnt; q++) {
        Tabella cur = orb[q], a, b;
        for (int f = 1; f <= n; f++) {
            tab_reindex(&cur, f, &a);
            aggiungi(orb, &cnt, &a);
        }
        tab_inverti(&cur, &a);
        aggiungi(orb, &cnt, &a);
        tab_reindex(&cur, n, &a);
        tab_reindex(&a, 2, &b);
        aggiungi(orb, &cnt, &b);
    }
    return cnt;
}

void tab_canonica(const Tabella *t, Tabella *out)
{
    Tabella orb[ORBITA_MAX];
    int cnt = tab_orbita(t, orb);
    int best = 0;
    for (int i = 1; i < cnt; i++)
        if (tab_confronta(&orb[i], &orb[best]) < 0)
            best = i;
    *out = orb[best];
}

int tab_triangoli(const Tabella *t)
{
    int n = t->n;
    int pos[N_MAX][N_MAX + 1];
    for (int r = 0; r < n; r++)
        for (int k = 0; k < n - 1; k++)
            pos[r][t->r[r][k]] = k;
    int cnt = 0;
    /* Terna {a,b,c}: su ciascuna riga le altre due sono adiacenti. */
    for (int a = 1; a <= n; a++)
        for (int b = a + 1; b <= n; b++)
            for (int c = b + 1; c <= n; c++) {
                if (abs(pos[a - 1][b] - pos[a - 1][c]) != 1) continue;
                if (abs(pos[b - 1][a] - pos[b - 1][c]) != 1) continue;
                if (abs(pos[c - 1][a] - pos[c - 1][b]) != 1) continue;
                cnt++;
            }
    return cnt;
}

int tab_ben_formata(const Tabella *t)
{
    int n = t->n;
    for (int r = 0; r < n; r++) {
        int visto[N_MAX + 1] = {0};
        for (int k = 0; k < n - 1; k++) {
            int x = t->r[r][k];
            if (x < 1 || x > n || x == r + 1 || visto[x])
                return 0;
            visto[x] = 1;
        }
    }
    return 1;
}

void tab_scrivi(FILE *f, const Tabella *t)
{
    for (int r = 0; r < t->n; r++) {
        for (int k = 0; k < t->n - 1; k++)
            fprintf(f, k ? " %d" : "%d", t->r[r][k]);
        fputc('\n', f);
    }
}

void tab_stampa_lista(FILE *f, const Tabella *t)
{
    fputc('[', f);
    for (int r = 0; r < t->n; r++) {
        fputs(r ? ",[" : "[", f);
        for (int k = 0; k < t->n - 1; k++)
            fprintf(f, k ? ",%d" : "%d", t->r[r][k]);
        fputc(']', f);
    }
    fputc(']', f);
}

int tab_leggi(const char *s, int n, Tabella *t)
{
    tab_vuota(t, n);
    char *fine;
    for (int r = 0; r < n; r++)
        for (int k = 0; k < n - 1; k++) {
            long v = strtol(s, &fine, 10);
            if (fine == s || v < 1 || v > n)
                return -1;
            t->r[r][k] = (unsigned char)v;
            s = fine;
        }
    return tab_ben_formata(t) ? 0 : -1;
}
