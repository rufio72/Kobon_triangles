/* Tabella degli eventi (TabellaD): forma, confronto, conversioni, lettura e
 * scrittura. */
#include "dtab.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void dtab_vuota(TabellaD *t, int n)
{
    memset(t, 0, sizeof *t);
    memset(t->pos, DT_NO, sizeof t->pos);
    t->n = n;
}

int dtab_parallele(const TabellaD *t, int i, int j)
{
    return i != j && t->pos[i][j] == DT_NO;
}

int dtab_evento(const TabellaD *t, int r, int e, int *mem)
{
    int c = 0;
    for (int j = 0; j < t->n; j++)
        if (j != r && t->pos[r][j] == e)
            mem[c++] = j;
    return c;
}

int dtab_ben_formata(const TabellaD *t)
{
    int n = t->n;
    if (n < 1 || n > N_MAX)
        return 0;
    for (int r = 0; r < n; r++) {
        if (t->pos[r][r] != DT_NO || t->nev[r] > n - 1)
            return 0;
        int usato[N_MAX] = {0};
        for (int j = 0; j < n; j++) {
            if (j == r)
                continue;
            int e = t->pos[r][j];
            if ((e == DT_NO) != (t->pos[j][r] == DT_NO))
                return 0;               /* incrocio non simmetrico */
            if (e == DT_NO)
                continue;
            if (e >= t->nev[r])
                return 0;
            usato[e] = 1;
        }
        for (int e = 0; e < t->nev[r]; e++)
            if (!usato[e])
                return 0;               /* evento vuoto */
    }
    /* Punti multipli: se j e k stanno nello stesso evento di r, allora r e k
     * stanno nello stesso evento di j. */
    for (int r = 0; r < n; r++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++) {
                if (r == j || r == k || j == k) continue;
                if (t->pos[r][j] == DT_NO || t->pos[r][j] != t->pos[r][k])
                    continue;
                if (t->pos[j][r] == DT_NO || t->pos[j][r] != t->pos[j][k])
                    return 0;
            }
    /* Parallelismo transitivo. */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++) {
                if (i == j || j == k || i == k) continue;
                if (dtab_parallele(t, i, j) && dtab_parallele(t, j, k) &&
                    !dtab_parallele(t, i, k))
                    return 0;
            }
    return 1;
}

int dtab_classi_consecutive(const TabellaD *t)
{
    int n = t->n;
    for (int i = 0; i < n; i++)
        for (int j = i + 2; j < n; j++)
            if (dtab_parallele(t, i, j))
                for (int k = i + 1; k < j; k++)
                    if (!dtab_parallele(t, i, k) || !dtab_parallele(t, k, j))
                        return 0;
    return 1;
}

int dtab_semplice(const TabellaD *t)
{
    for (int r = 0; r < t->n; r++)
        if (t->nev[r] != t->n - 1)
            return 0;
    return 1;
}

void dtab_chiave(const TabellaD *t, unsigned char *k)
{
    int w = 0, mem[N_MAX];
    memset(k, 0, DT_CHIAVE);
    for (int r = 0; r < t->n; r++) {
        for (int e = 0; e < t->nev[r]; e++) {
            int c = dtab_evento(t, r, e, mem);
            for (int q = 0; q < c; q++)
                k[w++] = (unsigned char)(mem[q] + 1);
            k[w++] = 0;
        }
        k[w++] = 0;
    }
}

int dtab_confronta(const TabellaD *a, const TabellaD *b)
{
    if (a->n != b->n)
        return a->n - b->n;
    unsigned char ka[DT_CHIAVE], kb[DT_CHIAVE];
    dtab_chiave(a, ka);
    dtab_chiave(b, kb);
    return memcmp(ka, kb, DT_CHIAVE);
}

void dtab_da_tabella(const Tabella *s, TabellaD *t)
{
    int n = s->n;
    dtab_vuota(t, n);
    for (int r = 0; r < n; r++) {
        t->nev[r] = (unsigned char)(n - 1);
        for (int k = 0; k < n - 1; k++)
            t->pos[r][s->r[r][k] - 1] = (unsigned char)k;
    }
}

