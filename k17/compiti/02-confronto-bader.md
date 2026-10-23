# Compito 02: confronto con la soluzione di Bader (17 rette, 85 triangoli)

Data: 2026-10-23. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Stabilire se l'arrangiamento a 17 rette e 85 triangoli pubblicato da Johannes
Bader coincide, a meno di isomorfismo, con una delle 8 classi del pacchetto
oppure è una nona classe. Lo si fa applicando la forma canonica di
`classify.py` direttamente alla tavola combinatoria di Bader, con uno script
nuovo `compare_bader.py`, e riportando l'esito nel README al posto della
riga che dichiara il confronto aperto.

### Vincoli assoluti

- Niente modifiche fuori da `/home/rufio72/git/kobon_triangles_k17`.
- Il file della tavola è un dato scaricato dal web: va solo letto con
  `json.load` e validato, mai eseguito, mai passato a `eval`/`exec`/`pickle`.
  Gli script che lo leggono si lanciano con `python3 -I`.
- Non modificare `classify.py`, `verify_direct_exact.py`, `build/estrai.py`,
  `k17_solutions.json`, `LICENSE`: il nuovo script importa da `classify.py`.
- Solo libreria standard di Python 3.12.3. Niente installazioni, sudo, push.
- Non lasciare `__pycache__` (nello script: `sys.dont_write_bytecode = True`
  prima dell'import di `classify`). Non committare: il commit lo fa chi ha
  lanciato il compito (il repository non ha ancora alcun commit).
- Lingua: brief e messaggi di commit in italiano; file pubblicati (README,
  `compare_bader.py`, campo `provenance` del JSON) in inglese, come il resto
  del pacchetto.
- Nessun file di codice sopra le 800 righe (atteso `compare_bader.py`: 60-90).

## Cosa esiste già (fatti verificati)

| File | Fatto |
|---|---|
| `/tmp/claude-1000/-home-rufio72-git/e2f4b89a-00b7-4a79-8605-52b0cc8e36f0/scratchpad/bader17_table.json` | 1103 byte. Dict con chiavi `n`=17, `triangles`=85, `author`="Johannes Bader", `source`="https://zegalur.github.io/line-order/gallery/kobon.html (LineOrder Gallery #1, retrieved 2026-10-23)", `format`="Local sequences, 1-based: row i lists, in order along line i, the labels of the 16 lines it crosses.", `table` (17 righe da 16 interi). (misurato) |
| `classify.py` (210 righe) | Letto per intero. Vedi sotto. |
| `README.md` (161 righe) | Righe 90-93: paragrafo da sostituire. Righe 98-105: albero «Layout». Righe 114-124: «Verify it yourself». Riga 13: «Best previously published \| 85 (a single arrangement, by Bader)». (letto) |
| `SHA256SUMS.txt` | 6 righe, formato `sha256  percorso`, ordine alfabetico: `LICENSE README.md build/estrai.py classify.py k17_solutions.json verify_direct_exact.py`. (letto) |
| `k17_solutions.json` | chiavi `k`=17, `triangles`=85, `tamura_bound`, `note`, `source`, `count`=21, `class_count`=8, `arrangements`, `classes`. Ogni classe: `class`, `representative`, `members`, `canonical_sha256`. (misurato) |

Il repository git non ha commit; i file sono tutti non tracciati.

### `classify.py`: la forma canonica è puramente combinatoria (letto)

Nessuna coordinata entra nella forma canonica. Le funzioni utili:

- `canonical_form(ev)` (riga 94): `ev[i]` è la lista, nell'ordine lungo la
  retta `i`, degli *eventi*, ognuno un `frozenset` di indici 0-based delle
  rette incontrate in quel punto. Restituisce una tupla di tuple. Per ogni
  scelta di retta iniziale e verso, etichetta le altre nell'ordine di
  attraversamento (ties enumerati), poi per ogni retta prende il verso
  lessicograficamente minore (`min(s, s[::-1])`); il risultato è il minimo.
- `canonical_sha256(form)` (riga 128): SHA-256 di `json.dumps(form,
  separators=(",", ":"))`.
- `count_by_adjacency(ev)` (riga 73): numero di triangoli dal criterio di
  adiacenza degli eventi.
- `events(lines)` (riga 39): è l'unica funzione che usa le coordinate
  razionali; serve solo a produrre `ev`. Non serve per Bader.

Quindi la tavola di Bader si traduce in `ev` senza coordinate:
`ev[i] = [frozenset([j - 1]) for j in table[i]]` (indici 0-based, ogni
evento è un singolo incrocio: la tavola non ha punti multipli).

### Convenzioni (nessun adattamento necessario, ma da dichiarare nel README)

- Verso delle rette: la forma canonica è invariante per inversione del verso
  di ogni singola retta (docstring righe 94-103 e `--self-test`).
- Etichettatura: invariante per rinumerazione (1-based di Bader vs 0-based).
- Riflessione del piano: ogni riflessione inverte o conserva l'ordine lungo
  ciascuna retta, quindi è coperta dall'inversione per retta (il self-test
  prova `x -> -x`).
