"""SAT con simmetria SPECULARE imposta (famiglia mirror, k=18).

Usa il supporto nativo `mirrored=True` del generatore di Savchuk:
convenzione con la retta 1 autosimmetrica (riflessa su se stessa,
riga invertita con rietichettatura i -> N-i) e, per N pari, anche la
retta 1+N/2; le altre a coppie r <-> N-r+2 (1-based). E' la famiglia
speculare "asse perpendicolare a 2 rette"; la famiglia a 9 coppie
senza rette fisse NON e' coperta da questa convenzione.

Budget mancanti identico al caso C3 (n=18): T>=94 <=> m<=6;
T>=93 <=> m<=9. Modello solo per arrangement semplici.
Pipeline identica a kobon_sat_sym: Kissat + DRAT, giudice count_comb.
"""
import os, subprocess, time
from kobon_sat import (m_var_ids, KISSAT, DRATTRIM, WORK)
import koboncnf_ext as koboncnf


def gen_cnf_mir(n, m_slots, card_max, path):
    missing = []
    for r in range(1, n + 1):
        missing += [r] * m_slots
    koboncnf.generate(n, path, missing_triangles=missing,
                      mirrored=True, force_missing_use=False)
    if m_slots == 0:
        return
    n_vars, M, slots = m_var_ids(n, m_slots)
    extra = []
    u = {}
    for (r, k) in slots:
        n_vars += 1
        u[(r, k)] = n_vars
        for i in range(n):
            for j in range(n):
                if r == i or r == j or i == j:
                    continue
                extra.append([-M[(k, r, i, j)], u[(r, k)]])
    U = [u[s] for s in slots]
    K = card_max
    s_reg = {}
    for i in range(1, len(U) + 1):
        for j in range(1, K + 1):
            n_vars += 1
            s_reg[(i, j)] = n_vars
    extra.append([-U[0], s_reg[(1, 1)]])
    for j in range(2, K + 1):
        extra.append([-s_reg[(1, j)]])
    for i in range(2, len(U) + 1):
        extra.append([-U[i - 1], s_reg[(i, 1)]])
        for j in range(1, K + 1):
            extra.append([-s_reg[(i - 1, j)], s_reg[(i, j)]])
            if j > 1:
                extra.append([-U[i - 1], -s_reg[(i - 1, j - 1)],
                              s_reg[(i, j)]])
        extra.append([-U[i - 1], -s_reg[(i - 1, K)]])
    with open(path) as f:
        content = f.read()
    lines = content.split("\n")
    for idx, l in enumerate(lines):
        if l.startswith("p cnf "):
            _, _, v0, c0 = l.split()
            lines[idx] = f"p cnf {n_vars} {int(c0) + len(extra)}"
            break
    with open(path, "w") as f:
        f.write("\n".join(lines))
        for cl in extra:
            f.write("\n" + " ".join(str(x) for x in cl) + " 0")


def tab_mirror_sym(tab):
    """Verifica la simmetria speculare della tabella estratta secondo
    la convenzione del modello (0-based: riga 0 -> se stessa invertita
    con i->N-i; riga r -> riga (N-r)%N con i->(N-i)%N)."""
    n = len(tab)
    z = [[x - 1 for x in row] for row in tab]

    def m(i):                       # immagine 0-based di una retta
        return (n - i) % n

    for r in range(n):
        img = [m(x) for x in z[r]]
        if r == 0:
            if z[0] != img[::-1]:
                return False
        else:
            if z[m(r)] != img:
                return False
    return True


def run_step_mir(tag, n, m_slots, card_max, expect, timeout=None):
    cnf = os.path.join(WORK, f"{tag}.cnf")
    out = os.path.join(WORK, f"{tag}.out")
    proof = os.path.join(WORK, f"{tag}.drat")
    res = {"tag": tag, "n": n, "mirror": True,
           "m_slots": m_slots, "card_max": card_max}
    t0 = time.time()
    gen_cnf_mir(n, m_slots, card_max, cnf)
    res["t_gen"] = round(time.time() - t0, 1)
    t0 = time.time()
    try:
        with open(out, "w") as f:
            subprocess.run([KISSAT, cnf, proof], stdout=f,
                           stderr=subprocess.STDOUT, timeout=timeout)
    except subprocess.TimeoutExpired:
        res["status"] = "TIMEOUT"
        res["t_solve"] = round(time.time() - t0, 1)
        print(f"  [{tag}] TIMEOUT dopo {res['t_solve']}s", flush=True)
        return res
    res["t_solve"] = round(time.time() - t0, 1)
    status = None
    for l in open(out):
        if l.startswith("s "):
            status = l.split()[1]
    res["status"] = status
    print(f"  [{tag}] gen {res['t_gen']}s | kissat {res['t_solve']}s "
          f"-> {status}", flush=True)
    if status == "SATISFIABLE":
        missing = []
        for r in range(1, n + 1):
            missing += [r] * m_slots
        r2 = koboncnf.generate(n, out, missing_triangles=missing,
                               mirrored=True, generate_table=True)
        tab = r2["table"]
        L = [[x - 1 for x in row] for row in tab]
        from kobon_comb import count_comb
        cnt, _, _ = count_comb(L)
        res["count"] = cnt
        res["table"] = tab
        res["mir"] = tab_mirror_sym(tab)
        print(f"  [{tag}] estratto: count_comb = {cnt}, specularita' "
              f"{'OK' if res['mir'] else 'VIOLATA!'}", flush=True)
    elif status == "UNSATISFIABLE":
        t0 = time.time()
        v = subprocess.run([DRATTRIM, cnf, proof], capture_output=True,
                           text=True)
        res["t_verify"] = round(time.time() - t0, 1)
        res["verified"] = "s VERIFIED" in v.stdout
        print(f"  [{tag}] drat-trim {res['t_verify']}s -> "
              f"{'VERIFICATO' if res['verified'] else 'NON VERIFICATO!'}",
              flush=True)
    if expect == "?":
        res["ok"] = status in ("SATISFIABLE", "UNSATISFIABLE")
    else:
        res["ok"] = (expect == "SAT" and status == "SATISFIABLE") or \
                    (expect == "UNSAT" and status == "UNSATISFIABLE" and
                     res.get("verified"))
    print(f"  [{tag}] atteso {expect}: {'OK' if res['ok'] else 'FALLITO'}",
          flush=True)
    return res
