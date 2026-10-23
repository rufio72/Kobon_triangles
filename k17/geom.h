/* Exact geometry of line arrangements: lines, crossing events, adjacency.
 *
 * A line a*x + b*y = c with rational a, b, c is scaled by the positive
 * least common multiple of the three denominators, giving an integer
 * triple. A positive factor preserves coincidences, parallelism, the
 * side of a point and the order of points along the line.
 */
#ifndef GEOM_H
#define GEOM_H

#include <gmp.h>

#define GEOM_MAXK 64           /* maximum number of lines */

typedef struct {
    mpz_t a, b, c;
} zline;

/* Event structure: for every line i, the sequence of events along it;
 * event e of line i holds the lines mem[i][start[i][e] .. start[i][e+1])
 * in increasing order. Also used for canonical forms. */
typedef struct {
    int k;
    int nev[GEOM_MAXK];
    int start[GEOM_MAXK][GEOM_MAXK + 1];
    int mem[GEOM_MAXK][GEOM_MAXK];
} events_t;

/* A sorted tuple of line indices (parallel pair or multiple point). */
typedef struct {
    int n;
    int v[GEOM_MAXK];
} tuple_t;

typedef struct {
    int n;
    tuple_t *t;
} tuple_list;

/* Parse "p/q" or "p" (optional '-' on p, q > 0); 0 on success. */
int geom_parse_rat(mpq_t q, const char *s);

void zline_init(zline *l);
void zline_clear(zline *l);
/* Integer line from three rationals; 0 on success, -1 if a = b = 0. */
int zline_from_q(zline *l, const mpq_t a, const mpq_t b, const mpq_t c);
/* Integer line from three "p/q" strings; 0 on success. */
int zline_from_strings(zline *l, const char *s[3]);

/* Events along every line, parallel pairs (sorted pairs, sorted list)
 * and multiple points (sorted tuples of >= 3 lines, sorted list).
 * par and multi may be NULL; otherwise free them with tuple_list_free. */
void geom_events(const zline *lines, int k, events_t *ev,
                 tuple_list *par, tuple_list *multi);

/* Triangle count by adjacency of events along each line. */
int geom_count_by_adjacency(const events_t *ev);

/* Lexicographic comparison of tuples (a shorter prefix is smaller). */
int tuple_cmp(const tuple_t *x, const tuple_t *y);
void tuple_list_free(tuple_list *l);
/* Python-like repr of a list of tuples, e.g. "[(0, 1), (2, 3)]";
 * returns a malloc'd string. */
char *tuple_list_repr(const tuple_list *l);

#endif
