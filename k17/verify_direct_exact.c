/* Verifier 1/2 — DIRECT classical definition, exact arithmetic.
 *
 * Usage:           ./verify_direct_exact [k17_solutions.json] [--id k17_05]
 * Requirements:    a C compiler and GMP.
 * Expected output: last line  DIRECT EXACT: 21/21 PASS
 *
 * For every arrangement of 17 lines in the file (or only the one given
 * with --id), loads the lines (field "lines_frac", exact rationals a, b, c
 * with a*x + b*y = c) and counts Kobon triangles straight from the
 * classical definition, in exact integer arithmetic (GMP): a triple of
 * lines counts iff its three pairwise intersection points are distinct
 * (nonzero area) and no other line meets the open interior — i.e. no
 * other line strictly separates two of the three vertices. A line passing
 * through a vertex only touches the triangle, which still counts; this is
 * the convention of the published record solutions.
 *
 * Each line is scaled by the positive lcm of its denominators to integers
 * (a, b, c); the crossing of two lines is the homogeneous integer point
 * (X, Y, W) = (c1*b2 - c2*b1, a1*c2 - a2*c1, a1*b2 - a2*b1), W != 0. The
 * three vertices are collinear iff the 3x3 determinant of their
 * homogeneous coordinates vanishes; line m puts (X, Y, W) on the side
 * sgn(a*X + b*Y - c*W) * sgn(W).
 *
 * Also checks: pairwise distinct lines; the parallel pairs are exactly
 * the declared ones (field "parallel_pairs"); the count equals the
 * declared "triangles" (85 = Tamura's bound for 17 lines); every line is
 * a side of exactly k - 2 = 15 counted triangles, as Tamura's bound
 * requires when it is attained.
 *
 * This program does not use geom.c: the direct definition is kept
 * independent of the event-based verifier (classify).
 */
#include <gmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_min.h"

#define MAXK 64

typedef struct {
    mpz_t a, b, c;
} line_t;

/* Growing text buffer for error messages. */
typedef struct {
    char *s;
    size_t len, cap;
} sbuf;

