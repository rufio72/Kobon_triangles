#!/usr/bin/env python3
"""Conteggio dei triangoli nei sotto-arrangiamenti a 17 rette.

Per ciascuno dei 2357 arrangiamenti a 18 rette con 93 triangoli
(2337 in all_solutions.json, 20 in sol1..sol20/lines_rational.json) e per
ciascuna retta, conta in aritmetica razionale esatta i triangoli
dell'arrangiamento ottenuto togliendo quella retta. La logica di conteggio
è quella di verify_direct_exact.py, ristretta a un sottoinsieme di indici.

Uso:  python3 esperimenti/sotto17.py [--pilota] [--da N --a M]
                                     [--processi P] [--uscita FILE]
"""
import argparse
import itertools
import json
import os
import sys
from collections import Counter
from fractions import Fraction
from multiprocessing import Pool
from pathlib import Path

RADICE = Path(__file__).resolve().parent.parent
K = 18
BASE = 93
MINIMO, MASSIMO, SOGLIA_BASSA = 61, 85, 77


def _rette(lines_frac):
    """Converte le terne di stringhe "p/q" in terne di Fraction."""
    return [tuple(Fraction(x) for x in ln) for ln in lines_frac]


def carica_tutti():
    """Restituisce i 2357 arrangiamenti: prima all_<id>, poi sol1..sol20."""
    elenco = []
    with open(RADICE / "all_solutions.json") as f:
        dati = json.load(f)
    for s in dati["solutions"]:
        elenco.append({"nome": f"all_{s['id']}",
                       "rette": _rette(s["lines_frac"]),
                       "triple": s.get("triple_points", []),
                       "parallele": s.get("parallel_pairs", [])})
    for n in range(1, 21):
        with open(RADICE / f"sol{n}" / "lines_rational.json") as f:
            d = json.load(f)
        elenco.append({"nome": f"sol{n}",
                       "rette": _rette(d["lines_frac"]),
                       "triple": d.get("declared_triple_points", []),
                       "parallele": d.get("declared_parallel_pairs", [])})
    chiavi = {tuple(e["rette"]) for e in elenco}
    assert len(chiavi) == len(elenco), "ci sono arrangiamenti con rette identiche"
    return elenco


def _intersezioni(rette, indici):
    """Punti di intersezione per coppie i<j di indici; insieme delle parallele."""
    punti, par = {}, set()
    for i, j in itertools.combinations(indici, 2):
        a1, b1, c1 = rette[i]
        a2, b2, c2 = rette[j]
        det = a1 * b2 - a2 * b1
        if det == 0:
            par.add((i, j))
            continue
        punti[(i, j)] = ((c1 * b2 - c2 * b1) / det, (a1 * c2 - a2 * c1) / det)
    return punti, par


def _area_nulla(v):
    (x1, y1), (x2, y2), (x3, y3) = v
    return (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1) == 0


def conta_triangoli(rette, escluso=None):
    """Triangoli dell'arrangiamento senza la retta `escluso` (None: tutte)."""
    indici = [i for i in range(len(rette)) if i != escluso]
    punti, _ = _intersezioni(rette, indici)
    conteggio = 0
    for i, j, l in itertools.combinations(indici, 3):
        if (i, j) not in punti or (i, l) not in punti or (j, l) not in punti:
            continue
        v = [punti[(i, j)], punti[(i, l)], punti[(j, l)]]
        if _area_nulla(v):
            continue                    # terna concorrente: area nulla
        vuoto = True
        for m in indici:
            if m in (i, j, l):
                continue
            a, b, c = rette[m]
            s = [a * x + b * y - c for x, y in v]
            # separazione stretta di due vertici: la retta entra nell'interno
            if (min(s[0], s[1]) < 0 < max(s[0], s[1]) or
                    min(s[0], s[2]) < 0 < max(s[0], s[2]) or
                    min(s[1], s[2]) < 0 < max(s[1], s[2])):
                vuoto = False
                break
        conteggio += vuoto
    return conteggio


