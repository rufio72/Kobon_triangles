/* Verifier 2/2 — EVENTS, adjacency count and COMPLETE canonical form.
 *
 * Usage:           ./classify [k17_solutions.json] [--self-test]
 * Requirements:    a C compiler and GMP.
 * Expected output: last line  CLASSES: <n> distinct, <m> arrangements — PASS
 *
 * For every arrangement in the file (field "lines_frac", exact rationals
 * a, b, c with a*x + b*y = c) this program re-derives, in exact
 * arithmetic, the sequence of crossing events along each line (an event
 * is the set of lines met at one point), and from it:
 *
 * - the triangle count by the adjacency criterion (a triple counts iff
 *   every pair crosses, the three crossings are not one single point, and
 *   on each of the three lines the two relevant events are consecutive);
 * - the multiple points and parallel pairs, checked against the declared
 *   ones ("triple_points", "parallel_pairs");
 * - a complete canonical form for the isomorphism relation "relabel the
 *   lines and reverse the traversal direction of individual lines",
 *   whose SHA-256 is checked against "canonical_sha256".
 *
 * Arrangements are then grouped by canonical form and the groups are
 * checked against the declared "classes".
 *
 * With --self-test, the canonical form of each arrangement is also
 * recomputed after random relabeling of the lines, random sign changes of
 * whole lines (which reverse their traversal direction) and the
 * reflection x -> -x; it must not change.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canon.h"
#include "geom.h"
#include "json_min.h"

#define TRIALS 5

static void die(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

/* Deterministic PRNG (xorshift64) for the self-test. */
static uint64_t rng_state = 0x9E3779B97F4A7C15ULL;

static uint64_t rng_next(void)
{
    uint64_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return rng_state = x;
}

static int rng_below(int n)
{
    return (int)(rng_next() % (uint64_t)n);
}

/* Lines of an arrangement; returns k, exits on malformed data. */
static int load_lines(const jnode *arr, zline *L)
{
    jnode *lf = json_get(arr, "lines_frac");
    if (!lf || lf->t != J_ARR || lf->n > GEOM_MAXK)
        die("bad lines_frac");
    for (size_t i = 0; i < lf->n; i++) {
        jnode *t = lf->items[i];
        const char *s[3];
        if (t->t != J_ARR || t->n != 3)
            die("bad line in lines_frac");
        for (int c = 0; c < 3; c++) {
            if (t->items[c]->t != J_STR)
                die("bad coefficient in lines_frac");
            s[c] = t->items[c]->s;
        }
        zline_init(&L[i]);
        if (zline_from_strings(&L[i], s))
            die("bad coefficient in lines_frac");
    }
    return (int)lf->n;
}

static void free_lines(zline *L, int k)
{
    for (int i = 0; i < k; i++)
        zline_clear(&L[i]);
}

/* Canonical form unchanged under relabeling, sign flips, x -> -x. */
static int self_test(const zline *L, int k)
{
    static events_t ev, ref, form;
    zline nw[GEOM_MAXK];
    int perm[GEOM_MAXK], ok = 1;
    geom_events(L, k, &ev, NULL, NULL);
    canonical_form(&ev, &ref);
    for (int i = 0; i < k; i++)
        zline_init(&nw[i]);
    for (int t = 0; t < TRIALS && ok; t++) {
        for (int i = 0; i < k; i++)
            perm[i] = i;
        for (int i = k - 1; i > 0; i--) {     /* Fisher-Yates */
            int j = rng_below(i + 1), x = perm[i];
            perm[i] = perm[j];
            perm[j] = x;
        }
        for (int i = 0; i < k; i++) {
            const zline *o = &L[perm[i]];
            mpz_set(nw[i].a, o->a);
            mpz_set(nw[i].b, o->b);
            mpz_set(nw[i].c, o->c);
            if (rng_next() >> 63) {
                mpz_neg(nw[i].a, nw[i].a);
                mpz_neg(nw[i].b, nw[i].b);
                mpz_neg(nw[i].c, nw[i].c);
            }
            mpz_neg(nw[i].a, nw[i].a);        /* reflection x -> -x */
        }
        geom_events(nw, k, &ev, NULL, NULL);
        canonical_form(&ev, &form);
        ok = canon_cmp(&form, &ref) == 0;
    }
    free_lines(nw, k);
    return ok;
}

