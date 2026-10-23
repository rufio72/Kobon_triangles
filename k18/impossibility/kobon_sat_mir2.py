"""SAT con simmetria speculare SENZA RETTE FISSE (k/2 coppie, k pari).

La famiglia mancante: mirror con l'asse che non e' perpendicolare a
nessuna retta (a k=18: 9 coppie scambiate, nessuna retta fissa; i
triangoli vanno a coppie -> conteggio PARI, 94 = 47 coppie).

Involuzione DERIVATA EMPIRICAMENTE da disposizioni speculari
geometriche vere (90 casi k=6/8/10, etichette ordinate per angolo,
coppie specchiate su x=0): con sigma(i) = N-1-i,
    riga[sigma i] = inversa(sigma(riga[i]))
(lo specchio inverte l'orientazione: riga rietichettata E ribaltata).
In variabili A (0-based): A(r,i,k) => A(N-1-r, N-1-i, N-2-k).
Clausole ADDITIVE sul modello base di Savchuk (mirrored=False, rot=1):
il generatore validato non viene toccato.

Pipeline identica a kobon_sat_sym/mir: Kissat + DRAT, giudice
count_comb, verifica dell'involuzione sulla tabella estratta.
"""
import os, subprocess, time
from kobon_sat import (m_var_ids, KISSAT, DRATTRIM, WORK)
import koboncnf_ext as koboncnf


def a_ids(n):
    """Replica ESATTA della numerazione del generatore: per ogni (r,i)
    prima le coppie G,X su j, poi le A(r,i,k). Ritorna dict A."""
    A = {}
    unknowns = 0
    for r in range(n):
        for i in range(n):
            if r == i:
                continue
            for j in range(n):
                if r == j or i == j:
                    continue
                unknowns += 2                    # G, X
            for k in range(n - 1):
                unknowns += 1
                A[(r, i, k)] = unknowns
    return A


def gen_cnf_mir2(n, m_slots, card_max, path):
    assert n % 2 == 0
    missing = []
    for r in range(1, n + 1):
        missing += [r] * m_slots
    koboncnf.generate(n, path, missing_triangles=missing,
                      force_missing_use=False)
    A = a_ids(n)
    extra = []
    # involuzione speculare senza rette fisse
    for r in range(n):
        for i in range(n):
            if i == r:
                continue
            for k in range(n - 1):
                extra.append([-A[(r, i, k)],
                              A[(n - 1 - r, n - 1 - i, n - 2 - k)]])
    n_vars, M, slots = m_var_ids(n, m_slots)
    if m_slots > 0:
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


def tab_mir2_sym(tab):
    """Tabella (1-based) invariante per l'involuzione senza rette
    fisse: riga[sigma r] = inversa(sigma(riga[r])), sigma 0-based
    i -> N-1-i."""
    n = len(tab)
    z = [[x - 1 for x in row] for row in tab]
    for r in range(n):
        img = [n - 1 - x for x in z[r]][::-1]
        if z[n - 1 - r] != img:
            return False
    return True


def run_step_mir2(tag, n, m_slots, card_max, expect, timeout=None):
    cnf = os.path.join(WORK, f"{tag}.cnf")
    out = os.path.join(WORK, f"{tag}.out")
    proof = os.path.join(WORK, f"{tag}.drat")
    res = {"tag": tag, "n": n, "mirror2": True,
           "m_slots": m_slots, "card_max": card_max}
    t0 = time.time()
    gen_cnf_mir2(n, m_slots, card_max, cnf)
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
                               generate_table=True)
        tab = r2["table"]
        L = [[x - 1 for x in row] for row in tab]
        from kobon_comb import count_comb
        cnt, _, _ = count_comb(L)
        res["count"] = cnt
        res["table"] = tab
        res["mir2"] = tab_mir2_sym(tab)
        print(f"  [{tag}] estratto: count_comb = {cnt}, involuzione "
              f"{'OK' if res['mir2'] else 'VIOLATA!'}", flush=True)
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
