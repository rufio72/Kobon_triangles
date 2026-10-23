# c3_symmetric/ — 69 new C3-symmetric 93-triangle arrangements

Appendix (2026-08-15). While closing the symmetric families (see
[`../impossibility/`](../impossibility/)), the C3-symmetric **orbital
annealing kernel** produced 93-triangle arrangements of its own. After
exact canonical dedup:

| | |
|---|---|
| Distinct C3-symmetric classes at 93 | **69** (+1 found by SAT, at the repo root) |
| Isomorphic to any of the 2 337 in `all_solutions.json` | **none** |
| Straightened to real lines, exact rational certification | 48 |
| Pseudoline-only so far (no real-line realization found) | 21 |
| With triple points / parallel pairs (always in σ-orbits of 3) | 3 / 10 |

Every arrangement here has combinatorial 120°-rotation symmetry:
lines fall into 6 orbits of 3 under `σ(i) = i+6 (mod 18)`, and the 93
triangles into 31 orbits of 3 (none has the central invariant
triangle — by the orbit arithmetic a C3-symmetric count is `≡ 0 or 1
(mod 3)`, and `93 = 3·31` needs no central triangle).

Free random search never lands on exact symmetry (it is a
measure-zero subspace), which is why none of these appeared among the
2 337: symmetry had to be **imposed** to find them.

Format: `arrangements.json` — one entry per class with the event
table (crossing orders, degenerate events as groups), degeneracy
counts, provenance (campaign + seed), and, where straightening
succeeded, exact rational `lines_frac` plus float `lines_float`.
`straightened: false` means no real-line realization was found within
our search budget — **not** a proof that none exists.

Verify a straightened entry with the package's own tools, e.g.:

```bash
python3 - << 'EOF'
import json, sys
sys.path.insert(0, "impossibility")
from fractions import Fraction
from kobon_eventi import count_events
d = json.load(open("c3_symmetric/arrangements.json"))
s = next(x for x in d["solutions"] if x["straightened"])
ev = [[frozenset(e) for e in row] for row in s["events"]]
print(count_events(ev)[0], "triangles (expected 93)")
EOF
```
