"""SAT con simmetria rotazionale C3 imposta (fase 2, k=18).

Riusa TUTTO da kobon_sat.py (numerazione variabili, slot mancanti +
cardinalita' di Sinz, pipeline Kissat/DRAT/estrazione) e attiva il
supporto NATIVO `rotational_symmetry` del generatore di Savchuk:
clausole A(r,i,k) => A(sr,si,sk) con sr = (r + j*2N/ROT) mod N e
inversione di riga quando l'angolo scavalca pi (la contabilita' delle
riflessioni e' dell'autore, non nostra). Con ROT=3 e' la nostra C3.

Budget dei mancanti (n=18): segmenti finiti n(n-2)=288;
  T>=94 <=> mancanti <= 288-3*94 = 6
  T>=93 <=> mancanti <= 9
NB: modello solo per arrangement SEMPLICI (vedi scala_sat.py): il
verdetto UNSAT@94 direbbe "nessun 94 C3-simmetrico SEMPLICE"; i casi
con punti tripli/parallele richiedono sotto-modelli dedicati (il punto
triplo NEL CENTRO e' comunque escluso dall'aritmetica mod 3: tetto 93).

Ogni SAT: tabella -> count_comb (giudice) + verifica C3 combinatoria.
Ogni UNSAT: certificato DRAT verificato con drat-trim.
"""
import os, subprocess, time
from kobon_sat import (m_var_ids, KISSAT, DRATTRIM, WORK)
import koboncnf_ext as koboncnf


def gen_cnf_sym(n, m_slots, card_max, path, rot=3):
    """Come kobon_sat.gen_cnf ma con rotational_symmetry=rot.
    Le clausole ROT non aggiungono variabili: la numerazione (e quindi
    m_var_ids) resta identica al modello base."""
    missing = []
    for r in range(1, n + 1):
        missing += [r] * m_slots
    koboncnf.generate(n, path, missing_triangles=missing,
                      rotational_symmetry=rot, force_missing_use=False)
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


def tab_c3_sym(tab, rot=3):
    """Verifica che la tabella estratta rispetti la simmetria del
    modello: A(r,i,k) => A(sr,si,sk) con la convenzione di Savchuk
    (P=2N/ROT, riga invertita se l'immagine scavalca pi)."""
    n = len(tab)
    P = (2 * n) // rot
    for r in range(n):
        for j in range(1, rot):
            pr = r + j * P
            sr = pr % n
            rev = not ((pr < n) or (pr >= 2 * n))
            img = [((x - 1 + j * P) % n) + 1 for x in tab[r]]
            if rev:
                img = img[::-1]
            if tab[sr] != img:
                return False
    return True


def run_step_sym(tag, n, m_slots, card_max, expect, timeout=None, rot=3):
    """Un gradino simmetrico: genera, risolve, verifica (giudice +
    simmetria per SAT, DRAT per UNSAT). expect in {SAT, UNSAT, ?}."""
    cnf = os.path.join(WORK, f"{tag}.cnf")
    out = os.path.join(WORK, f"{tag}.out")
    proof = os.path.join(WORK, f"{tag}.drat")
    res = {"tag": tag, "n": n, "rot": rot,
           "m_slots": m_slots, "card_max": card_max}
    t0 = time.time()
    gen_cnf_sym(n, m_slots, card_max, cnf, rot=rot)
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
                               rotational_symmetry=rot,
                               generate_table=True)
        tab = r2["table"]
        L = [[x - 1 for x in row] for row in tab]
        from kobon_comb import count_comb
        cnt, _, _ = count_comb(L)
        res["count"] = cnt
        res["table"] = tab
        res["c3"] = tab_c3_sym(tab, rot)
        print(f"  [{tag}] estratto: count_comb = {cnt}, "
              f"simmetria C3 {'OK' if res['c3'] else 'VIOLATA!'}",
              flush=True)
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
