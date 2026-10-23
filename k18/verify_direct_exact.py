#!/usr/bin/env python3
"""Verifier 1/2 — DIRECT classical definition, exact arithmetic.

Usage:           python3 verify_direct_exact.py [solN/lines_rational.json]
Requirements:    Python 3 standard library only.
Expected output: last line  DIRECT EXACT COUNT: <count> — PASS

Loads the lines (field "lines_frac", exact rationals a, b, c with
a*x + b*y = c) and counts Kobon triangles straight from the classical
definition, entirely in fractions.Fraction: a triple of lines counts
iff its three pairwise intersection points are distinct (nonzero area)
and no other line meets the open interior — i.e. no other line strictly
separates two of the three vertices. A line passing through a vertex
only touches the triangle, which still counts; this is the convention
of the published record solutions.

Also checks: pairwise distinct lines; the parallel pairs are exactly
the declared ones (field "declared_parallel_pairs").
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
    par_dic = {tuple(sorted(p))
               for p in data.get("declared_parallel_pairs", [])}

    for (i, li), (j, lj) in itertools.combinations(enumerate(lines), 2):
        assert not (li[0] * lj[1] == lj[0] * li[1] and
                    li[1] * lj[2] == lj[1] * li[2] and
                    li[0] * lj[2] == lj[0] * li[2]), \
            f"lines {i} and {j} coincide"

    P = {}
    par = set()
    for i, j in itertools.combinations(range(k), 2):
        a1, b1, c1 = lines[i]
        a2, b2, c2 = lines[j]
        det = a1 * b2 - a2 * b1
        if det == 0:
            par.add((i, j))
            continue
        P[(i, j)] = ((c1 * b2 - c2 * b1) / det, (a1 * c2 - a2 * c1) / det)
    assert par == par_dic, f"parallel pairs {sorted(par)} != declared"
    print(f"structure OK: {k} distinct lines, "
          f"{len(par)} parallel pairs (as declared)")

    count = 0
    for i, j, l in itertools.combinations(range(k), 3):
        if (i, j) not in P or (i, l) not in P or (j, l) not in P:
            continue
        v = [P[(i, j)], P[(i, l)], P[(j, l)]]
        (x1, y1), (x2, y2), (x3, y3) = v
        if (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1) == 0:
            continue                    # concurrent triple: zero area
        empty = True
        for m in range(k):
            if m in (i, j, l):
                continue
            a, b, c = lines[m]
            s = [a * x + b * y - c for x, y in v]
            if (s[0] < 0 < s[1]) or (s[1] < 0 < s[0]) or \
               (s[0] < 0 < s[2]) or (s[2] < 0 < s[0]) or \
               (s[1] < 0 < s[2]) or (s[2] < 0 < s[1]):
                empty = False
                break
        count += empty
    status = "PASS" if count == atteso else "FAIL"
    print(f"DIRECT EXACT COUNT: {count} — {status}")
    return 0 if count == atteso else 1


if __name__ == "__main__":
    sys.exit(main())