- Limite da dichiarare: la tavola della galleria è combinatoria, quindi
  l'esito vale per l'ordine di attraversamento affine delle rette. Se
  l'esito fosse «nessuna classe» e la galleria usasse sequenze cicliche
  (piano proiettivo), si dovrebbero provare le rotazioni cicliche di ogni
  riga: vedi domanda 3. Non serve se l'esito è una corrispondenza.
- La realizzabilità della tavola con rette vere non è verificata qui (la
  tavola è di Bader, che la pubblica come arrangiamento di rette); il
  README lo dice.

### Misure già fatte dall'analista (sola lettura, script nella scratchpad, nulla scritto nel progetto)

Importando `classify` e applicandolo alla tavola:

- righe: ogni riga i, con `i+1`, è una permutazione di 1..17: vero;
- `count_by_adjacency` = 85;
- `canonical_sha256` = `5834f74309210141eb194a733258c665399527d0ce5c65f6d091f459d43d5a3f`;
- coincide con la classe 5 (`k17_05`, `k17_09`, `k17_10`, `k17_14`, `k17_15`;
  5 membri, nessun punto triplo, nessuna coppia parallela) e con nessun'altra
  delle 8. Tempo 0,1 s.

Lo sviluppatore deve riottenerlo con lo script che scrive, non fidarsi di
questa riga.

## Struttura da realizzare

1. **Copia del dato**: `bader17_table.json` nella radice del pacchetto, copia
   byte per byte (`cp`) del file in scratchpad; poi aggiungere al JSON,
   rispettando i campi esistenti, un campo `provenance` (inglese): tavola
   recuperata il 2026-10-23 dalla galleria LineOrder di Savchuk
   (URL già nel campo `source`), arrangiamento di Johannes Bader, non
   verificata in coordinate. Meglio: non modificare la copia e dichiarare la
   provenienza solo nel README e nel docstring dello script (vedi domanda 2).
   Se si aggiunge il campo, `cmp` con l'originale non sarà più identico: la
   verifica 3 assume la copia non modificata.
