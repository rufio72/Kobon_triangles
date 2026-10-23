#!/usr/bin/env python3
"""Verifier 3/3 — checks EVERY arrangement in simple_solutions.json.

Usage:           python3 verify_bundle.py [simple_solutions.json]
Requirements:    Python 3 standard library only.
Expected output: last line  ALL <n> SOLUTIONS: 72 TRIANGLES — PASS

For each arrangement it counts Kobon triangles straight from the
classical definition, entirely in fractions.Fraction (no floating
point): a triple of lines counts iff its three pairwise intersection
points are distinct (nonzero area) and no other line meets the open
interior — i.e. no other line strictly separates two of the three
vertices. A line passing through a vertex only touches the triangle,
which still counts; this is the convention of the published record
solutions.

It also checks that every arrangement really is SIMPLE (no three lines
concurrent, no two lines parallel), as claimed in the file.
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


def count(lines):
    k = len(lines)
    P = {}
    for i, j in combinations(range(k), 2):
        P[(i, j)] = inter(lines[i], lines[j])
    n = 0
    for i, j, m in combinations(range(k), 3):
        v = [P[(i, j)], P[(i, m)], P[(j, m)]]
        if any(p is None for p in v):
            continue
        if v[0] == v[1] or v[0] == v[2] or v[1] == v[2]:
            continue
        blocked = False
        for q in range(k):
            if q in (i, j, m):
                continue
            s = [side(lines[q], p) for p in v]
            pos = sum(1 for x in s if x > 0)
            neg = sum(1 for x in s if x < 0)
            if pos and neg:            # strictly separates two vertices
                blocked = True
                break
        if not blocked:
            n += 1
    return n


def is_simple(lines):
    k = len(lines)
    for i, j in combinations(range(k), 2):
        if inter(lines[i], lines[j]) is None:
            return False, f"lines {i} and {j} are parallel"
    pts = {}
    for i, j in combinations(range(k), 2):
        pts.setdefault(inter(lines[i], lines[j]), []).append((i, j))
    for p, who in pts.items():
        if len(who) > 1:
            return False, f"concurrent lines at {p}: {who}"
    return True, ""


def main():
    fn = sys.argv[1] if len(sys.argv) > 1 else "simple_solutions.json"
    with open(fn) as f:
        data = json.load(f)
    sols = data["solutions"]
    expected = data.get("triangles", 72)
    bad = 0
    for s in sols:
        lines = [tuple(Fraction(x) for x in ln) for ln in s["lines_frac"]]
        ok, why = is_simple(lines)
        if not ok:
            print(f"solution {s['id']}: NOT SIMPLE — {why}")
            bad += 1
            continue
        n = count(lines)
        if n != expected:
            print(f"solution {s['id']}: {n} triangles "
                  f"(expected {expected}) — FAIL")
            bad += 1
    if bad:
        print(f"{bad} SOLUTION(S) FAILED")
        return 1
    print(f"ALL {len(sols)} SOLUTIONS: {expected} TRIANGLES — PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