void dtab_a_eventi(const TabellaD *t, events_t *ev)
{
    int mem[N_MAX];
    ev->k = t->n;
    for (int r = 0; r < t->n; r++) {
        int q = 0;
        ev->nev[r] = t->nev[r];
        for (int e = 0; e < t->nev[r]; e++) {
            ev->start[r][e] = q;
            int c = dtab_evento(t, r, e, mem);
            for (int x = 0; x < c; x++)
                ev->mem[r][q++] = mem[x];
        }
        ev->start[r][t->nev[r]] = q;
    }
}

int dtab_da_eventi(const events_t *ev, TabellaD *t)
{
    if (ev->k > N_MAX)
        return -1;
    dtab_vuota(t, ev->k);
    for (int r = 0; r < ev->k; r++) {
        t->nev[r] = (unsigned char)ev->nev[r];
        for (int e = 0; e < ev->nev[r]; e++)
            for (int q = ev->start[r][e]; q < ev->start[r][e + 1]; q++)
                t->pos[r][ev->mem[r][q]] = (unsigned char)e;
    }
    return dtab_ben_formata(t) ? 0 : -1;
}

int dtab_triangoli(const TabellaD *t)
{
    static events_t ev;
    dtab_a_eventi(t, &ev);
    return geom_count_by_adjacency(&ev);
}

int dtab_punti_multipli(const TabellaD *t, int tup[][N_MAX], int *len)
{
    int n = t->n, c = 0, mem[N_MAX];
    for (int r = 0; r < n; r++)
        for (int e = 0; e < t->nev[r]; e++) {
            int m = dtab_evento(t, r, e, mem);
            if (m < 2 || mem[0] < r)
                continue;   /* la tupla si registra dalla sua retta minima */
            tup[c][0] = r;
            for (int q = 0; q < m; q++)
                tup[c][q + 1] = mem[q];
            len[c++] = m + 1;
        }
    /* Ordinamento lessicografico (insertion sort, poche tuple). */
    for (int a = 1; a < c; a++)
        for (int b = a; b > 0; b--) {
            int x = 0;
            while (x < len[b] && x < len[b - 1] && tup[b][x] == tup[b - 1][x])
                x++;
            int minore = x < len[b] && x < len[b - 1]
                             ? tup[b][x] < tup[b - 1][x]
                             : len[b] < len[b - 1];
            if (!minore)
                break;
            int tmp[N_MAX], tl = len[b];
            memcpy(tmp, tup[b], sizeof tmp);
            memcpy(tup[b], tup[b - 1], sizeof tmp);
            memcpy(tup[b - 1], tmp, sizeof tmp);
            len[b] = len[b - 1];
            len[b - 1] = tl;
        }
    return c;
}

void dtab_scrivi_multipli(FILE *f, const TabellaD *t)
{
    static int tup[N_MAX * N_MAX][N_MAX];
    int len[N_MAX * N_MAX];
    int c = dtab_punti_multipli(t, tup, len);
    if (c == 0)
        fputs("nessuno", f);
    for (int q = 0; q < c; q++) {
        fputs(q ? " (" : "(", f);
        for (int x = 0; x < len[q]; x++)
            fprintf(f, x ? " %d" : "%d", tup[q][x] + 1);
        fputc(')', f);
    }
}

void dtab_scrivi_parallele(FILE *f, const TabellaD *t)
{
    int n = t->n, scritte = 0, fatto[N_MAX] = {0};
    for (int i = 0; i < n; i++) {
        if (fatto[i])
            continue;
        int c = 0;
        for (int j = i + 1; j < n; j++)
            if (dtab_parallele(t, i, j))
                c++;
        if (c == 0)
            continue;
        fputs(scritte++ ? " {" : "{", f);
        fprintf(f, "%d", i + 1);
        for (int j = i + 1; j < n; j++)
            if (dtab_parallele(t, i, j)) {
                fprintf(f, " %d", j + 1);
                fatto[j] = 1;
            }
        fputc('}', f);
    }
    if (!scritte)
        fputs("nessuna", f);
}

