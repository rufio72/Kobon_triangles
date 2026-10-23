# kobon — arrangements for the Kobon triangle problem

One repository for all the solution data found in this project, one folder
per number of lines. Each `k<n>/` folder is a self-contained package with
its own README, exact rational data, verifiers and checksums.

| folder | lines | triangles | content |
|---|---|---|---|
| `k14/` | 14 | 53 | optimal solutions (also published at [kobon_triangles_k14](https://github.com/rufio72/kobon_triangles_k14), the OEIS-referenced copy — that repository is frozen) |
| `k16/` | 16 | 72 | 307 pairwise non-isomorphic optimal solutions |
| `k17/` | 17 | 85 | 21 optimal arrangements in 8 isomorphism classes, extracted as sub-arrangements of the k=18 record solutions; tools in C |
| `k18/` | 18 | 93 | 2357+ pairwise non-isomorphic record arrangements, SAT impossibility results for the symmetric families, sub-arrangement analysis |
| `k3/` … `k10/` | 3–10 | — | exhaustive census of simple optimal types per n (C + SAT, UNSAT proofs at max+1): 1, 1, 1, 2, 1, 0, 1, 36 types. At n=8 no simple arrangement reaches 15 (simple maximum is 14, matching Bartholdi–Blanc–Loisel): the known 15 needs degeneracies |
| `census/` | — | — | the census tool (C + kissat); next: n=11–12, then degenerate arrangements |

Status: everything here is verified in-house, pending external review.
Licensing: each package carries its own LICENSE (CC-BY-4.0).

## Census summary

Optimal types per number of lines n at the maximum known number of triangles
T (OEIS A006066), at the pseudoline level. A simple arrangement of n lines
has n(n−2) finite segments and every triangle uses 3 of them, so the slack
n(n−2) − 3·T counts the segments that are not a triangle side (with multiple
points two triangles may share a segment, so the bound applies to simple
arrangements only). Simple types: `census/census-kobon` (task 01, `tipoN.txt`
/ `soluzioni.json`); degenerate types (multiple points and/or parallel
classes): `census/census-degeneri` (`degenereN.txt`). In every `k<n>/README.md`
the section "Degenerate arrangements" gives the T+1 ceiling check.

| n | segments n(n−2) | T max | slack n(n−2) − 3·T | simple types | degenerate types | total |
|---|---|---|---|---|---|---|
| 3 | 3 | 1 | 0 | 1 | 0 | 1 |
| 4 | 8 | 2 | 2 | 1 | 2 | 3 |
| 5 | 15 | 5 | 0 | 1 | 0 | 1 |
| 6 | 24 | 7 | 3 | 2 | 6 | 8 |
| 7 | 35 | 11 | 2 | 1 | 0 | 1 |
| 8 | 48 | 15 | 3 | 0 | 1 | 1 |
| 9 | 63 | 21 | 0 | 1 | 0 | 1 |
| 10 | 80 | 25 | 5 | 36 | ≥ 49 (partial) | ≥ 85 (partial) |

For n = 8 the single type is the published 15-triangle solution (same event
canonical form as the ud1/kobon-solutions certificate). For n = 3..8 the
degenerate counts are confirmed by an independent exhaustive enumeration of
degenerate wiring diagrams.

n = 10 is partial: the degenerate enumeration was stopped before completion
(49 types found; resumable from `census/tmp/stato-deg-n10.txt`) and the
ceiling check at 26 was still running when this table was written.
