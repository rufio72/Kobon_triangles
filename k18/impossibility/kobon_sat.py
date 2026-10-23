"""Chiusura di N(14) via SAT — infrastruttura.

Codifica: modello CNF di Savchuk (kobon-cnf, arXiv:2507.07951) usato COME
GENERATORE senza modifiche + estensione ADDITIVA per il vincolo di
cardinalita' globale sui triangoli mancanti:
- il modello base (variabili A/G/X, consistenza via X, ottimalita' via G)
  impone che ogni segmento finito sia lato di un triangolo, salvo i
  segmenti "mancanti" marcati dalle variabili M dei k slot per retta;
- noi diamo a OGNI retta m slot (missing_triangles = tutte le rette
  ripetute m volte) e aggiungiamo in coda al CNF: variabili u(r,k)
  "slot usato" con M(k,r,i,j) -> u(r,k), e un contatore sequenziale di
  Sinz sum(u) <= m. Cosi' UNA SOLA formula copre tutte le distribuzioni
  dei mancanti: T >= (n(n-2) - m)/3.
- Conteggio: n(n-2) segmenti finiti totali; ogni triangolo usa 3 segmenti,
  un segmento appartiene a al piu' un triangolo => T = (segmenti coperti)/3.
  n=14: T>=54 <=> mancanti <= 6. UNSAT => nessun arrangiamento di
  pseudorette con >=54 triangoli.
Solver: Kissat 4.0.4 con proof DRAT; verifica: drat-trim.
Ogni SAT: tabella estratta -> ordini 0-based -> count_comb (contatore
Python validato). Ogni UNSAT: certificato DRAT verificato. Tutto cronometrato.
"""
import os, subprocess, sys, time

SAT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "sat")
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
# fork minimo di koboncnf.py (Savchuk): la clausola "almeno un mancante
# per retta elencata" e' opzionale — coi nostri slot-su-tutte-le-rette
# + cardinalita' globale deve essere DISATTIVATA
import koboncnf_ext as koboncnf

KISSAT = os.path.join(SAT_DIR, "kissat", "build", "kissat")
DRATTRIM = os.path.join(SAT_DIR, "drat-trim", "drat-trim")
WORK = os.path.join(SAT_DIR, "work")
os.makedirs(WORK, exist_ok=True)


def m_var_ids(n, m_slots):
    """Replica ESATTA della numerazione delle variabili di koboncnf.py.
    Ritorna (n_vars_totali, dict M[(k,r,i,j)] -> id, lista slot (r,k))."""
    unknowns = 0
    for r in range(n):
        for i in range(n):
            if r == i:
                continue
            for j in range(n):
                if r == j or i == j:
                    continue
                unknowns += 2                       # G, X
            unknowns += n - 1                       # A
    M = {}
    slots = []
    for r in range(n):                              # MT: ogni retta, m slot
        if m_slots == 0:
            break
        for k in range(m_slots):
            slots.append((r, k))
            for i in range(n):
                for j in range(n):
                    if r == i or r == j or i == j:
                        continue
                    unknowns += 1
                    M[(k, r, i, j)] = unknowns
    return unknowns, M, slots


def gen_cnf(n, m_slots, card_max, path):
    """Genera il CNF; se m_slots>0 aggiunge u(r,k) e sum(u) <= card_max."""
    missing = []
    for r in range(1, n + 1):
        missing += [r] * m_slots
    koboncnf.generate(n, path, missing_triangles=missing,
                      force_missing_use=False)
    if m_slots == 0:
        return
    n_vars, M, slots = m_var_ids(n, m_slots)
    extra = []
    # u(r,k): M(k,r,i,j) -> u(r,k)
    u = {}
    for (r, k) in slots:
        n_vars += 1
        u[(r, k)] = n_vars
        for i in range(n):
            for j in range(n):
                if r == i or r == j or i == j:
                    continue
                extra.append([-M[(k, r, i, j)], u[(r, k)]])
    # contatore sequenziale di Sinz: sum(u) <= card_max
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
    # riscrive il file col nuovo header e le clausole extra in coda
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


def run_step(tag, n, m_slots, card_max, expect, timeout=None):
    """Esegue un gradino: genera, risolve (con proof), verifica.
    expect in {'SAT','UNSAT'}. Ritorna dict con esiti e tempi."""
    cnf = os.path.join(WORK, f"{tag}.cnf")
    out = os.path.join(WORK, f"{tag}.out")
    proof = os.path.join(WORK, f"{tag}.drat")
    res = {"tag": tag}
    t0 = time.time()
    gen_cnf(n, m_slots, card_max, cnf)
    res["t_gen"] = time.time() - t0
    t0 = time.time()
    with open(out, "w") as f:
        p = subprocess.run([KISSAT, cnf, proof], stdout=f,
                           stderr=subprocess.STDOUT, timeout=timeout)
    res["t_solve"] = time.time() - t0
    status = None
    for l in open(out):
        if l.startswith("s "):
            status = l.split()[1]
    res["status"] = status
    print(f"  [{tag}] gen {res['t_gen']:.1f}s | kissat {res['t_solve']:.1f}s "
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
        print(f"  [{tag}] modello estratto, count_comb = {cnt}", flush=True)
    elif status == "UNSATISFIABLE":
        t0 = time.time()
        v = subprocess.run([DRATTRIM, cnf, proof], capture_output=True,
                           text=True)
        res["t_verify"] = time.time() - t0
        res["verified"] = "s VERIFIED" in v.stdout
        print(f"  [{tag}] drat-trim {res['t_verify']:.1f}s -> "
              f"{'VERIFICATO' if res['verified'] else 'NON VERIFICATO!'}",
              flush=True)
    ok = (expect == "SAT" and status == "SATISFIABLE") or \
         (expect == "UNSAT" and status == "UNSATISFIABLE" and
          res.get("verified"))
    res["ok"] = ok
    print(f"  [{tag}] atteso {expect}: {'OK' if ok else 'FALLITO'}",
        flush=True)
    return res
