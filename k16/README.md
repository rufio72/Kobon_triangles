# k16_final — 307 optimal 72-triangle arrangements of 16 lines

This package contains **307 pairwise non-isomorphic arrangements of 16
straight lines**, each forming **72 non-overlapping triangles** — the
proven maximum for 16 lines.

Before this work, **one** such arrangement was publicly known.

| | |
|---|---|
| Lines (*k*) | 16 |
| Triangles per arrangement | 72 |
| Proven upper bound | 72 — these arrangements are **optimal** |
| Arrangements in this package | **307**, pairwise non-isomorphic |
| — with degeneracies (own folder + figure) | 28 |
| — simple (bundled in one JSON) | 279 |
| Arithmetic | exact rationals, no floating point |
| Independent checks per arrangement | 3 |

> **Status: not yet independently confirmed.** Every arrangement passes
> the verification scripts included here (three independent counting
> methods, exact rational arithmetic), but nothing in this package has
> been checked by anyone outside the project. Please regard the data as
> **verified in-house, pending external review** — and see
> [Verify it yourself](#verify-it-yourself): it takes one command.

## Why 72 is the whole story

For the Kobon triangle problem, `N(k)` is the largest number of
non-overlapping triangles formed by an arrangement of `k` lines.

* Tamura's bound: `N(k) ≤ ⌊k(k−2)/3⌋` — for `k = 16` this gives 74.
* For even `k` the stronger Bartholdi–Blanc–Loisel bound applies:
  `N(k) ≤ ⌊k(k−7/3)/3⌋` — for `k = 16` this gives **72**.

Since arrangements with 72 triangles exist, `N(16) = 72` is settled:
**73 and 74 are impossible.** Consequently this package does not claim
a record — records are not available at `k = 16`. What it contributes
is *abundance and variety*: 307 essentially different ways to reach the
optimum, where previously one was published.

## Layout

```
k16_final/
├── sol1/ … sol28/            arrangements WITH degeneracies
│   ├── figure.png            two panels: full view + zoom on the dense core
│   ├── figure.svg            vector version, zoom without limits
│   └── lines_rational.json   exact rational coefficients + declared structure
├── simple_solutions.json     the 279 simple arrangements, all in one file
├── verify_direct_exact.py    verifier 1 — classical definition
├── verify_events.py          verifier 2 — crossing-order (event) based
├── verify_bundle.py          verifier 3 — runs over simple_solutions.json
├── SHA256SUMS.txt
└── LICENSE
```

Only the 28 degenerate arrangements get a folder and a picture: they
are the structurally interesting minority, and a figure per arrangement
would otherwise mean 307 near-identical pictures. The 279 simple ones
live in a single JSON file, fully specified and fully verifiable.

Line coefficients are given as exact rationals `(a, b, c)` meaning
`a·x + b·y = c`. The `lines_float` fields are a convenience for
plotting only — **all verification uses `lines_frac`**.

## The 28 degenerate arrangements

A *triple point* is three lines through one point; a *parallel pair* is
two lines that never meet. Both are allowed by the problem and both
appear in the previously published solution, which has one parallel
pair.

| structure | count | folders |
|---|---|---|
| **3 triple points** | **1** | **sol1** |
| 2 triple points | 3 | sol2 – sol4 |
| 1 triple point + 1 parallel pair | 1 | sol5 |
| 1 triple point | 14 | sol6 – sol19 |
| 1 parallel pair | 9 | sol20 – sol28 |

`sol1` is the most degenerate arrangement we found at this size.

## How the arrangements were found

1. **Search.** Simulated annealing over pseudoline arrangements
   (crossing orders), with the temperature plateau calibrated for this
   problem size. Once calibrated, essentially every random seed reaches
   72 triangles, and almost every seed reaches a *different* one.
2. **Realization.** Each pseudoline arrangement is realized with actual
   straight lines: a numeric descent fixes the crossing orders, with
   triple points imposed by construction (a dependent line is *defined*
   through the intersection of two supports, never fitted to it) and
   parallel classes sharing a direction.
3. **Exact certification.** The realized lines are recomputed in exact
   rational arithmetic; crossing orders are re-extracted with exact
   comparisons and matched against the target; triangles are recounted.
   Anything that fails at any step is discarded, not repaired.

A failure to realize a pseudoline arrangement is never evidence of
non-realizability — such cases were simply dropped.

## What "non-isomorphic" means here

Two arrangements are considered the same if one can be obtained from
the other by **relabeling the lines** and **reversing the traversal
direction of individual lines**. All 307 arrangements are pairwise
distinct in this sense, and all 307 are distinct from the previously
published one. The check is exhaustive: candidates are first separated
by a combinatorial invariant, then every surviving collision is decided
by an exact isomorphism search.

## Verify it yourself

Python 3, standard library only, no dependencies:

```bash
# one degenerate arrangement (two independent methods)
python3 verify_direct_exact.py sol1/lines_rational.json
python3 verify_events.py       sol1/lines_rational.json

# all 279 simple arrangements at once
python3 verify_bundle.py

# every degenerate arrangement
for d in sol*/; do python3 verify_direct_exact.py "$d/lines_rational.json"; done

# file integrity
sha256sum -c SHA256SUMS.txt
```

Each verifier prints a line ending in `PASS`.

* `verify_direct_exact.py` — the classical definition: a triple of
  lines counts iff its three pairwise intersection points are distinct
  and no other line meets the open interior.
* `verify_events.py` — an independent route: builds each line's
  sequence of crossings and counts triangles from adjacency in those
  sequences.
* `verify_bundle.py` — applies the classical definition to every
  arrangement in `simple_solutions.json`, and additionally checks that
  each one really is simple (no three lines concurrent, no two lines
  parallel).

**Counting convention.** A line passing exactly through a vertex only
*touches* the triangle, which still counts; several triangles may meet
at a triple point. This is the convention of the published record
solutions (e.g. the 15-triangle solution for `k = 8`).

## Citing

If you use this data, please cite it as:

> A. Maiorana, *307 optimal 72-triangle arrangements of 16 lines*
> (Kobon triangle problem), 2026. Package `k16_final`.

## License

© 2026 Andrea Maiorana. Data, figures and scripts are released under the
[Creative Commons Attribution 4.0 International license][cc] (CC BY
4.0) — see [LICENSE](LICENSE). You may share and adapt freely,
including commercially, with appropriate credit.

Developed by the author with the help of Claude Code (Anthropic).

[cc]: https://creativecommons.org/licenses/by/4.0/