2. **`compare_bader.py`** (radice, inglese, stile di `classify.py`: docstring
   con Usage/Requirements/Expected output). Firma e comportamento:
   - `python3 compare_bader.py [bader17_table.json] [k17_solutions.json]`
   - `sys.dont_write_bytecode = True`, poi `import classify`.
   - `load_table(path) -> list[list[int]]`: `json.load`, controlla che
     `table` sia una lista di 17 liste, ogni riga permutazione di
     `{1..17} \ {i+1}` (solo `int`, non `bool`), e che `n`/`triangles`
     coincidano con 17/85; altrimenti `SystemExit` con messaggio.
   - `to_events(table)`: come sopra, 0-based, un `frozenset` per evento.
   - Controllo di coerenza: il conteggio per adiacenza deve essere 85 e la
     tavola deve essere simmetrica come incidenza (ogni coppia si incrocia
     una volta; già garantito dal controllo delle permutazioni).
   - Calcola `classify.canonical_sha256(classify.canonical_form(ev))`,
     lo cerca tra `classes[*].canonical_sha256` di `k17_solutions.json`,
     stampa l'esito.
   - `--self-test`: ricalcola la forma dopo una rinumerazione casuale delle
     rette (seed fisso) e dopo inversione di righe casuali (la riga inversa
     va riscritta con le etichette permutate); deve restare invariata.
   - Formato di uscita (ultima riga, esatta se la corrispondenza vale):
     `BADER: class 5 (k17_05 k17_09 k17_10 k17_14 k17_15) — MATCH`
     Se nessuna classe: `BADER: no match — NEW CLASS (9th)`. Codice di
     uscita 0 in entrambi i casi se le validazioni passano; 1 se falliscono.
     Righe precedenti: `table: 17 lines, all pairs cross`,
     `triangles (adjacency): 85`, `canonical_sha256: <hash>`, e con
     `--self-test` `SELF-TEST: canonical form invariant — PASS`.
