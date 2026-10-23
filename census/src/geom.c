/* Exact geometry of line arrangements: lines, crossing events, adjacency. */
#include "geom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int geom_parse_rat(mpq_t q, const char *s)
{
    const char *p = s;
    if (*p == '-')
        p++;
    if (*p < '0' || *p > '9')
        return -1;
    while (*p >= '0' && *p <= '9')
        p++;
    if (*p == '/') {
        p++;
        if (*p < '0' || *p > '9')
            return -1;
        while (*p >= '0' && *p <= '9')
            p++;
    }
    if (*p != '\0')
        return -1;
    if (mpq_set_str(q, s, 10) != 0)
        return -1;
    if (mpz_sgn(mpq_denref(q)) == 0)
        return -1;
    mpq_canonicalize(q);
    return 0;
}

void zline_init(zline *l)
{
    mpz_inits(l->a, l->b, l->c, NULL);
}

void zline_clear(zline *l)
{
    mpz_clears(l->a, l->b, l->c, NULL);
}

int zline_from_q(zline *l, const mpq_t a, const mpq_t b, const mpq_t c)
{
    mpz_t m;
    mpz_init(m);
    mpz_lcm(m, mpq_denref(a), mpq_denref(b));
    mpz_lcm(m, m, mpq_denref(c));          /* positive */
    mpz_divexact(l->a, m, mpq_denref(a));
    mpz_mul(l->a, l->a, mpq_numref(a));
    mpz_divexact(l->b, m, mpq_denref(b));
    mpz_mul(l->b, l->b, mpq_numref(b));
    mpz_divexact(l->c, m, mpq_denref(c));
    mpz_mul(l->c, l->c, mpq_numref(c));
    mpz_clear(m);
    return (mpz_sgn(l->a) == 0 && mpz_sgn(l->b) == 0) ? -1 : 0;
}

int zline_from_strings(zline *l, const char *s[3])
{
    mpq_t q[3];
    int rc = 0;
    for (int t = 0; t < 3; t++)
        mpq_init(q[t]);
    for (int t = 0; t < 3 && rc == 0; t++)
        rc = geom_parse_rat(q[t], s[t]);
    if (rc == 0)
        rc = zline_from_q(l, q[0], q[1], q[2]);
    for (int t = 0; t < 3; t++)
        mpq_clear(q[t]);
    return rc;
}

int tuple_cmp(const tuple_t *x, const tuple_t *y)
{
    int n = x->n < y->n ? x->n : y->n;
    for (int i = 0; i < n; i++)
        if (x->v[i] != y->v[i])
            return x->v[i] < y->v[i] ? -1 : 1;
    return (x->n > y->n) - (x->n < y->n);
}

static int tuple_cmp_void(const void *x, const void *y)
{
    return tuple_cmp(x, y);
}

