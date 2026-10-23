# k17 — 21 optimal arrangements of 17 lines (85 triangles), 8 isomorphism classes

This package contains **21 arrangements of 17 straight lines**, each
forming **85 non-overlapping triangles** — which is Tamura's upper
bound for 17 lines, so every one of them is **optimal**. Up to
isomorphism they fall into **8 distinct classes**.

| | |
|---|---|
| Lines (*k*) | 17 |
| Triangles per arrangement | 85 |
| Tamura's bound | ⌊17·15/3⌋ = 85, so these are **optimal** |
| Best previously published | 85 (a single arrangement, by Bader) |
| Arrangements in this package | 21 |
| Isomorphism classes | **8** |
| Structure | all simple: no triple points, no parallel pairs |
| Arithmetic | exact rationals, no floating point |
| Independent checks per arrangement | 2 |

> **Status: not yet independently confirmed.** Every arrangement passes
> the verification scripts included here (two independent counting
> methods, exact rational arithmetic, plus a canonical-form
> classification), but nothing in this package has been checked by
> anyone outside the project. Please regard the data as
> **verified in-house, pending external review** — and see
> [Verify it yourself](#verify-it-yourself): it takes two commands.

## How they were obtained

The source is the package `k18_final` (A. Maiorana, 2026): 2357
arrangements of 18 lines with 93 triangles each. For every one of them
and every one of its 18 lines, the line was removed and the triangles of
the remaining 17-line arrangement were counted in exact arithmetic.
Exactly **21** of these 2357 × 18 = 42,426 sub-arrangements reach 85:

* **5** come from 18-line arrangements *with* triple points, by
  removing a line that passes through all of their triple points (so
  the 17 remaining lines are in general position);
* **16** come from *simple* 18-line arrangements.

Line coefficients are copied verbatim from the source; for each
arrangement the JSON records the source arrangement, the removed line,
the source index of every kept line and the source's degeneracies.
The program `build/estrai.c` (internal, comments in Italian) performs
the extraction; it needs a local copy of `k18_final` (option `--k18`)
and is included to document the provenance.

## The 8 classes

Classes are numbered by first occurrence; the representative is the
first member. Each member is listed as `id (source, removed line)`.

| class | size | representative | members |
|---|---|---|---|
| 1 | 5 | k17_01 | k17_01 (all_43, 0), k17_13 (all_1032, 1), k17_16 (all_1558, 17), k17_17 (all_1562, 13), k17_19 (sol4, 17) |
| 2 | 1 | k17_02 | k17_02 (all_134, 4) |
| 3 | 2 | k17_03 | k17_03 (all_273, 12), k17_07 (all_514, 2) |
| 4 | 3 | k17_04 | k17_04 (all_336, 17), k17_12 (all_953, 3), k17_20 (sol9, 1) |
| 5 | 5 | k17_05 | k17_05 (all_354, 10), k17_09 (all_832, 8), k17_10 (all_839, 16), k17_14 (all_1052, 5), k17_15 (all_1152, 5) |
| 6 | 3 | k17_06 | k17_06 (all_479, 15), k17_11 (all_897, 5), k17_21 (sol11, 13) |
| 7 | 1 | k17_08 | k17_08 (all_608, 11) |
| 8 | 1 | k17_18 | k17_18 (all_1761, 12) |

Sources `all_N` are entries of `all_solutions.json` in `k18_final`,
`solN` its folders; removed lines are 0-based indices in the source.
All 21 occurrences are published, not only the 8 representatives: the
same optimal 17-line arrangement often sits inside several different
93-triangle arrangements of 18 lines.

## What "non-isomorphic" means here

Two arrangements are considered the same if one can be obtained from
the other by **relabeling the lines** and **reversing the traversal
direction of individual lines** — the same notion used in `k18_final`.

The classification uses a **complete canonical form**, not just an
invariant. For each line, the sequence of crossings along it is
computed exactly. Choosing one line as line 0, and a direction on it,
fixes the labels of all other lines in the order in which they cross
line 0; ties (several lines through one point of line 0, lines parallel
to it) are enumerated exhaustively. Each relabeled sequence is then
taken in whichever direction is lexicographically smaller, and the
canonical form is the minimum over all candidates. Two arrangements
therefore have the same canonical form **if and only if** they are
isomorphic in the sense above. Its SHA-256 is stored per arrangement
(`canonical_sha256`); `classify --self-test` also checks that the
form is unchanged under random relabeling, random reversal of lines and
mirror reflection.

**Comparison with Bader's arrangement: it belongs to class 5.** The
crossing table of Johannes Bader's published 85-triangle arrangement was
retrieved on 2026-10-23 from Savchuk's LineOrder gallery
(<https://zegalur.github.io/line-order/gallery/kobon.html>, Gallery #1)
and is stored verbatim in `bader17_table.json` (row *i* lists, in order
along line *i*, the lines it crosses). `compare_bader` turns it into
the same event structure used by `classify`, recounts 85 triangles by
adjacency and computes the same canonical form: it coincides with that
of **class 5** and with none of the other classes. So Bader's
arrangement is, up to isomorphism, a member of class 5 of this package;
classes 1–4 and 6–8 are different from it. This is a comparison with
that single published arrangement only; nothing is claimed about other
literature. The comparison is combinatorial: it concerns the (affine)
order in which the lines cross each other, up to relabeling and
reversal of individual lines, which also covers mirror reflections. The
realizability of the gallery's table by straight lines is not checked
here; it is Bader's, published as a line arrangement.

## Layout

```
k17_final/
├── k17_solutions.json        the 21 arrangements + the list of classes
├── verify_direct_exact.c     verifier 1 — classical definition
├── classify.c                verifier 2 — events, count, canonical form, classes
├── bader17_table.json        Bader's 17-line arrangement, crossing table (LineOrder gallery)
├── compare_bader.c           canonical form of Bader's table vs the 8 classes
├── geom.c, geom.h            exact line geometry: crossing events, adjacency count
├── canon.c, canon.h          complete canonical form and its SHA-256
├── json_min.c, json_min.h    minimal JSON parser
├── sha256.c, sha256.h        SHA-256 (FIPS 180-4)
├── Makefile                  make, make check, make clean
├── build/estrai.c            extraction from k18_final (provenance)
├── SHA256SUMS.txt
└── LICENSE
```

Line coefficients are given as exact rationals `(a, b, c)` meaning
`a·x + b·y = c`. The `lines_float` fields are a convenience for
plotting only — **all verification uses `lines_frac`**.

## Verify it yourself

A C compiler and GMP (Debian/Ubuntu: `apt install libgmp-dev`); no
other dependencies:

```bash
make

# classical definition, every arrangement (85 triangles, 15 per line)
./verify_direct_exact
./verify_direct_exact --id k17_05     # just one

# crossing sequences, adjacency count, canonical form and classes
./classify --self-test

# Bader's arrangement against the 8 classes
./compare_bader --self-test

# or all three at once
make check

# file integrity
sha256sum -c SHA256SUMS.txt
```

Each verifier ends with a line containing `PASS`.

* `verify_direct_exact` — the classical definition: a triple of
  lines counts iff its three pairwise intersection points are distinct
  and no other line meets the open interior. It also checks that every
  line is a side of exactly 15 triangles, as attaining Tamura's bound
  requires.
* `classify` — an independent route: builds each line's sequence of
  crossings, counts triangles from adjacency in those sequences,
  recomputes multiple points and parallel pairs, the canonical form and
  the classes, and checks all of them against the declared ones.
* `compare_bader` — validates Bader's crossing table
  (`bader17_table.json`), recounts its triangles by adjacency, computes
  its canonical form as `classify` does and looks it up among the
  declared classes; it ends with `BADER: class 5 (…) — MATCH`.

**Counting convention.** A line passing exactly through a vertex only
*touches* the triangle, which still counts; several triangles may meet
at a triple point. This is the convention of the published record
solutions (e.g. the 15-triangle solution for `k = 8`). The 21
arrangements here are simple, so the convention does not affect them.

## Citing

If you use this data, please cite it as:

> A. Maiorana, *21 optimal 85-triangle arrangements of 17 lines in 8
> isomorphism classes* (Kobon triangle problem), 2026. Package `k17_final`.

## License

© 2026 Andrea Maiorana. Data and scripts are released under the
[Creative Commons Attribution 4.0 International license][cc] (CC BY
4.0) — see [LICENSE](LICENSE). You may share and adapt freely,
including commercially, with appropriate credit.

Developed by the author with the help of Claude Code (Anthropic).

[cc]: https://creativecommons.org/licenses/by/4.0/
