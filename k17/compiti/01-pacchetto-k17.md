# Compito 01: pacchetto delle soluzioni ottimali a 17 rette (85 triangoli)

Data: 2026-10-23. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Costruire in `/home/rufio72/git/kobon_triangles_k17` un pacchetto pubblicabile
con i 21 arrangiamenti di 17 rette e 85 triangoli di Kobon (85 = tetto di
Tamura ⌊17·15/3⌋, quindi ottimali) ottenuti togliendo una retta da
arrangiamenti a 18 rette e 93 triangoli del pacchetto k18, raggruppati in
classi di isomorfismo con una forma canonica completa. Il dato principale
del pacchetto è il numero di classi distinte (misurato dall'analista: 8).

### Vincoli assoluti

- Il repository `/home/rufio72/git/kobon_triangles_k18` si legge soltanto:
  nessun file creato, modificato o cancellato lì (neanche `__pycache__`:
  eseguire Python con `python3 -I` o `PYTHONDONTWRITEBYTECODE=1` se si
  importa qualcosa da lì; meglio non importare, copiare il codice).
- Niente modifiche fuori da `/home/rufio72/git/kobon_triangles_k17`.
- Solo libreria standard di Python 3.12.3 (`fractions`, `itertools`,
  `json`, `hashlib`, `argparse`, `random`). Niente numpy, niente float nei
  calcoli (i float solo nel campo `lines_float`, per i grafici).
- Lingua: brief, script interno `build/estrai.py` e messaggi di commit in
  italiano; i file pubblicati (README, `verify_direct_exact.py`,
  `classify.py`, campo `note` del JSON) in inglese, come nel pacchetto k18.
- Nessun file di codice sopra le 800 righe (attesi: ~100-200 righe ciascuno).
- Niente sudo, niente installazioni, niente push.
- Non committare: il commit lo fa chi ha lanciato il compito. Solo `git init`.

## Cosa esiste già (fatti verificati)

Tutti i percorsi sotto sono relativi a `/home/rufio72/git/kobon_triangles_k18`.

| File | Contenuto (fonte) |
|---|---|
| `all_solutions.json` (6 MB) | dict: `k`=18, `triangles`=93, `note`, `count`=2337, `simple`=2070, `with_degeneracies`=267, `solutions` (lista). Voce: `id` (stringa "1".."2337"), `lines_frac` (18 terne di stringhe `"p/q"` = a, b, c con a·x+b·y=c), `triple_points`, `parallel_pairs`, `seed`, `arm`. Nessun `lines_float`. (misurato) |
| `solN/lines_rational.json` (N=1..20) | chiavi `k`, `count`(93), `lines_frac`, `lines_float`, `declared_triple_points`, `declared_parallel_pairs`, `structure`, `provenance`. (misurato su sol4) |
| `esperimenti/risultati_sotto17.json` | dict `k`, `conteggio_base`(93), `intervallo`, `arrangiamenti`: lista di 2357 voci `{nome, sotto17: [18 conteggi, indice = retta tolta], triple, parallele}`, nell'ordine all_1..all_2337 poi sol1..sol20. (misurato) |
| `esperimenti/sotto17.py` (194 righe) | `carica_tutti()` riga 34: carica i 2357 arrangiamenti come sopra; `_rette(lines_frac)` riga 29: `[tuple(Fraction(x) for x in ln) for ln in lines_frac]`. (da `grafo symbol`) |
| `verify_direct_exact.py` (82 righe, letto per intero) | Verificatore classico esatto su un file con `lines_frac`, `count`, `declared_parallel_pairs`. Da adattare. |
| `verify_events.py` (88 righe, letto per intero) | Sequenze di eventi per retta (parametro `s = -b1*x + a1*y`, eventi = insiemi di rette nello stesso punto), conteggio per adiacenza. Base della forma canonica. |
| `README.md` (237 righe) | Modello: tabella iniziale, riquadro «Status: not yet independently confirmed… **verified in-house, pending external review**», sezioni Layout, What "non-isomorphic" means here (righe 173-181), Verify it yourself, Counting convention, Citing, License («© 2026 Andrea Maiorana… CC BY 4.0… Developed by the author with the help of Claude Code (Anthropic).»). |
| `LICENSE` (18657 byte) | Testo CC BY 4.0: copiarlo identico. |

Isomorfismo nel k18 (README righe 175-177, testuale): «relabeling the lines
and reversing the traversal direction of individual lines». Il codice della
ricerca di isomorfismo del k18 non è nel repository: la forma canonica va
scritta qui (sotto).

`git -C /home/rufio72/git/kobon_triangles_k18 status --short` oggi stampa
esattamente due righe: `?? compiti/` e `?? esperimenti/`.

### I 21 sotto-17 (misurati dall'analista con script di prova indipendente)

Per ogni voce di `risultati_sotto17.json` e ogni indice `r` con
`sotto17[r] == 85`: tolta la retta `r`, il sotto-arrangiamento conta 85
(metodo a eventi), **non ha punti multipli né coppie parallele** (tutti e 21
semplici), e ogni retta borda esattamente 15 triangoli (k−2, come impone il
tetto di Tamura). Elenco nell'ordine del file, con id e classe da assegnare:

| id | fonte | retta tolta | degenerazioni della fonte | classe |
|---|---|---|---|---|
| k17_01 | all_43 | 0 | triple [[0,10,17]] | 1 |
| k17_02 | all_134 | 4 | triple [[0,4,10]] | 2 |
| k17_03 | all_273 | 12 | — | 3 |
| k17_04 | all_336 | 17 | — | 4 |
| k17_05 | all_354 | 10 | — | 5 |
| k17_06 | all_479 | 15 | — | 6 |
| k17_07 | all_514 | 2 | — | 3 |
| k17_08 | all_608 | 11 | — | 7 |
| k17_09 | all_832 | 8 | — | 5 |
| k17_10 | all_839 | 16 | — | 5 |
| k17_11 | all_897 | 5 | — | 6 |
| k17_12 | all_953 | 3 | — | 4 |
| k17_13 | all_1032 | 1 | — | 1 |
| k17_14 | all_1052 | 5 | — | 5 |
| k17_15 | all_1152 | 5 | — | 5 |
| k17_16 | all_1558 | 17 | — | 1 |
| k17_17 | all_1562 | 13 | — | 1 |
| k17_18 | all_1761 | 12 | — | 8 |
| k17_19 | sol4 | 17 | triple [[3,8,17],[9,15,17],[10,14,17]] | 1 |
| k17_20 | sol9 | 1 | triple [[1,4,14],[1,9,17]] | 4 |
| k17_21 | sol11 | 13 | triple [[2,8,13],[11,13,14]] | 6 |

Classi (numerate per prima occorrenza; rappresentante = prima occorrenza):
1: {01,13,16,17,19} (5) · 2: {02} (1) · 3: {03,07} (2) · 4: {04,12,20} (3)
· 5: {05,09,10,14,15} (5) · 6: {06,11,21} (3) · 7: {08} (1) · 8: {18} (1).
Totale 8 classi. Misurato con la forma canonica qui sotto; verificato anche
che è invariante sotto rinumerazione casuale delle rette, cambio di segno
dei coefficienti di rette a caso e riflessione x → −x (5 prove su k17_01).

## Struttura da realizzare

```
kobon_triangles_k17/
├── compiti/01-pacchetto-k17.md   (questo file, non toccarlo)
├── build/estrai.py               interno, italiano: legge il k18, scrive il JSON
├── k17_solutions.json            i 21 arrangiamenti + elenco classi
├── verify_direct_exact.py        verificatore 1: definizione classica, tutte le voci
├── classify.py                   verificatore 2: eventi, conteggio, forma canonica, classi
├── README.md  LICENSE  SHA256SUMS.txt
```

### `build/estrai.py`

Unico file che conosce il k18 (costante `K18 = Path("/home/rufio72/git/kobon_triangles_k18")`,
sovrascrivibile con `--k18`). Legge `risultati_sotto17.json`, prende le voci
con 85 nell'ordine del file, carica le rette (logica di `carica_tutti`),
toglie la retta, calcola forma canonica e classi importando `classify.py`
dalla radice del pacchetto, e scrive `k17_solutions.json` con
`json.dump(..., indent=1)` e newline finale. Deve fallire (assert) se le
voci non sono 21, se un conteggio non è 85 o se ci sono degenerazioni non
attese. Stampa in ultima riga: `scritto k17_solutions.json: 21 arrangiamenti, 8 classi`.

### Formato di `k17_solutions.json`

```
{ "k": 17, "triangles": 85, "tamura_bound": 85,
  "note": "<inglese: cosa sono, convenzione a·x+b·y=c, lines_float solo per grafici>",
  "source": "k18_final (A. Maiorana, 2026): 2357 arrangements of 18 lines with 93 triangles",
  "count": 21, "class_count": 8,
  "arrangements": [ {
      "id": "k17_01", "class": 1,
      "source_arrangement": "all_43", "removed_line": 0,
      "source_line_indices": [1, 2, ..., 17],      // indice nel 18 di ogni retta tenuta
      "source_triple_points": [[0,10,17]], "source_parallel_pairs": [],
      "lines_frac": [17 terne "p/q" copiate identiche dalla fonte],
      "lines_float": [17 terne float(Fraction(...))],
      "triple_points": [], "parallel_pairs": [],   // degenerazioni residue, ricalcolate
      "canonical_sha256": "<hex>" }, ... ],
  "classes": [ {"class": 1, "representative": "k17_01",
                "members": ["k17_01","k17_13","k17_16","k17_17","k17_19"],
                "canonical_sha256": "<hex>"}, ... ] }
```

Le stringhe `lines_frac` si copiano dalla fonte senza riscriverle.
`canonical_sha256` = `hashlib.sha256(json.dumps(forma, separators=(",", ":")).encode()).hexdigest()`
dove `forma` è la tupla restituita da `canonical_form` (diventa liste annidate).

### `verify_direct_exact.py`

Adattamento del file del k18 (stessa logica, stesso docstring in inglese
aggiornato a k=17). Uso: `python3 verify_direct_exact.py [k17_solutions.json] [--id k17_05]`.
Per ogni voce: rette distinte, coppie parallele = `parallel_pairs`
dichiarate, conteggio classico = 85, e in più conteggio per retta (ogni
retta è lato di esattamente 15 triangoli contati). Una riga per voce
`k17_01: 85 triangles, 15 per line — PASS`, ultima riga
`DIRECT EXACT: 21/21 PASS`; codice di uscita 0 solo se tutto passa.

### `classify.py`

Funzioni (nomi fissati, `build/estrai.py` le importa):

- `events(lines) -> (ev, parallel_pairs, multiple_points)`: come
  `verify_events.py` righe 37-60 (lista per retta di `frozenset` ordinati
  per `s`; coppie parallele ordinate; punti multipli come terne ordinate).
- `count_by_adjacency(ev) -> int`: righe 66-81 di `verify_events.py`.
- `canonical_form(ev) -> tuple`: forma canonica completa per «rinumerazione
  delle rette + verso di percorrenza di ogni singola retta». Da copiare:

```python
def canonical_form(ev):
    k = len(ev)
    best = None
    for x in range(k):
        for seq in (ev[x], ev[x][::-1]):
            crossing = set().union(*seq)
            paral = sorted(set(range(k)) - crossing - {x})
            groups = [sorted(e) for e in seq] + ([paral] if paral else [])
            for perms in itertools.product(
                    *[list(itertools.permutations(g)) for g in groups]):
                lab = {x: 0}
                for p in perms:
                    for j in p:
                        lab[j] = len(lab)
                form = []
                for i in sorted(range(k), key=lambda i: lab[i]):
                    s = tuple(tuple(sorted(lab[j] for j in e)) for e in ev[i])
                    form.append(min(s, s[::-1]))
                form = tuple(form)
                if best is None or form < best:
                    best = form
    return best
```

  Perché è completa (da riportare in breve nel README): la retta scelta
  come retta 0 e il suo verso fissano le etichette delle altre nell'ordine
  in cui la incrociano; i pareggi (punto multiplo, rette parallele a x) si
  enumerano tutti; la forma è il minimo su tutti i candidati, quindi due
  arrangiamenti hanno la stessa forma se e solo se sono isomorfi nel senso
  sopra. Costo misurato: 21 arrangiamenti in meno di 1 s.
- `self_test(lines, trials=5, seed=1) -> bool`: rinumera a caso le rette,
  cambia segno a (a,b,c) di rette a caso, applica x → −x (a → −a) e
  controlla che `canonical_form` non cambi.
- `main()`: uso `python3 classify.py [k17_solutions.json] [--self-test]`.
  Per ogni voce ricalcola eventi, conteggio (= 85), degenerazioni (=
  dichiarate), `canonical_sha256` (= dichiarato); raggruppa per forma e
  controlla che le classi coincidano con `classes` del JSON. Stampa una riga
  per classe `class 1: 5 arrangements (k17_01 k17_13 k17_16 k17_17 k17_19)`
  e in ultima riga `CLASSES: 8 distinct, 21 arrangements — PASS`. Con
  `--self-test` stampa prima `SELF-TEST: canonical form invariant on 21/21 — PASS`.

### `README.md` (inglese, sul modello del k18)

Titolo tipo `# k17 — 21 optimal arrangements of 17 lines (85 triangles), 8 isomorphism classes`.
Tabella: Lines 17; triangles 85; Tamura's bound ⌊17·15/3⌋ = 85, so optimal;
best previously published: 85 (a single arrangement, by Bader);
arrangements 21; isomorphism classes **8**; all simple (no triple points,
no parallel pairs); arithmetic exact rationals. Riquadro «Status: not yet
independently confirmed … **verified in-house, pending external review**».
Sezioni: How they were obtained (dal k18, togliendo una retta; 5 da
arrangiamenti con punti tripli togliendo una retta che passa per tutti i
loro punti tripli, 16 da arrangiamenti semplici); tabella delle 8 classi
(classe, rappresentante, membri con fonte/retta tolta); What "non-isomorphic"
means here (definizione + forma canonica completa, non solo un invariante);
il confronto con la soluzione di Bader **non è stato fatto** (nessuna
coordinata disponibile nel progetto): dirlo esplicitamente; Layout; Verify
it yourself (i comandi delle verifiche 3-5); Counting convention (come k18);
Citing; License (© 2026 Andrea Maiorana, CC BY 4.0, stessa frase su Claude Code).

### `SHA256SUMS.txt`

`sha256sum` di tutti i file pubblicati tranne sé stesso e tranne
`compiti/` (ordine alfabetico, percorsi relativi): `LICENSE README.md
build/estrai.py classify.py k17_solutions.json verify_direct_exact.py`.
Va rigenerato per ultimo, dopo ogni modifica al README.

## Verifiche obbligatorie

Tutte da `/home/rufio72/git/kobon_triangles_k17`. Uscite di formato
prescritte dal brief; i numeri (85, 15, classi e membri) misurati dall'analista.

1. `git init` fatto: `git -C /home/rufio72/git/kobon_triangles_k17 rev-parse --is-inside-work-tree` → `true`.
2. `python3 -I build/estrai.py` → ultima riga
   `scritto k17_solutions.json: 21 arrangiamenti, 8 classi`.
3. `python3 -I verify_direct_exact.py` → 21 righe `… 85 triangles, 15 per line — PASS`,
   ultima `DIRECT EXACT: 21/21 PASS`, codice 0.
4. `python3 -I classify.py --self-test` → `SELF-TEST: canonical form invariant on 21/21 — PASS`,
   8 righe di classe con dimensioni 5,1,2,3,5,3,1,1 e membri come nella
   tabella sopra, ultima `CLASSES: 8 distinct, 21 arrangements — PASS`.
5. `sha256sum -c SHA256SUMS.txt` → 6 righe tutte `OK`.
6. `python3 -I -c "import json;d=json.load(open('k17_solutions.json'));a=d['arrangements'];print(len(a),sorted({len(x['lines_frac']) for x in a}),sum(len(x['triple_points'])+len(x['parallel_pairs']) for x in a),len(d['classes']))"`
   → `21 [17] 0 8`.
7. Controllo di provenienza: per k17_19 (sol4, retta 17)
   `python3 -I -c "import json;a=json.load(open('k17_solutions.json'))['arrangements'][18];s=json.load(open('/home/rufio72/git/kobon_triangles_k18/sol4/lines_rational.json'))['lines_frac'];print(a['lines_frac']==s[:17])"` → `True`.
8. `wc -l *.py build/*.py | awk '$1>800 && $2!="totale" && $2!="total"'` → vuoto.
9. `git -C /home/rufio72/git/kobon_triangles_k18 status --short` → esattamente
   `?? compiti/` e `?? esperimenti/` (invariato), e
   `find /home/rufio72/git/kobon_triangles_k18 -newer /home/rufio72/git/kobon_triangles_k17/compiti/01-pacchetto-k17.md -type f` → vuoto.
10. `grep -c 'pending external review' README.md` → almeno 1;
    `cmp LICENSE /home/rufio72/git/kobon_triangles_k18/LICENSE` → nessuna uscita.

## Consegna

File sopra, `git init` senza commit. Messaggio di commit proposto (nel
rapporto): `Pacchetto k17: 21 arrangiamenti ottimali a 17 rette, 8 classi`
con riga finale `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
Rapporto finale di massimo trenta righe: uscita delle verifiche 2-10
(ultime righe), numero di classi trovato, eventuali deviazioni dal brief.
Se il numero di classi non è 8 o i membri differiscono, non correggere a
mano: riportare le due tabelle a confronto.

## Domande aperte

1. **Confronto con la soluzione pubblicata di Bader a 17 rette**: nel
   progetto non ci sono le sue coordinate, quindi non si può dire se una
   delle 8 classi sia la sua. Proposta: il README lo dichiara aperto
   («comparison with Bader's arrangement not yet done»); trascrivere le
   coordinate di Bader e confrontarle sarà un compito a parte. (scelta di progetto)
2. **Granularità dei file**: un unico `k17_solutions.json` con 21 voci e
   l'elenco classi, senza cartelle per classe né figure. Proposta: così;
   figure delle 8 rappresentanti eventualmente in un compito successivo.
   (preferenza)
3. **Pubblicare tutte le 21 occorrenze o solo le 8 rappresentanti**:
   proposta tutte e 21, con classe e provenienza, perché mostrano che la
   stessa soluzione a 17 sta dentro più soluzioni a 18 (dato interessante
   in sé). (preferenza)
4. **`build/estrai.py` nel pacchetto pubblicato**: dipende dal k18 tramite
   percorso assoluto. Proposta: tenerlo (documenta la provenienza), con
   `--k18` per altri percorsi e menzione nel README. (preferenza)
5. **Nome della cartella nel README** (`k17_final` come `k18_final`?).
   Proposta: `k17_final` nel blocco Layout e nella citazione. (preferenza)

## Decisioni della chat principale (2026-10-23)

1. Confronto con la soluzione a 17 di Bader (scelta di progetto): si segue la proposta — il README lo dichiara aperto. In parallelo la chat principale recupera la tavola degli attraversamenti di Bader dalla galleria di Savchuk; se utilizzabile, il confronto diventa un compito separato (02) e il README si aggiorna prima della pubblicazione.
2-5. Preferenze: accettate tutte le proposte del brief (un solo JSON senza cartelle né figure; pubblicare tutte le 21 occorrenze con classe e provenienza; build/estrai.py nel pacchetto con --k18; nome k17_final).

## Esito (2026-10-23)

Ciclo passato al primo colpo. Sviluppatore: 10/10 verifiche passate. Verificatore indipendente: 10/10 passate (non ha confrontato i membri delle classi con la tabella del brief, confronto però già fatto dallo sviluppatore). Token: analista (Opus) 51k, sviluppatore (Opus) 46k, verificatore (Sonnet) 15k.