3. **README.md**:
   - sostituire le righe 90-93 (paragrafo «Comparison with Bader's
     arrangement not yet done…This is open.») con un paragrafo che riporta
     l'esito (fonte, data di recupero, tavola in `bader17_table.json`, script
     `compare_bader.py`, classe corrispondente, e la nota sul limite
     combinatorio/convenzioni sopra). Se l'esito è «classe 5»: la
     soluzione di Bader è un membro della classe 5, le altre 7 classi non
     coincidono con essa; non scrivere «nuove» per le classi 1-4, 6-8
     rispetto a Bader senza dire che il confronto è con una sola soluzione
     (non si può dire altro sull'originalità rispetto alla letteratura).
   - Riga 13 («Best previously published | 85 (a single arrangement, by
     Bader)») lasciata com'è.
   - Albero «Layout» (righe 98-105): aggiungere `bader17_table.json` e
     `compare_bader.py` con una descrizione di una riga.
   - «Verify it yourself»: aggiungere `python3 compare_bader.py --self-test`
     e una riga in elenco che spiega lo script.
4. **`SHA256SUMS.txt`**: rigenerarlo con `sha256sum` sugli 8 file, ordine
   alfabetico: `LICENSE README.md bader17_table.json build/estrai.py
   classify.py compare_bader.py k17_solutions.json verify_direct_exact.py`.

## Verifiche obbligatorie

Da eseguire nella radice del pacchetto.

1. `python3 -I compare_bader.py --self-test` — uscita attesa (ultima riga
   misurabile solo dopo la scrittura; le tre righe di valori sono misurate
   dall'analista, il formato è prescritto):
   ```
   table: 17 lines, all pairs cross
   triangles (adjacency): 85
   canonical_sha256: 5834f74309210141eb194a733258c665399527d0ce5c65f6d091f459d43d5a3f
   SELF-TEST: canonical form invariant — PASS
   BADER: class 5 (k17_05 k17_09 k17_10 k17_14 k17_15) — MATCH
   ```
   Codice di uscita 0 (`echo $?` → 0).
2. `python3 -I classify.py --self-test | tail -1` — attesa, non misurata
   (verifica che il pacchetto esistente sia intatto):
   `CLASSES: 8 distinct, 21 arrangements — PASS`
3. `cmp bader17_table.json /tmp/claude-1000/-home-rufio72-git/e2f4b89a-00b7-4a79-8605-52b0cc8e36f0/scratchpad/bader17_table.json && echo uguale`
   — attesa: `uguale` (se si sceglie di non aggiungere il campo `provenance`,
   vedi domanda 2).
4. `python3 -I -c "import json; d=json.load(open('bader17_table.json')); print(len(d['table']), {len(r) for r in d['table']})"`
   — attesa: `17 {16}`.
5. Test negativo, solo in scratchpad (non nel progetto): copiare la tavola,
   scambiare due etichette in una riga e lanciare `python3 -I
   compare_bader.py <copia>` — attesa: uscita diversa da 0 e messaggio di
   errore sulla riga non valida (attesa, non misurata).
6. `sha256sum -c SHA256SUMS.txt` — attesa: 8 righe `OK`.
7. `grep -n -i "not yet done\|This is open" README.md` — attesa: nessuna
   uscita. `grep -n "compare_bader.py\|bader17_table.json" README.md` — attesa:
   almeno tre righe (esito, Layout, Verify).
8. `git status --short` — attesa: tutti i file non tracciati
   (`?? LICENSE ?? README.md ?? SHA256SUMS.txt ?? bader17_table.json ?? build/
   ?? classify.py ?? compare_bader.py ?? compiti/ ?? k17_solutions.json
   ?? verify_direct_exact.py`) e nessun `__pycache__`:
   `find . -name __pycache__ -not -path './.git/*'` — attesa vuota.
9. `find . -name '*.py' -not -path './.git/*' | xargs wc -l | awk '$1>800 && $2!="totale"'`
   — attesa: vuota.
10. `git diff --stat -- classify.py verify_direct_exact.py k17_solutions.json`
    non serve (non tracciati): verificare invece con `sha256sum
    classify.py verify_direct_exact.py k17_solutions.json build/estrai.py`
    che gli hash siano ancora quelli della versione precedente di
    `SHA256SUMS.txt` (cfbf5a3c…, 6325bda4…, 7f8742df…, 8a07d560…).

## Consegna

- File: `bader17_table.json`, `compare_bader.py`, `README.md` e
  `SHA256SUMS.txt` aggiornati. Nessun commit (vedi vincoli).
- Rapporto finale: massimo trenta righe, con l'uscita dei comandi 1, 2, 6, 8
  e le deviazioni dal brief. Se il confronto desse un esito diverso da
  «classe 5», riportarlo in evidenza e fermarsi prima di scrivere il README.

## Domande aperte

1. Se l'esito è una corrispondenza con la classe 5, il README deve dire
   «la soluzione di Bader appartiene alla classe 5 del pacchetto; le classi
   1-4 e 6-8 sono diverse da essa». Proposta: sì, senza affermare novità
   rispetto ad altra letteratura. **Preferenza.**
2. Provenienza nel JSON: aggiungere un campo `provenance` a
   `bader17_table.json` oppure copia identica con provenienza solo in
   README e docstring? Proposta: copia identica (i campi `source`,
   `author`, `format` già dichiarano la provenienza), così la verifica 3
   resta valida. **Preferenza.**
3. Rotazioni cicliche / piano proiettivo: la galleria potrebbe descrivere
   sequenze cicliche. Con l'esito attuale (corrispondenza esatta senza
   rotazioni) non serve; se lo sviluppatore ottenesse un esito diverso,
   proposta: fermarsi e riportare, senza implementare le rotazioni. **Scelta
   di progetto** (cambierebbe la semantica dell'isomorfismo dichiarata nel
   README), ma non scatta con l'esito misurato.
4. Nome e posizione dei file (`bader17_table.json`, `compare_bader.py` nella
   radice, non in `build/`, perché sono pubblicati e verificabili dal lettore).
   **Preferenza.**

## Decisioni della chat principale (2026-10-23)

1-2, 4: preferenze accettate come da proposta. 3: non scatta con l'esito misurato; se lo sviluppatore ottiene un esito diverso si ferma e riporta, come scritto.

## Esito (2026-10-23)

Ciclo passato al primo colpo. Sviluppatore: tutte le verifiche passate, con 4 deviazioni documentate (import con -I, repo già con un commit, messaggio del test negativo, frase README sui PASS). Verificatore indipendente: 9/10 passate, la 8 fallita solo perché il brief prevedeva uno stato git precedente al commit 7573baa. Esito scientifico: la soluzione di Bader coincide con la classe 5; le classi 1-4 e 6-8 sono distinte da essa. Token: analista (Sonnet) 36k, sviluppatore (Opus) 35k, verificatore (Sonnet) 15k.
