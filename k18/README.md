# k18_final — 2357 arrangements of 18 lines with 93 triangles

This package contains **2357 pairwise non-isomorphic arrangements of
18 straight lines**, each forming **93 non-overlapping
triangles** — the best count known for 18 lines.

| | |
|---|---|
| Lines (*k*) | 18 |
| Triangles per arrangement | 93 |
| Best previously published | 93 (a single arrangement, by Bader) |
| Proven upper bound | 94 — **not known to be attainable** |
| Arrangements in this package | **2357**, pairwise non-isomorphic |
| — with degeneracies | 287 (20 of them with a folder and figures) |
| — simple | 2070 |
| Arithmetic | exact rationals, no floating point |
| Independent checks per arrangement | 3 |

> **Status: not yet independently confirmed.** Every arrangement passes
> the verification scripts included here (three independent counting
> methods, exact rational arithmetic), but nothing in this package has
> been checked by anyone outside the project. Please regard the data as
> **verified in-house, pending external review** — and see
> [Verify it yourself](#verify-it-yourself): it takes one command.

## What this is, and what it is not

For the Kobon triangle problem, `N(k)` is the largest number of
non-overlapping triangles formed by an arrangement of `k` lines.

At `k = 18` the situation is genuinely open:

* Tamura's bound gives `N(18) ≤ ⌊18·16/3⌋ = 96`.
* For even `k` the stronger Bartholdi–Blanc–Loisel bound applies:
  `N(18) ≤ ⌊18·(18 − 7/3)/3⌋ = 94`.
* The best construction known is **93**. So `N(18)` is either
  93 or 94, and **nobody knows which.**

**This package does not close that gap.** It does not contain a
94-triangle arrangement and it does not prove that none exists.
What it contributes is *abundance and variety at the 93 level*:
2357 essentially different ways to reach 93, where previously
one was published.

### About 94: what we can and cannot say

Across **7,004 recorded annealing runs** at `k = 18`, the value
93 was reached **2,776 times** and 94 was reached
**never, not once** — including runs deliberately constrained to the
degeneracy patterns (94 would require a very particular structure)
and long runs at the cold-start temperature that makes 93 routine.

This is **evidence, not proof**. A stochastic search that never finds
something has not shown that the thing does not exist; it has shown
that the thing is not easy to find this way. Anyone reading this table
as "N(18) = 93" is reading more into it than it says. Settling
`k = 18` needs an exhaustive method (SAT over crossing orders, as
Savchuk used to settle `k = 11`), not more annealing.

## Where 94 provably cannot be (update 2026-08-15)

The exhaustive method has now been applied to the **symmetric** part
of the search space, using **Pavlo Savchuk's kobon-cnf model**
(arXiv:2507.07951, [zegalur/kobon-cnf](https://github.com/zegalur/kobon-cnf))
with Kissat 4.0.4 and DRAT-verified proofs. Machine-checked results,
full details and reproduction scripts in
[`impossibility/`](impossibility/):

* **No simple C3-symmetric arrangement of 18 pseudolines forms 94
  triangles** — the C3-symmetric simple maximum is **exactly 93**
  (SAT at 93 in 3 minutes; UNSAT at 94 in 99 minutes, 2.1 GB DRAT
  proof verified). C3 was the *only* rotational symmetry whose orbit
  arithmetic is compatible with 94.
* The mirror family with fixed lines (axis in the arrangement plus one
  perpendicular line) tops out **below 87**: UNSAT with verified
  proofs at every target from 94 down to 87.
* Rotations C2, C4, C6, C9, C18 are excluded by orbit arithmetic
  alone (e.g. under C4 every triangle orbit has size 4, so the count
  is a multiple of 4 — at most 92).
* The last symmetric family — mirror with **9 free pairs and no fixed
  line**, count necessarily even, `94 = 47 pairs` — is **UNSAT** as
  well (58 minutes, 1.7 GB DRAT proof verified).

With that, the classification is **complete**: every nontrivial
planar symmetry is either excluded by orbit arithmetic or closed by a
machine-verified certificate. **A simple 94-triangle arrangement of
18 pseudolines, if it exists, is asymmetric.**

These results cover simple pseudoline arrangements (hence also
straight lines). Degenerate symmetric cases stay open, except a
triple point at a C3 center (arithmetically capped at 93). The
asymmetric bulk of the search space remains untouched: **the gap
"93 or 94" is still open**, but it is now provably not hiding behind
any natural symmetry.

As a by-product, the symmetric search produced **70 new
arrangements**, all with combinatorial 120°-rotation symmetry and all
isomorphic to **none** of the 2,337 in this package (exact canonical
forms — free random search never lands on the measure-zero symmetric
subspace, so symmetry had to be imposed to find them). 69 come from
the C3 orbital annealing kernel — 48 of them straightened to real
lines with exact rational certification — and are bundled in
[`c3_symmetric/`](c3_symmetric/); the 70th was found by the SAT model
itself ([lines](c3_symmetric_93_lines.json), lines colored by
rotation orbit in the figure):

![A 93-triangle arrangement of 18 real lines with combinatorial
3-fold rotational symmetry, lines colored by orbit](c3_symmetric_93.svg)

## Layout

```
k18_final/
├── sol1/ … sol20/          the 20 structurally richest arrangements
│   ├── figure.png            two panels: full view + zoom on the dense core
│   ├── figure.svg            vector version, zoom without limits
│   └── lines_rational.json   exact rational coefficients + declared structure
├── all_solutions.json    the other 2337 arrangements, all in one file (6 MB)
├── verify_direct_exact.py    verifier 1 — classical definition
├── verify_events.py          verifier 2 — crossing-order (event) based
├── verify_bundle.py          verifier 3 — runs over all_solutions.json
├── SHA256SUMS.txt
└── LICENSE
```

Only 20 arrangements get a folder and a picture — the ones with
the richest degenerate structure. A figure per arrangement would
otherwise mean 2357 near-identical pictures. Everything else lives in
a single JSON file, fully specified and fully verifiable; of those,
267 also have degeneracies, declared per arrangement in the
`triple_points` and `parallel_pairs` fields.

Line coefficients are given as exact rationals `(a, b, c)` meaning
`a·x + b·y = c`. The `lines_float` fields are a convenience for
plotting only — **all verification uses `lines_frac`**.

## The 20 arrangements with figures

A *triple point* is three lines through one point; a *parallel pair* is
two lines that never meet. Both are allowed by the problem, and both
appear in previously published record solutions.

| structure | count | folders |
|---|---|---|
| 4 triple points | 1 | sol1 |
| 3 triple points | 3 | sol2 – sol4 |
| 2 triple points, 1 parallel pair | 1 | sol5 |
| 2 triple points | 15 | sol6 – sol20 |

## How the arrangements were found

1. **Search.** Simulated annealing over pseudoline arrangements
   (crossing orders), run from a deliberately cold start (temperature
   plateau `T0 = 0.8`). That calibration is what makes 93 routine
   rather than rare, and nearly every seed lands on a *different*
   arrangement.
2. **Realization.** Each pseudoline arrangement is realized with actual
   straight lines: a numeric descent fixes the crossing orders, with
   triple points imposed by construction (a dependent line is *defined*
   through the intersection of two supports, never fitted to it) and
   parallel classes sharing a direction.
3. **Exact certification.** The realized lines are recomputed in exact
   rational arithmetic; crossing orders are re-extracted with exact
   comparisons and matched against the target; triangles are recounted
   by a second, independent method. Anything that fails at any step is
   discarded, not repaired.

Not every pseudoline arrangement we found could be realized with
straight lines. **A failure to realize one is never evidence of
non-realizability** — such cases were simply dropped, and they are not
in this package.

## What "non-isomorphic" means here

Two arrangements are considered the same if one can be obtained from
the other by **relabeling the lines** and **reversing the traversal
direction of individual lines**. All 2357 arrangements are pairwise
distinct in this sense, and all 2357 are distinct from Bader's
published 93. The check is exhaustive: candidates are first
separated by a combinatorial invariant, then every surviving collision
is decided by an exact isomorphism search.

## Verify it yourself

Python 3, standard library only, no dependencies:

```bash
# one arrangement, two independent methods
python3 verify_direct_exact.py sol1/lines_rational.json
python3 verify_events.py       sol1/lines_rational.json

# every arrangement that has a folder
for d in sol*/; do python3 verify_direct_exact.py "$d/lines_rational.json"; done

# all 2337 bundled arrangements (slow: use several cores)
python3 verify_bundle.py --jobs=4

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
  arrangement in `all_solutions.json`, and additionally recomputes each one's
  multiple points and parallel pairs and checks they are exactly the
  declared ones. Whole-bundle runs take tens of minutes; `--jobs=N`
  spreads them over N cores.

**Counting convention.** A line passing exactly through a vertex only
*touches* the triangle, which still counts; several triangles may meet
at a triple point. This is the convention of the published record
solutions (e.g. the 15-triangle solution for `k = 8`).

## Citing

If you use this data, please cite it as:

> A. Maiorana, *2357 non-isomorphic 93-triangle arrangements of
> 18 lines* (Kobon triangle problem), 2026. Package `k18_final`.

## License

© 2026 Andrea Maiorana. Data, figures and scripts are released under the
[Creative Commons Attribution 4.0 International license][cc] (CC BY
4.0) — see [LICENSE](LICENSE). You may share and adapt freely,
including commercially, with appropriate credit.

Developed by the author with the help of Claude Code (Anthropic).

[cc]: https://creativecommons.org/licenses/by/4.0/