def lavora(elemento):
    """Calcola i 18 sotto-17 di un arrangiamento con i controlli di coerenza."""
    rette = elemento["rette"]
    ris = {"nome": elemento["nome"], "sotto17": [],
           "triple": elemento["triple"], "parallele": elemento["parallele"]}
    try:
        punti, par = _intersezioni(rette, range(len(rette)))
        dichiarate = {tuple(sorted(p)) for p in elemento["parallele"]}
        assert par == dichiarate, \
            f"coppie parallele {sorted(par)} diverse dalle dichiarate"
        for t in elemento["triple"]:
            i, j, l = sorted(t)
            v = [punti.get((i, j)), punti.get((i, l)), punti.get((j, l))]
            assert None not in v and _area_nulla(v), \
                f"la terna dichiarata {t} non è concorrente"
        tot18 = conta_triangoli(rette)
        assert tot18 == BASE, f"conteggio a 18 rette {tot18} invece di {BASE}"
        sotto = [conta_triangoli(rette, r) for r in range(len(rette))]
        ris["sotto17"] = sotto
        for r, n in enumerate(sotto):
            assert MINIMO <= n <= MASSIMO, \
                f"retta {r} tolta: {n} triangoli, fuori da [{MINIMO}, {MASSIMO}]"
    except AssertionError as e:
        ris["errore"] = str(e)
    return ris


def _distribuzione(valori):
    c = Counter(valori)
    return " ".join(f"{v}:{c[v]}" for v in sorted(c))


def riepilogo(risultati):
    """Stampa il riepilogo; restituisce il numero di errori."""
    validi = [r for r in risultati if "errore" not in r]
    tutti = [n for r in validi for n in r["sotto17"]]
    print(f"Arrangiamenti elaborati: {len(risultati)}; "
          f"conteggi sotto-17: {len(tutti)} ({K} per arrangiamento)")
    print(f"Distribuzione complessiva: {_distribuzione(tutti)}")
    print(f"Conteggi sotto {SOGLIA_BASSA}: "
          f"{sum(1 for n in tutti if n < SOGLIA_BASSA)}")
    print("Distribuzione dei massimi per arrangiamento: "
          f"{_distribuzione(max(r['sotto17']) for r in validi)}")
    perfetti = [r for r in validi if MASSIMO in r["sotto17"]]
    print(f"\n17 perfetti (sotto-17 = {MASSIMO}): {len(perfetti)}")
    for r in perfetti:
        tolte = [i for i, n in enumerate(r["sotto17"]) if n == MASSIMO]
        print(f"- {r['nome']}: rette tolte {tolte}")
        for t in tolte:
            tr = [x for x in r["triple"] if t in x]
            pa = [x for x in r["parallele"] if t in x]
            print(f"    retta {t}: in triple_points: {'sì' if tr else 'no'} {tr}; "
                  f"in parallel_pairs: {'sì' if pa else 'no'} {pa}")
    errori = [r for r in risultati if "errore" in r]
    print(f"\nErrori: {len(errori)}")
    for r in errori:
        print(f"- {r['nome']}: {r['errore']}")
    return len(errori)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--da", type=int, default=None, help="indice iniziale (0-based)")
    ap.add_argument("--a", type=int, default=None, help="indice finale escluso")
    ap.add_argument("--pilota", action="store_true",
                    help="solo sol1..sol20 (ultimi 20 dell'elenco)")
    ap.add_argument("--processi", type=int, default=os.cpu_count())
    ap.add_argument("--uscita", default=None, help="file JSON dei risultati")
    arg = ap.parse_args()

    elenco = carica_tutti()
    if arg.pilota:
        da, a = len(elenco) - 20, len(elenco)
        uscita = arg.uscita or RADICE / "esperimenti" / "risultati_pilota.json"
    else:
        da = arg.da if arg.da is not None else 0
        a = arg.a if arg.a is not None else len(elenco)
        uscita = arg.uscita or RADICE / "esperimenti" / "risultati_sotto17.json"
    scelti = elenco[da:a]

    with Pool(arg.processi) as pool:
        risultati = list(pool.imap(lavora, scelti, chunksize=4))

    dati = {"k": K, "conteggio_base": BASE, "intervallo": [da, a],
            "arrangiamenti": risultati}
    with open(uscita, "w") as f:
        json.dump(dati, f, indent=1, ensure_ascii=False)
    print(f"Risultati scritti in {uscita}\n")
    return 1 if riepilogo(risultati) else 0


if __name__ == "__main__":
    sys.exit(main())