static int tuple_cmp_void(const void *x, const void *y)
{
    return tuple_cmp(x, y);
}

/* Declared list of integer tuples, sorted; -1 if malformed. */
static int load_tuples(const jnode *a, tuple_list *out)
{
    out->n = 0;
    out->t = NULL;
    if (!a || a->t != J_ARR)
        return -1;
    out->t = calloc(a->n ? a->n : 1, sizeof *out->t);
    if (!out->t)
        die("out of memory");
    for (size_t i = 0; i < a->n; i++) {
        jnode *p = a->items[i];
        if (p->t != J_ARR || p->n > GEOM_MAXK)
            return -1;
        out->t[i].n = (int)p->n;
        for (size_t j = 0; j < p->n; j++) {
            long v;
            if (!json_long(p->items[j], &v))
                return -1;
            out->t[i].v[j] = (int)v;
        }
        out->n++;
    }
    qsort(out->t, (size_t)out->n, sizeof *out->t, tuple_cmp_void);
    return 0;
}

static int same_tuples(const tuple_list *x, const tuple_list *y)
{
    if (x->n != y->n)
        return 0;
    for (int i = 0; i < x->n; i++)
        if (tuple_cmp(&x->t[i], &y->t[i]))
            return 0;
    return 1;
}

typedef struct {
    char sha[65];
    int n;
    const char **members;
} group_t;

static int group_cmp(const void *x, const void *y)
{
    const group_t *a = x, *b = y;
    return strcmp(a->members[0], b->members[0]);
}

static int class_no(const jnode *c)
{
    long v;
    return json_long(json_get(c, "class"), &v) ? (int)v : 0;
}

static int class_cmp(const void *x, const void *y)
{
    int a = class_no(*(jnode *const *)x), b = class_no(*(jnode *const *)y);
    return (a > b) - (a < b);
}

