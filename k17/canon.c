/* Complete canonical form of an event structure, and its SHA-256.
 *
 * Every choice of a line x (to become line 0) and of its direction fixes
 * the labels of the other lines in the order they cross x; ties (lines
 * through a common point of x, lines parallel to x) are enumerated
 * exhaustively. Each line's relabeled sequence is then taken in the
 * direction that makes it smaller. The minimum over all candidates is
 * invariant under relabeling and per-line reversal.
 */
#include "canon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha256.h"

/* Compare two sorted integer sequences lexicographically. */
static int seq_cmp(const int *a, int na, const int *b, int nb)
{
    int n = na < nb ? na : nb;
    for (int i = 0; i < n; i++)
        if (a[i] != b[i])
            return a[i] < b[i] ? -1 : 1;
    return (na > nb) - (na < nb);
}

#define EV_PTR(f, i, e) (&(f)->mem[i][(f)->start[i][e]])
#define EV_LEN(f, i, e) ((f)->start[i][(e) + 1] - (f)->start[i][e])

/* Compare line i of x with line j of y (sequences of events). */
static int line_cmp(const events_t *x, int i, const events_t *y, int j)
{
    int n = x->nev[i] < y->nev[j] ? x->nev[i] : y->nev[j];
    for (int e = 0; e < n; e++) {
        int c = seq_cmp(EV_PTR(x, i, e), EV_LEN(x, i, e),
                        EV_PTR(y, j, e), EV_LEN(y, j, e));
        if (c)
            return c;
    }
    return (x->nev[i] > y->nev[j]) - (x->nev[i] < y->nev[j]);
}

int canon_cmp(const events_t *x, const events_t *y)
{
    int n = x->k < y->k ? x->k : y->k;
    for (int i = 0; i < n; i++) {
        int c = line_cmp(x, i, y, i);
        if (c)
            return c;
    }
    return (x->k > y->k) - (x->k < y->k);
}

