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
