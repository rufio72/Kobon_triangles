"""Contatore di RIFERIMENTO per arrangement degeneri (giudice, in Python).

Rappresentazione a EVENTI: per ogni retta, la sequenza degli eventi di
attraversamento; un evento e' un insieme di rette (1 = incrocio semplice,
>=2 = punto multiplo). Le coppie parallele semplicemente non compaiono
l'una nella riga dell'altra.

DEFINIZIONE di triangolo (da validare sulle soluzioni pubblicate):
tripla (x,y,z) tale che
  - ogni coppia si incrocia (esiste l'evento)
  - i tre punti di incrocio non coincidono (la tripla non e' interamente
    dentro un unico punto multiplo: area zero)
  - su ciascuna delle tre rette, i due eventi rilevanti sono ADIACENTI
    nella sequenza (nessun evento intermedio)
Ai punti multipli piu' triangoli possono toccarsi per il VERTICE: contano
tutti (adiacenza soddisfatta da lati opposti).

Formato tabella alla Savchuk (1-based, gruppi come sottoliste):
    [[8,6,7,4,5,2,3],[6,[4,8],7,...],...]
"""
import itertools


def parse_table(tab):
    """Da tabella Savchuk (1-based, gruppi) a eventi 0-based.
    Ritorna events[i] = lista di frozenset (le rette dell'evento)."""
    events = []
    for row in tab:
        er = []
        for x in row:
            if isinstance(x, list):
                er.append(frozenset(v - 1 for v in x))
            else:
                er.append(frozenset([x - 1]))
        events.append(er)
    return events


def count_events(events):
    """Conta i triangoli secondo la definizione sopra.
    Ritorna (conteggio, lista triple)."""
    k = len(events)
    # epos[i][j] = indice dell'evento di i che contiene j (None se paralleli)
    epos = [dict() for _ in range(k)]
    for i in range(k):
        for idx, ev in enumerate(events[i]):
            for j in ev:
                epos[i][j] = idx
    tris = []
    for x, y, z in itertools.combinations(range(k), 3):
        if (y not in epos[x] or z not in epos[x] or z not in epos[y]):
            continue                      # qualche coppia parallela
        exy, exz = epos[x][y], epos[x][z]
        eyx, eyz = epos[y][x], epos[y][z]
        ezx, ezy = epos[z][x], epos[z][y]
        if exy == exz:                    # tutti e tre per lo stesso punto
            continue
        if abs(exy - exz) == 1 and abs(eyx - eyz) == 1 and \
           abs(ezx - ezy) == 1:
            tris.append((x, y, z))
    return len(tris), tris


def from_simple_orders(L):
    """Da ordini semplici 0-based (formato del progetto) a eventi."""
    return [[frozenset([j]) for j in row] for row in L]


if __name__ == "__main__":
    # === ANCORA 1: la soluzione pubblicata k=8, 15 triangoli ============
    kobon_8 = [
        [8, 6, 7, 4, 5, 2, 3],
        [6, [4, 8], 7, 5, 1, 3],
        [4, 6, 5, 7, 8, 1, 2],
        [3, 6, [8, 2], 7, 1, 5],
        [6, 3, [7, 8], 2, 1, 4],
        [5, 3, 4, 2, 8, 1, 7],
        [3, [8, 5], 2, 4, 1, 6],
        [3, [5, 7], [2, 4], 6, 1],
    ]
    n, tris = count_events(parse_table(kobon_8))
    print(f"k=8 pubblicata (2 punti tripli): {n} triangoli "
          f"(attesi 15) {'OK' if n == 15 else 'FALLITO'}")

    # === controllo interno: sui semplici deve coincidere con count_comb =
    import random
    from kobon_comb import random_lines, crossing_orders, count_comb
    ok = tot = 0
    for k in range(4, 12):
        for s in range(13):
            L = crossing_orders(random_lines(k, random.Random(1000 + s + 100 * k)))
            n1, _, _ = count_comb(L)
            n2, _ = count_events(from_simple_orders(L))
            tot += 1
            ok += (n1 == n2)
    print(f"coerenza sui semplici: {ok}/{tot} "
          f"{'OK' if ok == tot else 'FALLITO'}")