static void sb_add(sbuf *b, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

static void sb_add(sbuf *b, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (b->len + (size_t)n + 1 > b->cap) {
        b->cap = (b->len + (size_t)n + 1) * 2;
        b->s = realloc(b->s, b->cap);
        if (!b->s) {
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
    }
    va_start(ap, fmt);
    vsnprintf(b->s + b->len, (size_t)n + 1, fmt, ap);
    va_end(ap);
    b->len += (size_t)n;
}

/* Append an error, separated by "; " from the previous ones. */
static void sb_err(sbuf *b, int *nerr)
{
    if ((*nerr)++)
        sb_add(b, "; ");
}

/* Parse "p/q" or "p" into a canonical rational; 0 on success. */
static int parse_rat(mpq_t q, const char *s)
{
    const char *p = s + (*s == '-');
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
    if (*p || mpq_set_str(q, s, 10) || mpz_sgn(mpq_denref(q)) == 0)
        return -1;
    mpq_canonicalize(q);
    return 0;
}

/* Integer line from a JSON triple of "p/q" strings; 0 on success. */
static int load_line(line_t *l, const jnode *t)
{
    mpq_t q[3];
    mpz_t m;
    int rc = 0;
    if (!t || t->t != J_ARR || t->n != 3)
        return -1;
    for (int i = 0; i < 3; i++)
        mpq_init(q[i]);
    mpz_init(m);
    for (int i = 0; i < 3 && !rc; i++)
        rc = t->items[i]->t != J_STR || parse_rat(q[i], t->items[i]->s);
    if (!rc) {
        mpz_lcm(m, mpq_denref(q[0]), mpq_denref(q[1]));
        mpz_lcm(m, m, mpq_denref(q[2]));
        mpz_t *dst[3] = { &l->a, &l->b, &l->c };
        for (int i = 0; i < 3; i++) {
            mpz_divexact(*dst[i], m, mpq_denref(q[i]));
            mpz_mul(*dst[i], *dst[i], mpq_numref(q[i]));
        }
    }
    for (int i = 0; i < 3; i++)
        mpq_clear(q[i]);
    mpz_clear(m);
    return rc ? -1 : 0;
}

/* Side of line l at homogeneous point p[0..2]: -1, 0, 1. */
static int side(const line_t *l, mpz_t p[3], mpz_t t)
{
    mpz_mul(t, l->a, p[0]);
    mpz_addmul(t, l->b, p[1]);
    mpz_submul(t, l->c, p[2]);
    return mpz_sgn(t) * mpz_sgn(p[2]);
}

/* Count triangles of k lines; fills per_line, appends errors to err. */
static int check(line_t *L, int k, int declared[][2], int ndecl,
                 int *per_line, sbuf *err, int *nerr)
{
    static mpz_t P[MAXK][MAXK][3];   /* crossing of i < j, homogeneous */
    static int has[MAXK][MAXK];
    static int inited = 0;
    mpz_t t, u, det;
    int par[MAXK * MAXK][2], npar = 0, count = 0;

    if (!inited) {
        for (int i = 0; i < MAXK; i++)
            for (int j = 0; j < MAXK; j++)
                mpz_inits(P[i][j][0], P[i][j][1], P[i][j][2], NULL);
        inited = 1;
    }
    mpz_inits(t, u, det, NULL);

    for (int i = 0; i < k; i++)
        for (int j = i + 1; j < k; j++) {
            line_t *li = &L[i], *lj = &L[j];
            mpz_mul(t, li->a, lj->b);
            mpz_mul(u, lj->a, li->b);
            if (mpz_cmp(t, u))
                continue;
            mpz_mul(t, li->b, lj->c);
            mpz_mul(u, lj->b, li->c);
            if (mpz_cmp(t, u))
                continue;
            mpz_mul(t, li->a, lj->c);
            mpz_mul(u, lj->a, li->c);
            if (mpz_cmp(t, u))
                continue;
            sb_err(err, nerr);
            sb_add(err, "lines %d and %d coincide", i, j);
        }

    for (int i = 0; i < k; i++)
        for (int j = i + 1; j < k; j++) {
            line_t *l1 = &L[i], *l2 = &L[j];
            mpz_t *p = P[i][j];
            mpz_mul(p[2], l1->a, l2->b);
            mpz_submul(p[2], l2->a, l1->b);
            has[i][j] = mpz_sgn(p[2]) != 0;
            if (!has[i][j]) {
                par[npar][0] = i;
                par[npar][1] = j;
                npar++;
                continue;
            }
            mpz_mul(p[0], l1->c, l2->b);
            mpz_submul(p[0], l2->c, l1->b);
            mpz_mul(p[1], l1->a, l2->c);
            mpz_submul(p[1], l2->a, l1->c);
        }
    int same = npar == ndecl;
    for (int q = 0; same && q < npar; q++)
        same = par[q][0] == declared[q][0] && par[q][1] == declared[q][1];
    if (!same) {
        sb_err(err, nerr);
        sb_add(err, "parallel pairs [");
        for (int q = 0; q < npar; q++)
            sb_add(err, "%s(%d, %d)", q ? ", " : "", par[q][0], par[q][1]);
        sb_add(err, "] != declared");
    }

    for (int i = 0; i < k; i++)
        per_line[i] = 0;
    for (int i = 0; i < k; i++)
        for (int j = i + 1; j < k; j++)
            for (int l = j + 1; l < k; l++) {
                if (!has[i][j] || !has[i][l] || !has[j][l])
                    continue;
                mpz_t *v[3] = { P[i][j], P[i][l], P[j][l] };
                /* det of rows (X, Y, W): zero area iff zero */
                mpz_mul(t, v[1][1], v[2][2]);
                mpz_submul(t, v[2][1], v[1][2]);
                mpz_mul(det, v[0][0], t);
                mpz_mul(t, v[1][0], v[2][2]);
                mpz_submul(t, v[2][0], v[1][2]);
                mpz_submul(det, v[0][1], t);
                mpz_mul(t, v[1][0], v[2][1]);
                mpz_submul(t, v[2][0], v[1][1]);
                mpz_addmul(det, v[0][2], t);
                if (mpz_sgn(det) == 0)
                    continue;           /* concurrent triple: zero area */
                int empty = 1;
                for (int m = 0; m < k && empty; m++) {
                    if (m == i || m == j || m == l)
                        continue;
                    int s0 = side(&L[m], v[0], t);
                    int s1 = side(&L[m], v[1], t);
                    int s2 = side(&L[m], v[2], t);
                    if (s0 * s1 < 0 || s0 * s2 < 0 || s1 * s2 < 0)
                        empty = 0;
                }
                if (empty) {
                    count++;
                    per_line[i]++;
                    per_line[j]++;
                    per_line[l]++;
                }
            }
    mpz_clears(t, u, det, NULL);
    return count;
}

static int pair_cmp(const void *x, const void *y)
{
    const int *a = x, *b = y;
    if (a[0] != b[0])
        return a[0] < b[0] ? -1 : 1;
    return (a[1] > b[1]) - (a[1] < b[1]);
}

static void usage(void)
{
    fprintf(stderr, "usage: verify_direct_exact [file] [--id ID]\n");
    exit(2);
}

int main(int argc, char **argv)
{
    const char *file = NULL, *id = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--id") == 0) {
            if (++i >= argc)
                usage();
            id = argv[i];
        } else if (strncmp(argv[i], "--id=", 5) == 0) {
            id = argv[i] + 5;
        } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            usage();
        } else if (!file) {
            file = argv[i];
        } else {
            usage();
        }
    }
    if (!file)
        file = "k17_solutions.json";

    char jerr[512];
    jnode *data = json_load_file(file, jerr, sizeof jerr);
    if (!data) {
        fprintf(stderr, "%s\n", jerr);
        return 1;
    }
    long target;
    jnode *arrs = json_get(data, "arrangements");
    if (!json_long(json_get(data, "triangles"), &target) ||
        !arrs || arrs->t != J_ARR) {
        fprintf(stderr, "%s: missing \"triangles\" or \"arrangements\"\n", file);
        return 1;
    }

    int total = 0, passed = 0;
    for (size_t a = 0; a < arrs->n; a++) {
        jnode *arr = arrs->items[a];
        jnode *aid = json_get(arr, "id");
        if (!aid || aid->t != J_STR) {
            fprintf(stderr, "arrangement %zu: missing id\n", a);
            return 1;
        }
        if (id && strcmp(aid->s, id) != 0)
            continue;
        total++;

        jnode *lf = json_get(arr, "lines_frac");
        if (!lf || lf->t != J_ARR || lf->n > MAXK) {
            fprintf(stderr, "%s: bad lines_frac\n", aid->s);
            return 1;
        }
        int k = (int)lf->n;
        line_t L[MAXK];
        for (int i = 0; i < k; i++) {
            mpz_inits(L[i].a, L[i].b, L[i].c, NULL);
            if (load_line(&L[i], lf->items[i])) {
                fprintf(stderr, "%s: bad line %d\n", aid->s, i);
                return 1;
            }
        }

        /* declared parallel pairs: set of sorted pairs */
        static int decl[MAXK * MAXK][2];
        int ndecl = 0, bad_decl = 0;
        jnode *pp = json_get(arr, "parallel_pairs");
        for (size_t q = 0; pp && pp->t == J_ARR && q < pp->n; q++) {
            jnode *p = pp->items[q];
            long x, y;
            if (!p || p->t != J_ARR || p->n != 2 || ndecl >= MAXK * MAXK ||
                !json_long(p->items[0], &x) || !json_long(p->items[1], &y)) {
                bad_decl = 1;
                continue;
            }
            decl[ndecl][0] = (int)(x < y ? x : y);
            decl[ndecl][1] = (int)(x < y ? y : x);
            ndecl++;
        }
        qsort(decl, (size_t)ndecl, sizeof decl[0], pair_cmp);
        int w = 0;
        for (int q = 0; q < ndecl; q++)
            if (w == 0 || pair_cmp(decl[w - 1], decl[q]))
                memcpy(decl[w++], decl[q], sizeof decl[0]);
        ndecl = bad_decl ? -1 : w;

        sbuf err = { NULL, 0, 0 };
        int nerr = 0, per_line[MAXK];
        int side_n = k - 2;
        int count = check(L, k, decl, ndecl, per_line, &err, &nerr);
        if (count != target) {
            sb_err(&err, &nerr);
            sb_add(&err, "count %d != %ld", count, target);
        }
        int all = 1;
        for (int i = 0; i < k; i++)
            all &= per_line[i] == side_n;
        if (!all) {
            sb_err(&err, &nerr);
            sb_add(&err, "triangles per line [");
            for (int i = 0; i < k; i++)
                sb_add(&err, i ? ", %d" : "%d", per_line[i]);
            sb_add(&err, "], expected %d", side_n);
        }
        if (nerr) {
            printf("%s: %d triangles — FAIL: %s\n", aid->s, count, err.s);
        } else {
            passed++;
            printf("%s: %d triangles, %d per line — PASS\n", aid->s, count, side_n);
        }
        free(err.s);
        for (int i = 0; i < k; i++)
            mpz_clears(L[i].a, L[i].b, L[i].c, NULL);
    }
    if (id && total == 0) {
        printf("no arrangement with id %s\n", id);
        json_free(data);
        return 1;
    }
    int ok = passed == total;
    printf("DIRECT EXACT: %d/%d %s\n", passed, total, ok ? "PASS" : "FAIL");
    json_free(data);
    return ok ? 0 : 1;
}
