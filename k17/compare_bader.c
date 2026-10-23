/* Comparison of Bader's 17-line, 85-triangle arrangement with the 8 classes.
 *
 * Usage:           ./compare_bader [bader17_table.json]
 *                      [k17_solutions.json] [--self-test]
 * Requirements:    a C compiler and GMP.
 * Expected output: last line
 *                  BADER: class 5 (k17_05 k17_09 k17_10 k17_14 k17_15) — MATCH
 *
 * The table in bader17_table.json is the combinatorial description of
 * Johannes Bader's arrangement as published in the LineOrder gallery of
 * Savchuk (https://zegalur.github.io/line-order/gallery/kobon.html,
 * retrieved 2026-10-23): row i lists, in order along line i, the 1-based
 * labels of the 16 lines it crosses. The file is read as data only and
 * validated. Realizability of the table by straight lines is not checked
 * here.
 *
 * The table is turned into the event structure used by classify (one
 * crossing per event, 0-based labels), the adjacency count must be 85, and
 * the canonical form of classify is computed and looked up among the
 * declared classes of k17_solutions.json. The comparison is therefore
 * combinatorial: it concerns the affine crossing order of the lines, up
 * to relabeling and reversal of individual lines (which also covers
 * reflections of the plane).
 *
 * With --self-test, the canonical form is also recomputed after random
 * relabelings of the lines and random reversals of rows; it must not
 * change.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canon.h"
#include "geom.h"
#include "json_min.h"

#define N 17
#define TRIANGLES 85
#define TRIALS 5

/* Print a validation error and exit with code 1. */
static void fail(const char *fmt, int a, int b)
{
    printf("ERROR: ");
    printf(fmt, a, b);
    printf("\n");
    exit(1);
}

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

/* Read and validate the local-sequence table; 1-based rows. */
static void load_table(const char *path, int table[N][N - 1])
{
    char jerr[512];
    jnode *data = json_load_file(path, jerr, sizeof jerr);
    long v;
    if (!data)
        die(jerr);
    if (data->t != J_OBJ)
        fail("top level is not an object", 0, 0);
    if (!json_long(json_get(data, "n"), &v) || v != N ||
        !json_long(json_get(data, "triangles"), &v) || v != TRIANGLES)
        fail("n/triangles are not %d/%d", N, TRIANGLES);
    jnode *t = json_get(data, "table");
    if (!t || t->t != J_ARR || t->n != N)
        fail("table is not a list of %d rows", N, 0);
    for (int i = 0; i < N; i++) {
        jnode *row = t->items[i];
        int seen[N + 2] = { 0 }, ok = row->t == J_ARR && row->n == N - 1;
        for (size_t j = 0; ok && j < row->n; j++) {
            ok = json_long(row->items[j], &v) && v >= 1 && v <= N &&
                 v != i + 1 && !seen[v];
            if (ok) {
                seen[v] = 1;
                table[i][j] = (int)v;
            }
        }
        if (!ok)
            fail("row %d is not a permutation of 1..17 without %d", i + 1, i + 1);
    }
    json_free(data);
}

/* 1-based rows -> events (0-based, one line per event). */
static void to_events(int table[N][N - 1], events_t *ev)
{
    ev->k = N;
    for (int i = 0; i < N; i++) {
        ev->nev[i] = N - 1;
        for (int e = 0; e < N - 1; e++) {
            ev->start[i][e] = e;
            ev->mem[i][e] = table[i][e] - 1;
        }
        ev->start[i][N - 1] = N - 1;
    }
}

/* Canonical form unchanged under relabeling and row reversals. */
static int self_test(int table[N][N - 1], const events_t *ref)
{
    static events_t ev, form;
    int perm[N], nw[N][N - 1];
    for (int t = 0; t < TRIALS; t++) {
        for (int i = 0; i < N; i++)
            perm[i] = i + 1;
        for (int i = N - 1; i > 0; i--) {     /* old label j -> perm[j-1] */
            int j = (int)(rng_next() % (uint64_t)(i + 1)), x = perm[i];
            perm[i] = perm[j];
            perm[j] = x;
        }
        for (int i = 0; i < N; i++) {
            int rev = (int)(rng_next() >> 63);
            for (int e = 0; e < N - 1; e++) {
                int src = rev ? N - 2 - e : e;
                nw[perm[i] - 1][e] = perm[table[i][src] - 1];
            }
        }
        to_events(nw, &ev);
        canonical_form(&ev, &form);
        if (canon_cmp(&form, ref) != 0)
            return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    const char *pos[2] = { "bader17_table.json", "k17_solutions.json" };
    int npos = 0, do_self = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--self-test") == 0)
            do_self = 1;
        else if ((argv[i][0] == '-' && argv[i][1] != '\0') || npos == 2)
            die("usage: compare_bader [table] [solutions] [--self-test]");
        else
            pos[npos++] = argv[i];
    }

    static int table[N][N - 1];
    static events_t ev, form;
    load_table(pos[0], table);
    printf("table: %d lines, all pairs cross\n", N);
    to_events(table, &ev);
    int cnt = geom_count_by_adjacency(&ev);
    printf("triangles (adjacency): %d\n", cnt);
    if (cnt != TRIANGLES)
        fail("adjacency count %d != %d", cnt, TRIANGLES);
    char h[65];
    canonical_form(&ev, &form);
    canonical_sha256(&form, h);
    printf("canonical_sha256: %s\n", h);

    if (do_self) {
        if (!self_test(table, &form)) {
            printf("SELF-TEST: canonical form invariant — FAIL\n");
            return 1;
        }
        printf("SELF-TEST: canonical form invariant — PASS\n");
    }

    char jerr[512];
    jnode *data = json_load_file(pos[1], jerr, sizeof jerr);
    if (!data)
        die(jerr);
    jnode *classes = json_get(data, "classes");
    if (!classes || classes->t != J_ARR)
        die("missing \"classes\" in the solutions file");
    for (size_t c = 0; c < classes->n; c++) {
        jnode *cl = classes->items[c];
        jnode *cs = json_get(cl, "canonical_sha256");
        if (!cs || cs->t != J_STR || strcmp(cs->s, h) != 0)
            continue;
        jnode *num = json_get(cl, "class"), *mem = json_get(cl, "members");
        if (!num || num->t != J_NUM || !mem || mem->t != J_ARR)
            die("malformed class entry");
        printf("BADER: class %s (", num->s);
        for (size_t m = 0; m < mem->n; m++)
            printf(m ? " %s" : "%s", mem->items[m]->s ? mem->items[m]->s : "");
        printf(") — MATCH\n");
        json_free(data);
        return 0;
    }
    printf("BADER: no match — NEW CLASS (%zuth)\n", classes->n + 1);
    json_free(data);
    return 0;
}