static void scrivi_riga(FILE *f, const TabellaD *t, int r)
{
    int mem[N_MAX];
    for (int e = 0; e < t->nev[r]; e++) {
        int c = dtab_evento(t, r, e, mem);
        if (e)
            fputc(' ', f);
        if (c > 1)
            fputc('(', f);
        for (int q = 0; q < c; q++)
            fprintf(f, q ? " %d" : "%d", mem[q] + 1);
        if (c > 1)
            fputc(')', f);
    }
}

void dtab_scrivi(FILE *f, const TabellaD *t)
{
    for (int r = 0; r < t->n; r++) {
        scrivi_riga(f, t, r);
        fputc('\n', f);
    }
}

void dtab_scrivi_compatta(FILE *f, const TabellaD *t)
{
    for (int r = 0; r < t->n; r++) {
        if (r)
            fputs(" ; ", f);
        scrivi_riga(f, t, r);
    }
}

/* Analizza una riga di testo [s, fine): membri[c] = {retta 0-based, evento}.
 * Restituisce 0 se ok, 1 se la riga è vuota, -1 se errore. */
static int leggi_riga(const char *s, const char *fine, int membri[][2],
                      int *nmem, int *nev)
{
    int e = 0, dentro = 0, vuota = 1, c = 0;
    while (s < fine) {
        if (isspace((unsigned char)*s)) {
            s++;
        } else if (*s == '(') {
            if (dentro)
                return -1;
            dentro = 1;
            vuota = 0;
            s++;
        } else if (*s == ')') {
            if (!dentro)
                return -1;
            dentro = 0;
            e++;
            s++;
        } else if (isdigit((unsigned char)*s)) {
            char *dopo;
            long v = strtol(s, &dopo, 10);
            s = dopo;
            if (v < 1 || v > N_MAX || c >= N_MAX)
                return -1;
            membri[c][0] = (int)v - 1;
            membri[c][1] = e;
            c++;
            vuota = 0;
            if (!dentro)
                e++;
        } else {
            return -1;
        }
    }
    if (dentro)
        return -1;
    *nmem = c;
    *nev = e;
    return vuota ? 1 : 0;
}

int dtab_leggi(const char *s, TabellaD *t)
{
    static int membri[N_MAX][N_MAX][2];
    int scarto[N_MAX][2];
    int nev[N_MAX], nmem[N_MAX], n = 0;
    while (*s) {
        const char *fine = s;
        while (*fine && *fine != '\n' && *fine != ';' && *fine != '#')
            fine++;
        int ev = 0, nm = 0;
        int esito = leggi_riga(s, fine, n < N_MAX ? membri[n] : scarto, &nm,
                               &ev);
        if (esito < 0 || (esito == 0 && n >= N_MAX))
            return -1;
        if (esito == 0) {
            nev[n] = ev;
            nmem[n] = nm;
            n++;
        }
        if (*fine == '#')
            while (*fine && *fine != '\n')
                fine++;
        s = *fine ? fine + 1 : fine;
    }
    if (n < 1)
        return -1;
    dtab_vuota(t, n);
    for (int r = 0; r < n; r++) {
        t->nev[r] = (unsigned char)nev[r];
        for (int q = 0; q < nmem[r]; q++) {
            int j = membri[r][q][0];
            if (j >= n || j == r || t->pos[r][j] != DT_NO)
                return -1;
            t->pos[r][j] = (unsigned char)membri[r][q][1];
        }
    }
    return dtab_ben_formata(t) ? 0 : -1;
}

int dtab_leggi_file(const char *percorso, TabellaD *t)
{
    FILE *f = fopen(percorso, "r");
    if (!f)
        return -1;
    char buf[1 << 14];
    size_t l = fread(buf, 1, sizeof buf - 1, f);
    fclose(f);
    buf[l] = '\0';
    return dtab_leggi(buf, t);
}