static int int_cmp(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

/* Next lexicographic permutation; 0 when the last one was reached. */
static int next_perm(int *a, int n)
{
    int i = n - 2;
    while (i >= 0 && a[i] >= a[i + 1])
        i--;
    if (i < 0)
        return 0;
    int j = n - 1;
    while (a[j] <= a[i])
        j--;
    int t = a[i]; a[i] = a[j]; a[j] = t;
    for (int l = i + 1, r = n - 1; l < r; l++, r--) {
        t = a[l]; a[l] = a[r]; a[r] = t;
    }
    return 1;
}

typedef struct {
    const events_t *ev;
    int k;
    int ngroups;
    int gstart[GEOM_MAXK + 1];  /* group g: gbuf[gstart[g] .. gstart[g+1]) */
    int gbuf[GEOM_MAXK];
    int lab[GEOM_MAXK];
    events_t cand;              /* candidate form being built */
    events_t tmp;               /* one relabeled line, forward order */
    events_t *best;
    int have_best;
} canon_state;

/* Build the form for the current labels and keep it if smaller. */
static void evaluate(canon_state *st)
{
    const events_t *ev = st->ev;
    int k = st->k, inv[GEOM_MAXK];
    for (int i = 0; i < k; i++)
        inv[st->lab[i]] = i;
    st->cand.k = k;
    st->tmp.k = 1;
    for (int L = 0; L < k; L++) {
        int i = inv[L], ne = ev->nev[i], q = 0;
        /* relabeled events of line i, forward, in tmp line 0 */
        for (int e = 0; e < ne; e++) {
            st->tmp.start[0][e] = q;
            for (int r = ev->start[i][e]; r < ev->start[i][e + 1]; r++)
                st->tmp.mem[0][q++] = st->lab[ev->mem[i][r]];
            qsort(&st->tmp.mem[0][st->tmp.start[0][e]],
                  (size_t)(q - st->tmp.start[0][e]), sizeof(int), int_cmp);
        }
        st->tmp.start[0][ne] = q;
        st->tmp.nev[0] = ne;
        /* forward vs reversed: s[e] against s[ne-1-e] */
        int rev = 0;
        for (int e = 0; e < ne; e++) {
            int c = seq_cmp(EV_PTR(&st->tmp, 0, e), EV_LEN(&st->tmp, 0, e),
                            EV_PTR(&st->tmp, 0, ne - 1 - e),
                            EV_LEN(&st->tmp, 0, ne - 1 - e));
            if (c) {
                rev = c > 0;
                break;
            }
        }
        int w = 0;
        for (int e = 0; e < ne; e++) {
            int src = rev ? ne - 1 - e : e;
            int len = EV_LEN(&st->tmp, 0, src);
            st->cand.start[L][e] = w;
            memcpy(&st->cand.mem[L][w], EV_PTR(&st->tmp, 0, src),
                   (size_t)len * sizeof(int));
            w += len;
        }
        st->cand.start[L][ne] = w;
        st->cand.nev[L] = ne;
    }
    if (!st->have_best || canon_cmp(&st->cand, st->best) < 0) {
        *st->best = st->cand;
        st->have_best = 1;
    }
}

/* Enumerate the permutations of groups g, g+1, ... (cartesian product). */
static void enumerate(canon_state *st, int g, int next_label)
{
    if (g == st->ngroups) {
        evaluate(st);
        return;
    }
    int n = st->gstart[g + 1] - st->gstart[g];
    int perm[GEOM_MAXK];
    memcpy(perm, &st->gbuf[st->gstart[g]], (size_t)n * sizeof(int));
    do {
        for (int t = 0; t < n; t++)
            st->lab[perm[t]] = next_label + t;
        enumerate(st, g + 1, next_label + n);
    } while (next_perm(perm, n));
}

void canonical_form(const events_t *ev, events_t *form)
{
    canon_state *st = malloc(sizeof *st);
    if (!st) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    st->ev = ev;
    st->k = ev->k;
    st->best = form;
    st->have_best = 0;
    for (int x = 0; x < ev->k; x++) {
        for (int dir = 0; dir < 2; dir++) {
            int seen[GEOM_MAXK] = { 0 }, w = 0, ng = 0;
            int ne = ev->nev[x];
            for (int t = 0; t < ne; t++) {
                int e = dir ? ne - 1 - t : t;
                st->gstart[ng++] = w;
                for (int r = ev->start[x][e]; r < ev->start[x][e + 1]; r++) {
                    st->gbuf[w++] = ev->mem[x][r];
                    seen[ev->mem[x][r]] = 1;
                }
                qsort(&st->gbuf[st->gstart[ng - 1]],
                      (size_t)(w - st->gstart[ng - 1]), sizeof(int), int_cmp);
            }
            /* lines not crossing x: parallel group, in increasing order */
            int wp = w;
            for (int j = 0; j < ev->k; j++)
                if (j != x && !seen[j])
                    st->gbuf[w++] = j;
            if (w > wp)
                st->gstart[ng++] = wp;
            st->gstart[ng] = w;
            st->ngroups = ng;
            st->lab[x] = 0;
            enumerate(st, 0, 1);
        }
    }
    free(st);
}

char *canon_json(const events_t *f)
{
    size_t cap = 3, len = 0;
    for (int i = 0; i < f->k; i++)
        cap += 3 + 3 * (size_t)f->nev[i] + 12 * (size_t)f->start[i][f->nev[i]];
    char *s = malloc(cap);
    if (!s) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    s[len++] = '[';
    for (int i = 0; i < f->k; i++) {
        if (i)
            s[len++] = ',';
        s[len++] = '[';
        for (int e = 0; e < f->nev[i]; e++) {
            if (e)
                s[len++] = ',';
            s[len++] = '[';
            for (int q = f->start[i][e]; q < f->start[i][e + 1]; q++)
                len += (size_t)sprintf(s + len, q > f->start[i][e] ? ",%d" : "%d",
                                       f->mem[i][q]);
            s[len++] = ']';
        }
        s[len++] = ']';
    }
    s[len++] = ']';
    s[len] = '\0';
    return s;
}

void canonical_sha256(const events_t *form, char out[65])
{
    char *s = canon_json(form);
    sha256_hex((const unsigned char *)s, strlen(s), out);
    free(s);
}
