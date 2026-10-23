"""SAT con CLASSI PARALLELE imposte (fork koboncnf_par).

Budget mancanti a k=18 (S = 288 - somma m_j(m_j-1), T >= 94 <=>
mancanti <= S - 282):
  1 coppia   -> S=286, m<=4     2 coppie -> S=284, m<=2
  3 coppie   -> S=282, m=0      classe di 3 -> S=282, m=0
WLOG le rette parallele sono le prime (il modello non ha etichette
privilegiate). UNSAT = impossibile (il fork puo' solo sovra-generare);
un SAT va riverificato (conteggio indipendente + raddrizzamento).
"""
import os, subprocess, time
from kobon_sat import cleanup_files, KISSAT, DRATTRIM, WORK
import koboncnf_par as kpar


def gen_cnf_par(n, parallel_classes, m_slots, card_max, path):
    missing = []
    for r in range(1, n + 1):
        missing += [r] * m_slots
    kpar.generate(n, path, parallel_classes=parallel_classes,
                  missing_triangles=missing, force_missing_use=False)
    if m_slots == 0:
        return
    MT = [(r, m_slots) for r in range(n)]
    G, X, A, M, n_vars = kpar.var_ids(n, parallel_classes, MT)
    par, AV, L = kpar.domains(n, parallel_classes)
    extra = []
    u = {}
    slots = [(r, k) for r in range(n) for k in range(m_slots)]
    for (r, k) in slots:
        n_vars += 1
        u[(r, k)] = n_vars
        for i in AV[r]:
            for j in AV[r]:
                if i == j:
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


def count_par_table(tab, n, parallel_classes):
    """Conteggio indipendente della tabella (1-based) con parallele:
    eventi singoli, coppie parallele semplicemente assenti."""
    from kobon_eventi import count_events
    events = [[frozenset([x - 1]) for x in row] for row in tab]
    # verifica struttura parallela: i partner non compaiono
    par, AV, L = kpar.domains(n, parallel_classes)
    for r in range(n):
        pres = set(x - 1 for x in tab[r])
        if pres != set(AV[r]):
            raise RuntimeError(f"riga {r}: dominio {sorted(pres)} != "
                               f"atteso {AV[r]}")
    return count_events(events)


def run_step_par(tag, n, parallel_classes, m_slots, card_max, expect,
                 timeout=None):
    cnf = os.path.join(WORK, f"{tag}.cnf")
    out = os.path.join(WORK, f"{tag}.out")
    proof = os.path.join(WORK, f"{tag}.drat")
    res = {"tag": tag, "n": n, "parallel_classes": parallel_classes,
           "m_slots": m_slots, "card_max": card_max}
    t0 = time.time()
    gen_cnf_par(n, parallel_classes, m_slots, card_max, cnf)
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
        cleanup_files(cnf, proof)
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
        r2 = kpar.generate(n, out, parallel_classes=parallel_classes,
                           missing_triangles=missing,
                           generate_table=True)
        tab = r2["table"]
        cnt, _ = count_par_table(tab, n, parallel_classes)
        res["count"] = cnt
        res["table"] = tab
        print(f"  [{tag}] estratto: conteggio giudice = {cnt}, "
              f"struttura parallela OK", flush=True)
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
    cleanup_files(cnf, proof)
    return res
