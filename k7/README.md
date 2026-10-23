# k7 — census of optimal Kobon arrangements, n = 7

- Lines: **7**
- Maximum number of triangles (OEIS A006066): **11**
- Number of isomorphism types: **1**
- Enumeration at 11 triangles: SAT model allowing at most 2 finite segments not on a triangle (7·5 − 3·11)
- Upper bound check at 12 triangles: impossible by segment counting (7·5 − 3·12 < 0)

## Files

`tipo1.txt` … `tipo1.txt`: one canonical table per type.

A table has 7 rows; row r lists, in order along pseudoline r, the 6 other pseudolines it crosses (lines numbered 1..7 by the circular order of directions, as in a wiring diagram sorted by slope). A triple {a,b,c} bounds an empty triangle iff on each of the three rows the other two appear in adjacent positions.

## Method

- CNF model of P. Savchuk (arXiv:2507.07951, https://github.com/zegalur/kobon-cnf), reimplemented from scratch in C, generalized with free "missing segment" variables and a Sinz cardinality constraint (at most 2 finite segments not on a triangle, i.e. at least 11 triangles).
- SAT solver: kissat 4.0.4, called repeatedly; after every solution the whole isomorphism orbit (line relabeling, orientation reversal, reflection) is blocked, until UNSAT.
- Every solution is validated by greedy reconstruction of a wiring diagram (it must be a genuine pseudoline arrangement) and its triangle count is checked. Solutions rejected by the validation: 0.
- Canonical form: lexicographic minimum of the orbit.

## Scope

This census covers **simple** arrangements only (no triple points, no parallel lines) at the **pseudoline** level. Realization with straight lines in rational coordinates (as in k14–k18) and degenerate arrangements (triple points, parallels) are future extensions and are not covered here.

Produced by `census/census-kobon` in this repository.
