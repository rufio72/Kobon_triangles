#!/usr/bin/env python3
"""Verifier 3/3 — checks EVERY arrangement in all_solutions.json.

Usage:           python3 verify_bundle.py [all_solutions.json] [--jobs N]
Requirements:    Python 3 standard library only.
Expected output: last line  ALL <n> ARRANGEMENTS: 93 TRIANGLES — PASS

For each arrangement it counts Kobon triangles straight from the
classical definition, entirely in fractions.Fraction (no floating
point): a triple of lines counts iff its three pairwise intersection
points are distinct (nonzero area) and no other line meets the open
interior — i.e. no other line strictly separates two of the three
vertices. A line passing through a vertex only touches the triangle,
which still counts; this is the convention of the published record
solutions.

It also recomputes the degeneracies of every arrangement — the multiple
points (three or more lines through one point) and the parallel pairs —
and checks they are EXACTLY the ones declared in the file. An
arrangement declared simple must really be simple.

This is a lot of exact arithmetic: expect roughly 20-40 minutes
single-threaded. Pass --jobs N to spread it over N cores.
"""
import json
import sys
from fractions import Fraction
from itertools import combinations


def inter(l1, l2):
    a1, b1, c1 = l1
    a2, b2, c2 = l2
    det = a1 * b2 - a2 * b1
    if det == 0:
        return None
    return ((c1 * b2 - c2 * b1) / det, (a1 * c2 - a2 * c1) / det)


def side(line, p):
    a, b, c = line
    return a * p[0] + b * p[1] - c


def count(lines, P):
    k = len(lines)
    n = 0
    for i, j, m in combinations(range(k), 3):
        v = [P.get((i, j)), P.get((i, m)), P.get((j, m))]
        if any(p is None for p in v):
            continue
        if v[0] == v[1] or v[0] == v[2] or v[1] == v[2]:
            continue
        blocked = False
        for q in range(k):
            if q in (i, j, m):
                continue
            s = [side(lines[q], p) for p in v]
            if any(x > 0 for x in s) and any(x < 0 for x in s):
                blocked = True          # strictly separates two vertices
                break
        if not blocked:
            n += 1
    return n


def degeneracies(lines):
    """Returns (multiple points, parallel pairs), both recomputed."""
    k = len(lines)
    P, par, pts = {}, set(), {}
    for i, j in combinations(range(k), 2):
        p = inter(lines[i], lines[j])
        if p is None:
            par.add((i, j))
            continue
        P[(i, j)] = p
        pts.setdefault(p, set()).update((i, j))
    multi = sorted(tuple(sorted(who)) for who in pts.values()
                   if len(who) > 2)
    return P, multi, sorted(par)


def check(sol, expected):
    lines = [tuple(Fraction(x) for x in ln) for ln in sol["lines_frac"]]
    P, multi, par = degeneracies(lines)
    dec_m = sorted(tuple(t) for t in sol.get("triple_points", []))
    dec_p = sorted(tuple(p) for p in sol.get("parallel_pairs", []))
    if multi != dec_m:
        return (f"arrangement {sol['id']}: multiple points {multi} "
                f"!= declared {dec_m} — FAIL")
    if par != dec_p:
        return (f"arrangement {sol['id']}: parallel pairs {par} "
                f"!= declared {dec_p} — FAIL")
    n = count(lines, P)
    if n != expected:
        return (f"arrangement {sol['id']}: {n} triangles "
                f"(expected {expected}) — FAIL")
    return None


def main():
    argv = [a for a in sys.argv[1:] if not a.startswith("--")]
    jobs = 1
    for a in sys.argv[1:]:
        if a.startswith("--jobs"):
            jobs = int(a.split("=")[1]) if "=" in a else \
                int(sys.argv[sys.argv.index(a) + 1])
    fn = argv[0] if argv and not argv[0].isdigit() else "all_solutions.json"
    with open(fn) as f:
        data = json.load(f)
    sols = data["solutions"]
    expected = data.get("triangles", 93)
    print(f"checking {len(sols)} arrangements of {data.get('k', 18)} "
          f"lines, expecting {expected} triangles each "
          f"({jobs} core{'s' if jobs > 1 else ''})", flush=True)

    bad = []
    if jobs > 1:
        from multiprocessing import Pool
        from functools import partial
        with Pool(jobs) as pool:
            it = pool.imap(partial(check, expected=expected), sols,
                           chunksize=4)
            for n, r in enumerate(it, 1):
                if r:
                    print(r, flush=True)
                    bad.append(r)
                if n % 100 == 0:
                    print(f"  {n}/{len(sols)}", flush=True)
    else:
        for n, s in enumerate(sols, 1):
            r = check(s, expected)
            if r:
                print(r, flush=True)
                bad.append(r)
            if n % 100 == 0:
                print(f"  {n}/{len(sols)}", flush=True)

    if bad:
        print(f"{len(bad)} ARRANGEMENT(S) FAILED")
        return 1
    print(f"ALL {len(sols)} ARRANGEMENTS: {expected} TRIANGLES — PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