/* Add t to l unless already present. */
static void tuple_list_add(tuple_list *l, const tuple_t *t, int *cap)
{
    for (int i = 0; i < l->n; i++)
        if (tuple_cmp(&l->t[i], t) == 0)
            return;
    if (l->n == *cap) {
        *cap = *cap ? 2 * *cap : 8;
        l->t = realloc(l->t, (size_t)*cap * sizeof *l->t);
        if (!l->t) {
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
    }
    l->t[l->n++] = *t;
}

void tuple_list_free(tuple_list *l)
{
    free(l->t);
    l->t = NULL;
    l->n = 0;
}

char *tuple_list_repr(const tuple_list *l)
{
    size_t cap = 3, len = 0;
    for (int i = 0; i < l->n; i++)
        cap += 6 + 22 * (size_t)l->t[i].n;
    char *s = malloc(cap);
    if (!s)
        return NULL;
    s[len++] = '[';
    for (int i = 0; i < l->n; i++) {
        if (i)
            len += (size_t)sprintf(s + len, ", ");
        s[len++] = '(';
        for (int j = 0; j < l->t[i].n; j++)
            len += (size_t)sprintf(s + len, j ? ", %d" : "%d", l->t[i].v[j]);
        if (l->t[i].n == 1)
            s[len++] = ',';
        s[len++] = ')';
    }
    s[len++] = ']';
    s[len] = '\0';
    return s;
}

void geom_events(const zline *L, int k, events_t *ev,
                 tuple_list *par, tuple_list *multi)
{
    mpz_t D, X, Y, t;
    mpq_t s[GEOM_MAXK];
    int order[GEOM_MAXK];
    int pcap = 0, mcap = 0;

    mpz_inits(D, X, Y, t, NULL);
    for (int j = 0; j < k; j++)
        mpq_init(s[j]);
    if (par) {
        par->n = 0;
        par->t = NULL;
    }
    if (multi) {
        multi->n = 0;
        multi->t = NULL;
    }
    ev->k = k;
    for (int i = 0; i < k; i++) {
        const zline *l1 = &L[i];
        int n = 0;
        for (int j = 0; j < k; j++) {
            if (j == i)
                continue;
            const zline *l2 = &L[j];
            /* D = a1*b2 - a2*b1 */
            mpz_mul(D, l1->a, l2->b);
            mpz_submul(D, l2->a, l1->b);
            if (mpz_sgn(D) == 0) {
                if (par) {
                    tuple_t p = { 2, { i < j ? i : j, i < j ? j : i } };
                    tuple_list_add(par, &p, &pcap);
                }
                continue;
            }
            /* X = c1*b2 - c2*b1, Y = a1*c2 - a2*c1 */
            mpz_mul(X, l1->c, l2->b);
            mpz_submul(X, l2->c, l1->b);
            mpz_mul(Y, l1->a, l2->c);
            mpz_submul(Y, l2->a, l1->c);
            /* s = (-b1*X + a1*Y) / D */
            mpz_mul(t, l1->a, Y);
            mpz_submul(t, l1->b, X);
            mpq_set_num(s[j], t);
            mpq_set_den(s[j], D);
            if (mpz_sgn(D) < 0) {
                mpz_neg(mpq_numref(s[j]), mpq_numref(s[j]));
                mpz_neg(mpq_denref(s[j]), mpq_denref(s[j]));
            }
            mpq_canonicalize(s[j]);
            /* Stable insertion by s (j increasing within ties). */
            int pos = n;
            while (pos > 0 && mpq_cmp(s[order[pos - 1]], s[j]) > 0) {
                order[pos] = order[pos - 1];
                pos--;
            }
            order[pos] = j;
            n++;
        }
        int ne = 0;
        for (int q = 0; q < n; q++) {
            if (q == 0 || !mpq_equal(s[order[q - 1]], s[order[q]]))
                ev->start[i][ne++] = q;
            ev->mem[i][q] = order[q];
        }
        ev->nev[i] = ne;
        ev->start[i][ne] = n;
    }
    if (multi) {
        for (int i = 0; i < k; i++)
            for (int e = 0; e < ev->nev[i]; e++) {
                int a = ev->start[i][e], b = ev->start[i][e + 1];
                if (b - a < 2)
                    continue;
                tuple_t tp = { 0, { 0 } };
                int ins = 0;
                for (int q = a; q < b; q++) {
                    if (!ins && i < ev->mem[i][q]) {
                        tp.v[tp.n++] = i;
                        ins = 1;
                    }
                    tp.v[tp.n++] = ev->mem[i][q];
                }
                if (!ins)
                    tp.v[tp.n++] = i;
                tuple_list_add(multi, &tp, &mcap);
            }
        qsort(multi->t, (size_t)multi->n, sizeof *multi->t, tuple_cmp_void);
    }
    if (par)
        qsort(par->t, (size_t)par->n, sizeof *par->t, tuple_cmp_void);
    for (int j = 0; j < k; j++)
        mpq_clear(s[j]);
    mpz_clears(D, X, Y, t, NULL);
}

int geom_count_by_adjacency(const events_t *ev)
{
    static int pos[GEOM_MAXK][GEOM_MAXK];
    int k = ev->k, count = 0;
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++)
            pos[i][j] = -1;
        for (int e = 0; e < ev->nev[i]; e++)
            for (int q = ev->start[i][e]; q < ev->start[i][e + 1]; q++)
                pos[i][ev->mem[i][q]] = e;
    }
    for (int x = 0; x < k; x++)
        for (int y = x + 1; y < k; y++)
            for (int z = y + 1; z < k; z++) {
                if (pos[x][y] < 0 || pos[x][z] < 0 || pos[y][z] < 0)
                    continue;           /* a parallel pair */
                if (pos[x][y] == pos[x][z])
                    continue;           /* all three through one point */
                if (abs(pos[x][y] - pos[x][z]) == 1 &&
                    abs(pos[y][x] - pos[y][z]) == 1 &&
                    abs(pos[z][x] - pos[z][y]) == 1)
                    count++;
            }
    return count;
}