int main(int argc, char **argv)
{
    const char *file = NULL;
    int do_self = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--self-test") == 0)
            do_self = 1;
        else if (argv[i][0] == '-' && argv[i][1] != '\0')
            die("usage: classify [file] [--self-test]");
        else if (!file)
            file = argv[i];
        else
            die("usage: classify [file] [--self-test]");
    }
    if (!file)
        file = "k17_solutions.json";

    char jerr[512];
    jnode *data = json_load_file(file, jerr, sizeof jerr);
    if (!data)
        die(jerr);
    jnode *arrs = json_get(data, "arrangements");
    jnode *classes = json_get(data, "classes");
    long target, class_count;
    if (!arrs || arrs->t != J_ARR || !classes || classes->t != J_ARR ||
        !json_long(json_get(data, "triangles"), &target) ||
        !json_long(json_get(data, "class_count"), &class_count))
        die("missing fields in the solutions file");
    int narr = (int)arrs->n, ok = 1;
    zline L[GEOM_MAXK];

    if (do_self) {
        int n = 0;
        for (int a = 0; a < narr; a++) {
            int k = load_lines(arrs->items[a], L);
            n += self_test(L, k);
            free_lines(L, k);
        }
        ok &= n == narr;
        printf("SELF-TEST: canonical form invariant on %d/%d — %s\n",
               n, narr, n == narr ? "PASS" : "FAIL");
    }

    group_t *groups = calloc((size_t)narr + 1, sizeof *groups);
    int ngroups = 0;
    static events_t ev, form;
    for (int a = 0; a < narr; a++) {
        jnode *arr = arrs->items[a];
        jnode *id = json_get(arr, "id"), *dsha = json_get(arr, "canonical_sha256");
        if (!groups || !id || id->t != J_STR)
            die("bad arrangement");
        int k = load_lines(arr, L);
        tuple_list par, multi, dpar, dmulti;
        geom_events(L, k, &ev, &par, &multi);
        free_lines(L, k);
        int cnt = geom_count_by_adjacency(&ev);
        char h[65];
        canonical_form(&ev, &form);
        canonical_sha256(&form, h);

        char errs[4096] = "";
        size_t el = 0;
        if (cnt != target)
            el += (size_t)snprintf(errs + el, sizeof errs - el, "%scount %d != %ld",
                                   el ? "; " : "", cnt, target);
        if (load_tuples(json_get(arr, "parallel_pairs"), &dpar) ||
            !same_tuples(&par, &dpar)) {
            char *r = tuple_list_repr(&par);
            el += (size_t)snprintf(errs + el, sizeof errs - el,
                                   "%sparallel pairs %s != declared", el ? "; " : "", r);
            free(r);
        }
        if (load_tuples(json_get(arr, "triple_points"), &dmulti) ||
            !same_tuples(&multi, &dmulti)) {
            char *r = tuple_list_repr(&multi);
            el += (size_t)snprintf(errs + el, sizeof errs - el,
                                   "%smultiple points %s != declared", el ? "; " : "", r);
            free(r);
        }
        if (!dsha || dsha->t != J_STR || strcmp(h, dsha->s) != 0)
            el += (size_t)snprintf(errs + el, sizeof errs - el,
                                   "%scanonical_sha256 differs from declared",
                                   el ? "; " : "");
        if (el) {
            ok = 0;
            printf("%s: FAIL — %s\n", id->s, errs);
        }
        tuple_list_free(&par);
        tuple_list_free(&multi);
        tuple_list_free(&dpar);
        tuple_list_free(&dmulti);

        int g = 0;
        while (g < ngroups && strcmp(groups[g].sha, h) != 0)
            g++;
        if (g == ngroups) {
            memcpy(groups[g].sha, h, 65);
            groups[g].members = calloc((size_t)narr, sizeof(char *));
            if (!groups[g].members)
                die("out of memory");
            ngroups++;
        }
        groups[g].members[groups[g].n++] = id->s;
    }

    qsort(groups, (size_t)ngroups, sizeof *groups, group_cmp);
    jnode **decl = malloc((classes->n + 1) * sizeof *decl);
    if (!decl)
        die("out of memory");
    memcpy(decl, classes->items, classes->n * sizeof *decl);
    /* stable sort by class number (insertion sort, few classes) */
    for (size_t i = 1; i < classes->n; i++)
        for (size_t j = i; j > 0 && class_cmp(&decl[j - 1], &decl[j]) > 0; j--) {
            jnode *t = decl[j];
            decl[j] = decl[j - 1];
            decl[j - 1] = t;
        }

    for (int g = 0; g < ngroups; g++) {
        printf("class %d: %d %s (", g + 1, groups[g].n,
               groups[g].n > 1 ? "arrangements" : "arrangement");
        for (int m = 0; m < groups[g].n; m++)
            printf(m ? " %s" : "%s", groups[g].members[m]);
        printf(")\n");
    }
    int same = (size_t)ngroups == classes->n;
    for (int g = 0; same && g < ngroups; g++) {
        jnode *c = decl[g];
        jnode *cs = json_get(c, "canonical_sha256"), *cm = json_get(c, "members");
        same = cs && cs->t == J_STR && strcmp(cs->s, groups[g].sha) == 0 &&
               cm && cm->t == J_ARR && cm->n == (size_t)groups[g].n;
        for (int m = 0; same && m < groups[g].n; m++)
            same = cm->items[m]->t == J_STR &&
                   strcmp(cm->items[m]->s, groups[g].members[m]) == 0;
    }
    if (!same)
        printf("declared classes differ from the computed ones\n");
    ok &= same && ngroups == class_count;
    printf("CLASSES: %d distinct, %d arrangements — %s\n", ngroups, narr,
           ok ? "PASS" : "FAIL");

    for (int g = 0; g < ngroups; g++)
        free(groups[g].members);
    free(groups);
    free(decl);
    json_free(data);
    return ok ? 0 : 1;
}
