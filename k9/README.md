# k9 — census of optimal Kobon arrangements, n = 9

- Lines: **9**
- Maximum number of triangles (OEIS A006066): **21**
- Number of isomorphism types: **1**
- Enumeration at 21 triangles: SAT model allowing at most 0 finite segments not on a triangle (9·7 − 3·21)
- Upper bound check at 22 triangles: impossible by segment counting (9·7 − 3·22 < 0)

## Files

`tipo1.txt` … `tipo1.txt`: one canonical table per type.

A table has 9 rows; row r lists, in order along pseudoline r, the 8 other pseudolines it crosses (lines numbered 1..9 by the circular order of directions, as in a wiring diagram sorted by slope). A triple {a,b,c} bounds an empty triangle iff on each of the three rows the other two appear in adjacent positions.

## Method

- CNF model of P. Savchuk (arXiv:2507.07951, https://github.com/zegalur/kobon-cnf), reimplemented from scratch in C, generalized with free "missing segment" variables and a Sinz cardinality constraint (at most 0 finite segments not on a triangle, i.e. at least 21 triangles).
- SAT solver: kissat 4.0.4, called repeatedly; after every solution the whole isomorphism orbit (line relabeling, orientation reversal, reflection) is blocked, until UNSAT.
- Every solution is validated by greedy reconstruction of a wiring diagram (it must be a genuine pseudoline arrangement) and its triangle count is checked. Solutions rejected by the validation: 0.
- Canonical form: lexicographic minimum of the orbit.

## Scope

This census covers **simple** arrangements only (no triple points, no parallel lines) at the **pseudoline** level. Realization with straight lines in rational coordinates (as in k14–k18) and degenerate arrangements (triple points, parallels) are future extensions and are not covered here.

Produced by `census/census-kobon` in this repository.

## Degenerate arrangements (census extension, 2026-10-23)

Allowing multiple points and parallel lines (counting convention of the
published records: a line through a vertex only touches the triangle),
the census at 21 triangles finds 0 degenerate types.
Total types, simple + degenerate: 1 + 0 = 1.
Ceiling at 22 with degeneracies: **UNSAT** (kissat, 107.1 s).
Method: extended CNF model (see census/), kissat 4.0.4, validation by
generalized wiring reconstruction.
Parallelism is transitive (classes of directions, as for straight lines);
types are pseudoline arrangements up to relabeling, reversal and
reflection. Solutions rejected by the wiring validation: 0.
