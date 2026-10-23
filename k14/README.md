# k14_final — Fifteen distinct 54-triangle arrangements for the Kobon triangle problem, k = 14

This package contains **fifteen pairwise non-isomorphic arrangements
of 14 straight lines**, each forming **54 non-overlapping triangles**.
Since 54 is the proven upper bound for k = 14, each arrangement — if
confirmed — is optimal, and together they would settle N(14) = 54
constructively, in fifteen essentially different ways. The best
previously published value was 53 (Bader).

> ⚠️ **Status: NOT yet independently confirmed.** The arrangements below
> pass the two verification scripts included in this package (exact
> rational arithmetic, two independent counting methods), but they have
> not yet been checked or confirmed by anyone outside this project.
> Until independent confirmation, they should be regarded as
> **candidate solutions**.

## What was found — the fifteen solutions

Solutions are pairwise distinct up to line relabeling and per-line
order reflection (isomorphism of the underlying arrangements).
Recurring structural motif: **no solution has parallel lines**, and all
but one have **exactly two triple points lying on a common line**. The
exception is solution 11, which has **four triple points** — a
structurally new type.

| sol | figure | triple points (0-based line indices) |
|-----|--------|--------------------------------------|
| 1 | <a href="sol1/figure.png"><img src="sol1/figure.png" width="120"></a> | {0,4,10}, {0,11,12} — on a common line |
| 2 | <a href="sol2/figure.png"><img src="sol2/figure.png" width="120"></a> | {2,4,13}, {2,8,11} — on a common line |
| 3 | <a href="sol3/figure.png"><img src="sol3/figure.png" width="120"></a> | {0,8,10}, {6,7,8} — on a common line |
| 4 | <a href="sol4/figure.png"><img src="sol4/figure.png" width="120"></a> | {1,5,8}, {5,6,11} — on a common line |
| 5 | <a href="sol5/figure.png"><img src="sol5/figure.png" width="120"></a> | {4,10,13}, {8,11,13} — on a common line |
| 6 | <a href="sol6/figure.png"><img src="sol6/figure.png" width="120"></a> | {1,5,10}, {3,9,10} — on a common line |
| 7 | <a href="sol7/figure.png"><img src="sol7/figure.png" width="120"></a> | {0,2,8}, {2,5,9} — on a common line |
| 8 | <a href="sol8/figure.png"><img src="sol8/figure.png" width="120"></a> | {2,8,11}, {3,4,8} — on a common line |
| 9 | <a href="sol9/figure.png"><img src="sol9/figure.png" width="120"></a> | {1,3,11}, {1,5,8} — on a common line |
| 10 | <a href="sol10/figure.png"><img src="sol10/figure.png" width="120"></a> | {2,5,8}, {8,9,11} — on a common line |
| 11 | <a href="sol11/figure.png"><img src="sol11/figure.png" width="120"></a> | {2,5,13}, {2,6,7}, {2,9,11}, {3,6,12} — **four** triple points, three on line 2 |
| 12 | <a href="sol12/figure.png"><img src="sol12/figure.png" width="120"></a> | {1,9,12}, {5,8,12} — on a common line |
| 13 | <a href="sol13/figure.png"><img src="sol13/figure.png" width="120"></a> | {0,7,12}, {0,11,13} — on a common line |
| 14 | <a href="sol14/figure.png"><img src="sol14/figure.png" width="120"></a> | {0,8,10}, {3,8,11} — on a common line |
| 15 | <a href="sol15/figure.png"><img src="sol15/figure.png" width="120"></a> | {0,7,12}, {2,12,13} — on a common line |

All other crossings are simple; no parallel pairs in any solution.
Each figure shows the full arrangement (top) and a detail of the dense
core (bottom); click for full resolution, or open the corresponding
`figure.svg` for unlimited zoom.

## Layout

```
sol1/ … sol15/          one folder per solution
  lines_rational.json   the 14 lines, exact rational coefficients
                        (a, b, c with a·x + b·y = c) + declared
                        degeneracy structure + provenance
  figure.png            rendering (full view + dense-core detail),
                        1800×3840 px
  figure.svg            same figure, vector (zoom without limits)
verify_direct_exact.py  verifier 1: direct classical definition
verify_events.py        verifier 2: event/wiring representation
SHA256SUMS.txt          checksums of all files
```

## Verify it yourself (two independent scripts, exact rational arithmetic)

Independent checking is exactly what these candidates still need — the
scripts below make it a one-liner:

```
for i in $(seq 1 15); do
  python3 verify_direct_exact.py sol$i/lines_rational.json
  python3 verify_events.py      sol$i/lines_rational.json
done
```

**Requirements:** Python 3 standard library only. Each run takes well
under a second and uses `fractions.Fraction` throughout — no floating
point anywhere. Expected output for every solution: `... — PASS` twice.

## Counting convention

A triangle is a triple of lines whose three pairwise intersection
points are distinct and whose open interior is not crossed by any other
line (an "empty triangle": a triangular face of the arrangement; such
triangles are automatically non-overlapping). A line through a vertex
only *touches* the triangle, which still counts; several triangles may
share a vertex at a triple point. This is the convention of the
published Kobon records (e.g. the k = 8 solution with 15 triangles).

## Provenance

Found on 2026-08-11/12 by stochastic search (simulated annealing with
adaptive reheating) over degenerate pseudoline arrangements, then
realized with straight lines under exact concurrency constraints and
verified in exact rational arithmetic. Per-solution search parameters
are recorded in each `lines_rational.json` (`provenance` field).
Developed by the author with the help of Claude Code (Anthropic).

## License

© 2026 Andrea Maiorana. This package (data, figures and scripts) is
released under the [Creative Commons Attribution 4.0 International
license](https://creativecommons.org/licenses/by/4.0/) (CC BY 4.0) —
see the [LICENSE](LICENSE) file. You may share and adapt it freely,
including commercially, as long as you give appropriate credit.
