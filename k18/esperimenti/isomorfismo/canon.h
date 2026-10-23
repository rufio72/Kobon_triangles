/* Complete canonical form of an event structure, and its SHA-256.
 *
 * Equal forms <=> isomorphic arrangements, for the relation "relabel
 * the lines and reverse the traversal direction of individual lines".
 */
#ifndef CANON_H
#define CANON_H

#include "geom.h"

/* Lexicographically least relabeled event structure of ev into form. */
void canonical_form(const events_t *ev, events_t *form);

/* Lexicographic comparison of nested sequences (-1, 0, 1); at every
 * level a shorter prefix is smaller, as for Python tuples. */
int canon_cmp(const events_t *x, const events_t *y);

/* Compact JSON of a form, e.g. "[[[3],[5]],[[0],[2]]]"; malloc'd. */
char *canon_json(const events_t *form);

/* SHA-256 (lowercase hex) of canon_json(form). */
void canonical_sha256(const events_t *form, char out[65]);

#endif
