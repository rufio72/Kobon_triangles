# k8 — census of optimal Kobon arrangements, n = 8

- Lines: **8**
- Maximum number of triangles (OEIS A006066): **15**
- Number of isomorphism types: **0**
- Enumeration at 15 triangles: SAT model allowing at most 3 finite segments not on a triangle (8·6 − 3·15)
- Upper bound check at 16 triangles: **UNSAT** (SAT model with at most 0 segments not on a triangle, kissat)

## Files

No table files: no simple pseudoline arrangement of 8 lines has 15 triangles (the SAT model is already UNSAT at 15). The known optimum 15 is therefore reached only by non-simple arrangements (triple points or parallel lines), outside the scope of this census.

A table has 8 rows; row r lists, in order along pseudoline r, the 7 other pseudolines it crosses (lines numbered 1..8 by the circular order of directions, as in a wiring diagram sorted by slope). A triple {a,b,c} bounds an empty triangle iff on each of the three rows the other two appear in adjacent positions.

## Method

- CNF model of P. Savchuk (arXiv:2507.07951, https://github.com/zegalur/kobon-cnf), reimplemented from scratch in C, generalized with free "missing segment" variables and a Sinz cardinality constraint (at most 3 finite segments not on a triangle, i.e. at least 15 triangles).
- SAT solver: kissat 4.0.4, called repeatedly; after every solution the whole isomorphism orbit (line relabeling, orientation reversal, reflection) is blocked, until UNSAT.
- Every solution is validated by greedy reconstruction of a wiring diagram (it must be a genuine pseudoline arrangement) and its triangle count is checked. Solutions rejected by the validation: 0.
- Canonical form: lexicographic minimum of the orbit.

## Scope

This census covers **simple** arrangements only (no triple points, no parallel lines) at the **pseudoline** level. Realization with straight lines in rational coordinates (as in k14–k18) and degenerate arrangements (triple points, parallels) are future extensions and are not covered here.

Produced by `census/census-kobon` in this repository.

## Degenerate arrangements (census extension, 2026-10-23)

Allowing multiple points and parallel lines (counting convention of the
published records: a line through a vertex only touches the triangle),
the census at 15 triangles finds 1 degenerate type (`degenere1.txt`).
Total types, simple + degenerate: 0 + 1 = 1.
Ceiling at 16 with degeneracies: **UNSAT** (kissat, 14.6 s).
Method: extended CNF model (see census/), kissat 4.0.4, validation by
generalized wiring reconstruction, cross-check against the published
rational solution (ud1/kobon-solutions certificate,
https://raw.githubusercontent.com/ud1/kobon-solutions/HEAD/gallery/certificates/8/8-1yvz24guzamh9.json).
Parallelism is transitive (classes of directions, as for straight lines);
types are pseudoline arrangements up to relabeling, reversal and
reflection. Solutions rejected by the wiring validation: 0.
`degenere1.txt` has the same event canonical form
(canonical_sha256 742a96a31159cbdf…) as the published solution
(`census/dati/k8-pubblicata.json`).
In a `degenere*.txt` table row r lists the events along line r; lines
through a common point are in parentheses, lines parallel to r are absent.
