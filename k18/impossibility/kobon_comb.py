import itertools, math, random

# ---------- contatore GEOMETRICO (riferimento, gia' validato ieri) ----------
def count_geometric(lines, eps=1e-9):
    k = len(lines)
    P = {}
    for i in range(k):
        for j in range(i + 1, k):
            a1, b1, c1 = lines[i]; a2, b2, c2 = lines[j]
            det = a1 * b2 - a2 * b1
            P[(i, j)] = None if abs(det) < 1e-12 else (
                (c1 * b2 - c2 * b1) / det, (a1 * c2 - a2 * c1) / det)
    cnt = 0
    for i, j, l in itertools.combinations(range(k), 3):
        v1, v2, v3 = P[(i, j)], P[(i, l)], P[(j, l)]
        if v1 is None or v2 is None or v3 is None:
            continue
        area = abs((v2[0]-v1[0])*(v3[1]-v1[1]) - (v3[0]-v1[0])*(v2[1]-v1[1]))
        if area < 1e-9:
            continue
        ok = True
        for m in range(k):
            if m in (i, j, l):
                continue
            a, b, c = lines[m]
            s = [a*x + b*y - c for x, y in (v1, v2, v3)]
            if max(s) > eps and min(s) < -eps:
                ok = False; break
        if ok:
            cnt += 1
    return cnt

# ---------- rappresentazione COMBINATORIA ----------
# Stato = per ogni retta i, l'ordine con cui le altre rette la attraversano.
# Criterio: (a,b,c) e' un triangolo-faccia sse su ciascuna delle tre rette
# i due incroci con le altre due sono ADIACENTI nell'ordine di attraversamento
# (nessun incrocio intermedio sui tre lati => nessuna retta taglia l'interno).

def random_lines(k, rng):
    while True:
        params = [(rng.uniform(0, math.pi), rng.uniform(-1, 1)) for _ in range(k)]
        ok = True
        for i in range(k):
            for j in range(i+1, k):
                dt = abs(params[i][0]-params[j][0]) % math.pi
                if min(dt, math.pi-dt) < 0.02:
                    ok = False
        if ok:
            return [(math.cos(t), math.sin(t), d) for t, d in params]

def crossing_orders(lines):
    k = len(lines); L = []
    for i in range(k):
        a1, b1, c1 = lines[i]
        pts = []
        for j in range(k):
            if j == i:
                continue
            a2, b2, c2 = lines[j]
            det = a1*b2 - a2*b1
            x = (c1*b2 - c2*b1) / det
            y = (a1*c2 - a2*c1) / det
            pts.append((-b1*x + a1*y, j))
        pts.sort()
        L.append([j for _, j in pts])
    return L

def count_comb(L):
    k = len(L)
    pos = [{j: idx for idx, j in enumerate(Li)} for Li in L]
    tris = []
    for a, b, c in itertools.combinations(range(k), 3):
        if (abs(pos[a][b]-pos[a][c]) == 1 and
            abs(pos[b][a]-pos[b][c]) == 1 and
            abs(pos[c][a]-pos[c][b]) == 1):
            tris.append((a, b, c))
    return len(tris), tris, pos

# ---------- VALIDAZIONE ----------
if __name__ == "__main__":
    rng = random.Random(1)
    print("validazione contatore combinatorio vs geometrico (100 test casuali):")
    fails = 0; tested = 0
    for k in range(3, 11):
        for t in range(13):
            lines = random_lines(k, rng)
            g = count_geometric(lines)
            c, _, _ = count_comb(crossing_orders(lines))
            tested += 1
            if g != c:
                fails += 1
                print(f"  MISMATCH k={k}: geom={g} comb={c}")
    print(f"  test: {tested}, mismatch: {fails}")
