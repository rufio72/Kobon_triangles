#!/usr/bin/env python3
"""Verifier 2/2 — EVENT/WIRING representation, exact arithmetic.

Usage:           python3 verify_events.py [solN/lines_rational.json]
Requirements:    Python 3 standard library only.
Expected output: last line  EVENT-BASED COUNT: <count> — PASS

Loads the lines and re-derives, in exact rational arithmetic, the
combinatorial structure of the arrangement: for each line, the sequence
of crossing events along it, where an event is the set of lines met at
one point (multiple points grouped by exact coincidence of the crossing
parameter).

Checks that the multiple points are EXACTLY the declared ones (field
"declared_triple_points") and the parallel pairs the declared ones,
then counts triangles by the adjacency criterion: a triple (x,y,z)
counts iff every pair crosses, the three crossings are not one single
point, and on each of the three lines the two relevant events are
CONSECUTIVE in the sequence. Vertex-sharing triangles at a multiple
point all count (convention of the published record solutions).
"""
import itertools
import json
import sys
from fractions import Fraction


def main():
    fn = sys.argv[1] if len(sys.argv) > 1 else "lines_rational.json"
    with open(fn) as f:
        data = json.load(f)
    lines = [tuple(Fraction(x) for x in ln) for ln in data["lines_frac"]]
    k = len(lines)
    atteso = data["count"]
    tri_dic = sorted(tuple(t) for t in data["declared_triple_points"])
    par_dic = {tuple(sorted(p))
               for p in data.get("declared_parallel_pairs", [])}

    events = []
    par = set()
    for i in range(k):
        a1, b1, c1 = lines[i]
        buckets = {}
        for j in range(k):
            if j == i:
                continue
            a2, b2, c2 = lines[j]
            det = a1 * b2 - a2 * b1
            if det == 0:
                par.add(tuple(sorted((i, j))))
                continue
            x = (c1 * b2 - c2 * b1) / det
            y = (a1 * c2 - a2 * c1) / det
            s = -b1 * x + a1 * y
            buckets.setdefault(s, set()).add(j)
        events.append([frozenset(buckets[s]) for s in sorted(buckets)])
    assert par == par_dic, f"parallel pairs {sorted(par)} != declared"

    multi = sorted({tuple(sorted({i} | set(e)))
                    for i, row in enumerate(events)
                    for e in row if len(e) >= 2})
    assert multi == tri_dic, \
        f"multiple-point structure {multi} != declared {tri_dic}"
    print(f"structure OK: multiple points {multi} as declared, "
          f"all other crossings simple")

    pos = [dict() for _ in range(k)]
    for i in range(k):
        for idx, ev in enumerate(events[i]):
            for j in ev:
                pos[i][j] = idx
    count = 0
    for x, y, z in itertools.combinations(range(k), 3):
        if y not in pos[x] or z not in pos[x] or z not in pos[y]:
            continue                    # a parallel pair
        if pos[x][y] == pos[x][z]:
            continue                    # all three through one point
        if abs(pos[x][y] - pos[x][z]) == 1 and \
           abs(pos[y][x] - pos[y][z]) == 1 and \
           abs(pos[z][x] - pos[z][y]) == 1:
            count += 1
    status = "PASS" if count == atteso else "FAIL"
    print(f"EVENT-BASED COUNT: {count} — {status}")
    return 0 if count == atteso else 1


if __name__ == "__main__":
    sys.exit(main())
