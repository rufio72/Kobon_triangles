# Compito 02: Censimento completo (arrangiamenti degeneri inclusi), n=3..10

Data: 2026-10-23. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Estendere il censimento del compito 01 (solo arrangiamenti semplici) a TUTTI i
tipi di arrangiamenti di pseudorette al massimo noto di triangoli (OEIS
A006066: 1,2,5,7,11,15,21,25 per n=3..10), ammettendo punti multipli (tre o
più rette per un punto) e classi di rette parallele. Priorità assoluta: n=8,
dove il massimo 15 non è raggiungibile da arrangiamenti semplici (massimo
semplice 14, dimostrato nel compito 01) e la soluzione pubblicata a 15 ha due
punti tripli. I risultati si AGGIUNGONO alle cartelle `k3`..`k10`: file nuovi
per i tipi degeneri e una sezione in coda ai README.

### Vincoli assoluti

- Niente modifiche fuori da `/home/rufio72/git/kobon/census/` e dalle
  aggiunte in `k3/`..`k10/`. In `k3`..`k10` NON modificare né cancellare i
  file esistenti (`tipoN.txt`, `soluzioni.json`); ai `README.md` si può solo
  AGGIUNGERE una sezione in coda (append puro: `git diff` senza righe
  rimosse). Non toccare `k14/ k16/ k17/ k18/` né `census/compiti/01-*`.
- Il binario `census-kobon` e il suo comportamento non cambiano: i file
  `src/{tabella,cnf,sat,wiring,bruto,risultati,costanti,main}.{c,h}` restano
  come sono, salvo che il Makefile va ristrutturato con liste esplicite di
  oggetti (oggi compila `src/*.c` in un solo binario: con due main collide).
  `./census-kobon 3` e `./census-kobon 5` devono dare gli stessi risultati di
  prima (fixture sotto).
- Codice nuovo in C (C11), `gcc -Wall -Wextra` senza warning, nessun file di
  codice sopra le 800 righe. Commenti e documenti in italiano; eccezioni: i
  README di `k{n}` in inglese (convenzione già decisa nel compito 01) e i
  file copiati da `k17`/`k18/esperimenti/isomorfismo`, che restano con i loro
  commenti inglesi.
- SAT solver: kissat 4.0.4, `/home/rufio72/.local/bin/kissat` (nel PATH,
  verificato con `which kissat`). Solo processo esterno.
- GMP è installata ed è richiesta per il riscontro con dati razionali esatti:
  header `/usr/include/x86_64-linux-gnu/gmp.h` (nel path di default di gcc),
  `libgmp.so` presente (verificato con `ldconfig -p`). Linkare con `-lgmp`.
- Niente Python nel prodotto (ammessi `Makefile` e script shell).
- Il repo è git, albero pulito (misurato). Niente push.

## Cosa esiste già (fatti verificati)

### Risultati del compito 01 (da non alterare)

Tipi SEMPLICI al massimo noto, per n=3..10: **1, 1, 1, 2, 1, 0, 1, 36**
(esito del compito 01; file: `k{n}/tipoN.txt` per n=3..7 e 9, `k8/` senza
tipi, `k10/soluzioni.json` con 36 tabelle; verificato con `ls`). Fixture
misurate (compito 01): canonica n=3 `[[2,3],[1,3],[1,2]]`; canonica n=5
`[[2,4,3,5],[1,4,5,3],[4,1,5,2],[3,1,2,5],[1,3,2,4]]`. Il compito 01 ha
inoltre dimostrato UNSAT a T+1 per gli arrangiamenti SEMPLICI di ogni
n=3..10 (aritmetica dei segmenti oppure kissat).

### Convenzione di conteggio dei triangoli con degenerazioni

È quella dei dati reali `k18/` (README, sezione «Counting convention») e del
verificatore `k18/verify_events.py` (88 righe, letto per intero), validata su
2357 arrangiamenti reali: una terna {x,y,z} conta come triangolo ⇔

1. ogni coppia della terna si incrocia (nessuna coppia parallela);
2. i tre incroci non sono un punto solo (basta controllare su UN ospite:
   posizione di y ≠ posizione di z lungo x);
3. su ciascuna delle tre rette, gli EVENTI delle altre due sono CONSECUTIVI
   nella sequenza degli eventi (|pos−pos| = 1).

Un «evento» su una retta è l'insieme delle rette incontrate in uno stesso
punto (≥2 rette = punto multiplo). Una retta che passa per un vertice del
triangolo lo tocca soltanto: il triangolo conta lo stesso. Più triangoli
possono condividere un vertice in un punto multiplo, e anche un LATO (vedi
sotto): è la convenzione delle soluzioni record pubblicate.

### La soluzione pubblicata per n=8 a 15 triangoli (riscontro)

La galleria LineOrder (zegalur) parte da n=10 e NON contiene n=8
(verificato scaricando la pagina). Il riscontro certificato viene dalla
galleria collegata a OEIS A006066:
`https://raw.githubusercontent.com/ud1/kobon-solutions/HEAD/gallery/certificates/8/8-1yvz24guzamh9.json`
(8 rette razionali esatte `a·x+b·y=c`, campo `lines_frac`; nessuna LICENSE
nel repo: sono dati fattuali, citare l'URL). Coefficienti (misurati, copiare
esatti):

```
riga 0: ( 0,    -1,  9805/13478)      riga 4: ( 17/2, -1,  277/2)
riga 1: (-1/2,  -1,  27/2)            riga 5: ( 2,    -1,  153053/13478)
riga 2: (-3/2,  -1, -76090/6739)      riga 6: ( 1,    -1,  117795/13478)
riga 3: (-7/2,  -1, -20433/6739)      riga 7: ( 1/2,  -1, -11/2)
```

Misurato oggi con aritmetica razionale esatta (stesso algoritmo di
`verify_events.py`): **15 triangoli**, punti tripli **(0,2,6)** e
**(3,5,6)** (0-based), **nessuna coppia parallela**. Sequenze di eventi
(0-based, ordinate lungo ogni retta; fixture da cablare nell'autotest):

```
0: [1][7][3][5][2 6][4]      4: [3][1][2][0][6][7][5]
1: [0][7][6][5][3][4][2]     5: [1][3 6][0][2][7][4]
2: [3][7][5][0 6][4][1]      6: [1][3 5][0 2][4][7]
3: [2][7][0][5 6][1][4]      7: [1][0][3][2][5][4][6]
```

ATTENZIONE, fatto che cambia il modello: qui 3·15 = 45 lati ma i segmenti
finiti sono solo 42 (retta 6: 4; rette 0,2,3,5: 5; rette 1,4,7: 6). Con i
punti multipli due triangoli possono condividere un segmento (uno per lato):
l'aritmetica «3T ≤ n(n−2)» del compito 01 e il modello «ogni segmento è lato
di un triangolo salvo m mancanti» NON valgono più. Il conteggio dei triangoli
va imposto direttamente (cardinalità sulle variabili di triangolo).

### Il modello di riferimento non copre le degenerazioni

Dal repo `zegalur/kobon-cnf` (clonato in sola lettura) e dal paper
arXiv:2507.07951 (sezioni 2-3 lette): la NOTAZIONE a tabella del paper copre
parallele e punti multipli (par. 2.1: numerazione in senso orario lungo un
cerchio che racchiude gli incroci, classi di parallele con numeri
consecutivi), ma il modello CNF (sez. 3 e `koboncnf.py`) è SOLO per
arrangiamenti semplici; il paper lo dice espressamente («parallel lines and
multi-line cross points ... are underutilized in this research»). L'opzione
`-L` di `gen.py` elenca per indice le righe che possono avere UNA mancanza
ciascuna (voci ripetute = più mancanze sulla stessa retta): i 120 multiset di
3 righe su n=8 provati nel compito 01 esauriscono le distribuzioni delle 3
mancanze nel modello SEMPLICE, e l'UNSAT conferma solo che il massimo
semplice è 14. Il modello degenere va progettato qui (sotto).

### Codice riusabile (da `grafo` non serve: file corti letti per intero)

- `k18/esperimenti/isomorfismo/{geom.h,geom.c,canon.h,canon.c,json_min.h,
  json_min.c,sha256.h,sha256.c}` (66+259+24+236+34+421+~2×100 righe):
  pacchetto autonomo in C con GMP, già usato per il ricontrollo k18 del
  2026-10-23. Tipi e funzioni chiave (firme esatte da `geom.h`/`canon.h`):
  - `events_t` (`GEOM_MAXK 64`): per ogni retta la sequenza di eventi;
    l'evento e della retta i è `mem[i][start[i][e] .. start[i][e+1])`.
  - `void geom_events(const zline *lines, int k, events_t *ev, tuple_list
    *par, tuple_list *multi)`: eventi, coppie parallele e punti multipli
    esatti da rette razionali.
  - `int geom_count_by_adjacency(const events_t *ev)`: conteggio triangoli
    con il criterio di adiacenza qui sopra (validato sui dati k18).
  - `void canonical_form(const events_t *ev, events_t *form)` e
    `canonical_sha256`: forma canonica completa per la relazione
    «rietichettatura + inversione del verso di ciascuna retta presa da sola»
    (gestisce già pareggi = punti multipli e rette che non si incrociano =
    parallele). NOTA: questo gruppo (per-retta) è più largo del gruppo del
    censimento (sotto): usarlo come INVARIANTE PONTE verso i dati reali, non
    come definizione di tipo.
  Copiare questi 8 file in `census/src/` così come sono (commenti inglesi
  ammessi).
- `census/src/*.h` del compito 01 (letti per intero): `sat_risolvi(kissat,
  cnf_path, out_path, nvar, val)` riusabile tal quale; in `cnf.c` c'è il
  contatore sequenziale di Sinz «al più m» da riusare come modello; il ciclo
  di enumerazione con ripartenza è in `src/main.c` (340 righe, leggibile come
  esempio); `costanti.h` ha `N_MAX 12` e `MAXT[]` riusabili.
- `k18/sol1/lines_rational.json`: n=18, `count: 93`, 4 punti tripli
  (dichiarati nel README k18) — collaudo di `geom_events` e del conteggio su
  dati reali ricchi di degenerazioni.

## Struttura da realizzare

Tutto in `census/src/` (nomi liberi ma indicativi), nuovo binario
`census-degeneri` (linkato con `-lgmp`), script `esegui-degeneri.sh`.
Il Makefile elenca esplicitamente gli oggetti dei due binari.

| file nuovo        | contenuto                                               | ~righe |
|-------------------|---------------------------------------------------------|--------|
| `src/dtab.c/.h`   | tipo TabellaD (righe di eventi, 0-based, paralleli assenti dalla riga), lettura/scrittura, confronto, conversione a `events_t` | 250 |
| `src/dorbita.c/.h`| wiring generalizzato (validazione), orbita ≤4n, tabella canonica | 300 |
| `src/dcnf.c/.h`   | modello CNF degenere (sotto), blocco, Sinz, modo `--controlla` | 450 |
| `src/dbruto.c/.h` | DFS esaustiva dei wiring degeneri, n≤6 (n=7 facoltativa) | 220 |
| `src/dmain.c`     | CLI, ciclo di enumerazione, autotest, riscontro, scrittura risultati | 400 |
| `census/dati/k8-pubblicata.json` | copia del certificato ud1 adattata al formato k18 (`lines_frac`, `count: 15`, campi dichiarati), con campo `source_url` | — |

CLI: `./census-degeneri <n> [--out DIR] [--unsat] [--brute] [--con-semplici]
[--controlla FILE] [--autotest] [--riscontro JSON] [--canone FILE]
[--kissat PATH] [--tmp DIR]`.

### Rappresentazione e gruppo di isomorfismo

TabellaD: riga r = sequenza di eventi lungo la retta r nel verso della
numerazione; evento = insieme di rette (≥2 = punto multiplo); le rette
parallele a r non compaiono nella riga r; classi di parallele con numeri
consecutivi (convenzione del paper, par. 2.1). Caso semplice = Tabella del
compito 01. Il parallelismo è TRANSITIVO (semantica delle rette euclidee,
coerente con k18; vedi Domande aperte 1).

Gruppo del censimento = stesso della fase 1, generalizzato: le ≤4n
presentazioni date dalla costruzione del paper (scelta della retta #1 e del
suo estremo d'ingresso: 2n; verso di percorrenza del cerchio: ×2 = specchio).
Tipo = classe; tabella canonica = minimo lessicografico delle presentazioni
valide (confronto riga per riga, evento per evento come `canon_cmp`:
prefisso più corto minore, eventi come insiemi ordinati).

Orbita per via geometrica, non con formule: (1) dalla tabella ricostruisci il
wiring generalizzato (greedy: permutazione iniziale identità; a ogni passo
cerca un insieme S di ≥2 rette ADIACENTI nella permutazione tale che per ogni
u∈S il prossimo evento non consumato della riga u sia esattamente S∖{u};
rovescia S come blocco e avanza i cursori; valida ⇔ tutte le righe si
consumano; le coppie parallele non si incrociano mai). (2) ordine ciclico del
bordo: ingressi (permutazione iniziale, dall'alto in basso) poi uscite
(permutazione finale, dal basso in alto). (3) per ciascuno dei 2n estremi di
partenza e 2 versi di percorrenza: numera le rette nell'ordine del primo
estremo incontrato, verso di ogni retta dal suo primo estremo; ricostruisci
la TabellaD; scarta le presentazioni con classi di parallele non consecutive.
CALIBRATURA OBBLIGATORIA: sui casi semplici l'orbita deve coincidere con
`tab_orbita` del compito 01 (fixture n=3 orbita di taglia 2, n=5 taglia 2,
canoniche sopra); la presentazione «identità» deve restituire la tabella di
partenza. Se i versi non tornano, aggiustare la direzione della camminata
finché le fixture passano: le convenzioni si fissano empiricamente lì.

### Modello CNF degenere (progettato qui; da validare con `--controlla`)

Rette 0-based. Variabili:
- `P(i,j)`, i<j: parallele.
- `T3(a,b,c)`, a<b<c: per un punto comune.
- `X(r,i,j)`, r,i,j distinti: r incrocia i e j e l'evento di j viene
  STRETTAMENTE dopo quello di i lungo r.
- `adj(r,{i,j})`: eventi di i e j consecutivi su r (ausiliaria).
- `tri(a,b,c)`, a<b<c: la terna è un triangolo.
- ausiliarie `b1/b2` (sotto) e contatori di Sinz.

Clausole (∀ indici validi; `T(x,y,z)`/`P(x,y)` con indici riordinati):

- D1 classi consecutive (⇒ transitività): per i<k<j: ¬P(i,j) ∨ P(i,k) e
  ¬P(i,j) ∨ P(k,j).
- D2 niente ordine senza incrocio: ¬X(r,i,j) ∨ ¬P(r,i); ¬X(r,i,j) ∨ ¬P(r,j).
- D3 tricotomia per riga: X(r,i,j) ∨ X(r,j,i) ∨ T(r,i,j) ∨ P(r,i) ∨ P(r,j);
  mutua esclusione a coppie fra X(r,i,j), X(r,j,i), T(r,i,j).
  (Se i∥j ma entrambe incrociano r, i due incroci sono distinti: la
  tricotomia resta e T è escluso da D4.)
- D4 punti multipli solo fra rette incidenti: ¬T(a,b,c) ∨ ¬P(a,b) (3 clausole
  per terna).
- D5 transitività sulla riga r (con pareggi): ¬X(r,i,j) ∨ ¬X(r,j,k) ∨ X(r,i,k);
  ¬T(r,i,j) ∨ ¬X(r,j,k) ∨ X(r,i,k); ¬X(r,i,j) ∨ ¬T(r,j,k) ∨ X(r,i,k).
- D6 chiusura del punto comune: ¬T(a,b,c) ∨ ¬T(a,b,d) ∨ T(a,c,d) (per tutte
  le quaterne, nelle combinazioni che condividono due indici).
- D7 coerenza delle terne (convenzione di numerazione; generalizza la
  clausola 11-X del compito 01). Per ogni terna ordinata (r,i,j), compagni
  C1, C2 di X(r,i,j) secondo la tabella del compito 01 (qui ricopiata, con X
  al posto di G):

  | caso    | X(r,i,j) ⇒ C1 ∧ C2  | X(r,j,i) ⇒ C1' ∧ C2' |
  |---------|----------------------|-----------------------|
  | r<i<j   | X(i,r,j), X(j,r,i)   | X(i,j,r), X(j,i,r)    |
  | r<j<i   | X(j,i,r), X(i,j,r)   | X(j,r,i), X(i,r,j)    |
  | i<r<j   | X(i,r,j), X(j,i,r)   | X(i,j,r), X(j,r,i)    |
  | j<r<i   | X(j,i,r), X(i,r,j)   | X(j,r,i), X(i,j,r)    |
  | i<j<r   | X(i,j,r), X(j,i,r)   | X(i,r,j), X(j,r,i)    |
  | j<i<r   | X(j,r,i), X(i,r,j)   | X(j,i,r), X(i,j,r)    |

  Con guardia GU = P(r,i) ∨ P(r,j) ∨ P(i,j) ∨ T(r,i,j), per ogni direzione
  (a,b) della coppia: ¬X(r,a,b) ∨ C1 ∨ GU; ¬X(r,a,b) ∨ C2 ∨ GU;
  X(r,a,b) ∨ ¬C1 ∨ ¬C2 ∨ GU. Le guardie indeboliscono e basta: ciò che
  sfugge lo scarta la validazione wiring.
- D8 adiacenza. Ausiliarie per d ∉ {r,i,j}: b1(r,{i,j},d) ⇔ X(r,i,d)∧X(r,d,j);
  b2 ⇔ X(r,j,d)∧X(r,d,i) (Tseitin completo, 3 clausole per verso).
  adj(r,{i,j}) ⇔ (X(r,i,j) ∨ X(r,j,i)) ∧ ⋀_d ¬b1 ∧ ⋀_d ¬b2:
  ¬adj ∨ X(r,i,j) ∨ X(r,j,i); ¬adj ∨ ¬b1(d); ¬adj ∨ ¬b2(d);
  adj ∨ ¬X(r,i,j) ∨ ⋁_d (b1(d) ∨ b2(d)); idem con X(r,j,i).
- D9 triangolo: tri(a,b,c) ⇔ adj(a,{b,c}) ∧ adj(b,{a,c}) ∧ adj(c,{a,b})
  (l'adiacenza implica già incrocio e vertici distinti): ¬tri ∨ adj (×3);
  tri ∨ ¬adj(a,·) ∨ ¬adj(b,·) ∨ ¬adj(c,·).
- D10 cardinalità: censimento = esattamente T triangoli (Sinz «≤T» sui tri E
  «≤ C(n,3)−T» sui ¬tri); `--unsat` = almeno T+1 (solo «≤ C(n,3)−T−1» sui
  ¬tri).
- D11 almeno una degenerazione: ⋁ P(i,j) ∨ ⋁ T(a,b,c). Attiva nel censimento
  (i semplici sono già censiti nel compito 01), OMESSA con `--unsat` (la
  prova del tetto dev'essere autonoma) e con `--con-semplici` (controllo
  incrociato, n≤6).
- D12 blocco di una tabella: disgiunzione dei NEGATI di tutti i letterali
  strutturali veri (X, P, T3) della sua presentazione (i falsi sono
  implicati). Bloccare tutte le presentazioni valide dell'orbita.

Estrazione dal modello: paralleli da P; posizione di i sulla riga r = numero
di j con X(r,j,i) vero; posizioni uguali = stesso evento. Poi: validazione
wiring; conteggio con `geom_count_by_adjacency` sulla conversione a
`events_t` (deve dare T, altrimenti abort: incoerenza del modello); tabella
canonica; se nuova registrala; in OGNI caso blocca l'orbita e ripeti fino a
UNSAT. Ripartenza come nel compito 01 (file di stato in `census/tmp/`,
es. `stato-deg-n<n>.txt` con tutte le tabelle bloccate).

`--controlla`: genera il CNF (senza D11/D12) e VERIFICA che l'assegnamento
indotto da una TabellaD data soddisfi ogni clausola, stampando le violate.
È la difesa principale contro un modello troppo forte (che perderebbe tipi).

### Forza bruta indipendente (`--brute`, n≤6)

DFS sui wiring degeneri: stato = permutazione + coppie incrociate; mosse =
rovesciare un blocco ADIACENTE di taglia ≥2 a coppie non ancora incrociate
(taglia m = punto di molteplicità m); in ogni stato è lecito FERMARSI se la
relazione «mai incrociate» è transitiva (classi parallele). A ogni foglia:
eventi, conteggio per adiacenza, se = T dedup per tabella canonica. Il totale
a T deve essere = semplici (fase 1) + degeneri (questo censimento). n=7
facoltativa se resta sotto qualche minuto.

### Autotest (`--autotest`), tutti obbligatori

1. Riscontro k8: da `census/dati/k8-pubblicata.json` con `geom_events` →
   15 triangoli, punti multipli (0,2,6) e (3,5,6), zero coppie parallele
   (valori misurati, sopra); eventi identici alla fixture qui sopra.
2. `--controlla` sulla presentazione in forma di convenzione della soluzione
   pubblicata (ottenuta via orbita): almeno una presentazione valida
   soddisfa TUTTE le clausole (se nessuna, il modello è sbagliato: fermarsi).
3. `--controlla` sulle canoniche semplici n=3 e n=5 del compito 01 (con la
   loro T: 1 e 5): tutte le clausole soddisfatte.
4. Orbite semplici coerenti con la fase 1 (fixture n=3 e n=5, taglie 2 e 2,
   stesse canoniche).
5. k18 sol1 (`k18/sol1/lines_rational.json`, sola lettura): 93 triangoli,
   4 punti tripli (dichiarati nel file): collaudo di geom su degenerazioni
   reali con n=18 (events_t regge fino a 64 rette).

### Risultati e README (append)

Per ogni n con tipi degeneri trovati: `k{n}/degenere1.txt`, `degenere2.txt`…
tabella canonica 1-based, eventi multipli fra parentesi, es. riga
`2 8 4 6 (3 7) 5`; intestazione `#` con n, T, punti multipli (terne 1-based),
classi parallele, metodo. In coda a ogni `k{n}/README.md` (inglese, stile del
file) una sezione:

```
## Degenerate arrangements (census extension, 2026-MM-DD)

Allowing multiple points and parallel lines (counting convention of the
published records: a line through a vertex only touches the triangle),
the census at 15 triangles finds N degenerate types (degenere1.txt ...).
Total types, simple + degenerate: 0 + N.
Ceiling at 16 with degeneracies: UNSAT / open (runtime).
Method: extended CNF model (see census/), kissat 4.0.4, validation by
generalized wiring reconstruction, cross-check against the published
rational solution (ud1/kobon-solutions certificate).
```

(adattare i numeri per ogni n; per n=8 citare l'URL del certificato).
`esegui-degeneri.sh`: ordine n = 8 (priorità), poi 3..7, 9, 10; riepilogo
finale `n | T | semplici | degeneri | totale | UNSAT T+1`.

## Verifiche obbligatorie

1. `cd /home/rufio72/git/kobon/census && make` → compila `census-kobon` E
   `census-degeneri`, nessun warning con `-Wall -Wextra`.
2. `find /home/rufio72/git/kobon/census/src -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale"'`
   → vuota.
3. `./census-kobon 3` e `./census-kobon 5` → 1 tipo ciascuno, canoniche
   identiche alle fixture del compito 01 (regressione: misurate).
4. `./census-degeneri --autotest` → i 5 autotest sopra tutti OK (le uscite
   attese 15 / (0,2,6)+(3,5,6) / 0 parallele / 93 / 4 punti tripli sono
   MISURATE; il resto è atteso, non misurato).
5. `./census-degeneri 8 --out ../k8` → ALMENO 1 tipo a 15 triangoli; il
   canone-eventi (`canonical_sha256`) di uno dei tipi coincide con quello di
   `./census-degeneri --riscontro dati/k8-pubblicata.json` (confronto tra due
   uscite dello stesso strumento; il numero totale di tipi è il risultato
   principale, da riportare).
6. `./census-degeneri <n> --brute` per n=3..6 → totale a T = semplici fase 1
   + degeneri trovati, cioè 1+d3, 1+d4, 1+d5, 2+d6 (d_n dai censimenti; per
   n=3 atteso d3=0: ogni degenerazione su 3 rette distrugge l'unico
   triangolo possibile — attesa, non misurata).
7. `./census-degeneri <n> --unsat` per n=3..8 → UNSAT (attesa, non misurata:
   per le rette vere è il tetto di Tamura/Clément–Bader; a livello
   pseudorette degeneri è possibile un SAT: in tal caso validare la tabella
   e riportarla in evidenza, NON è un errore automatico). Per n=9 e 10
   lanciare con tetto di tempo (proposta: 12 h ciascuno); se scade, nel
   README «open (runtime)».
8. `git -C /home/rufio72/git/kobon status --short` → solo `census/` e
   `k3/`..`k10/`;
   `git -C /home/rufio72/git/kobon diff -U0 -- ../k3 ../k4 ../k5 ../k6 ../k7 ../k8 ../k9 ../k10 | grep '^-' | grep -v '^---'`
   (eseguito dalla radice del repo, percorsi k3..k10) → vuota: ai file
   esistenti si è solo aggiunto.
9. `./esegui-degeneri.sh` → cartelle aggiornate e riepilogo; i conteggi dei
   degeneri per n=4..10 non sono noti in anticipo: sono il risultato e vanno
   nel rapporto.

## Consegna

- Codice, dati di riscontro, risultati e README come sopra. Commit nel repo
  con messaggio in italiano e riga finale
  `Co-Authored-By: <modello usato> <noreply@anthropic.com>`. Niente push.
- Rapporto finale: massimo trenta righe, con la tabella
  `n | T | semplici | degeneri | totale | UNSAT T+1`, il numero di tipi di
  n=8 a 15 in evidenza, l'uscita delle verifiche e le deviazioni dal brief.
- Se il ponte canone-eventi (gruppo per-retta) fonde due tipi distinti per
  tabella canonica, riportarlo esplicitamente (non fonderli nei file).

## Domande aperte

1. (scelta di progetto) Parallelismo TRANSITIVO imposto nel modello e nella
   DFS (semantica delle rette euclidee; a livello pseudorette esisterebbero
   «non incroci» non transitivi). Proposta: sì, transitivo — il problema di
   Kobon è sulle rette e i dati k18/ud1 usano classi di direzione; i README
   lo dichiarano.
2. (scelta di progetto) Prova UNSAT a T+1 per n=9 e 10 con tetto di 12 ore
   ciascuna, altrimenti dichiarata aperta nel README. Il modello degenere è
   molto più largo di quello semplice e il tempo non è misurabile prima.
   Proposta: accettare il tetto; un eventuale run lungo si fa in un compito
   successivo.
3. (preferenza) Nomi: binario `census-degeneri`, file `degenereN.txt`,
   script `esegui-degeneri.sh`, dati in `census/dati/`. Copia del
   certificato ud1 (solo dati numerici, con `source_url`) committata nel
   repo. Proposta: adottare così.
4. (preferenza) Censimento con D11 (solo tipi degeneri; i semplici restano
   quelli della fase 1, mai rienumerati) e controllo incrociato dei semplici
   demandato a `--brute`/`--con-semplici` su n≤6. Proposta: adottare così.
5. (preferenza) Aggiungere in coda al README di radice del repo una riga o
   colonna con i totali semplici+degeneri (append puro). Proposta: sì.

## Decisioni della chat principale (2026-10-23)

1 (scelta di progetto): parallelismo transitivo — sì, come da proposta: l'obiettivo sono arrangiamenti di rette vere, dove la transitività è geometrica.
2 (scelta di progetto): tetto 12 h per gli UNSAT a T+1 su n=9/10 — accettato; in caso di sforamento il tetto resta dichiarato aperto nel README, l'enumerazione dei tipi al massimo non ne dipende.
3-5 (preferenze): accettate come da proposta, compreso l'aggiornamento del README di radice.

## Esito (2026-10-23)

Ciclo completato. Sviluppatore (Opus, 237,8k token, 117 chiamate, ~4h22):
censimento degenere chiuso per n=3..9, n=10 parziale (36 semplici + ≥49
degeneri, stato conservato in census/tmp/stato-deg-n10.txt, ripresa con
`./census-degeneri 10 --out ../k10`); n=8 dà 1 solo tipo degenere a 15,
hash canonico identico alla soluzione pubblicata (riscontro passato);
tetto T+1 UNSAT per n=3..9, aperto a n=10 (kissat interrotto allo
spegnimento della macchina senza verdetto). Commit dello sviluppatore:
8c173a7. Incidente dichiarato: perso lo stato della prima corsa n=10
(~2h rifatte), senza effetti sul risultato.

Verificatore (Sonnet, 16,0k token): verifiche 1-6 e 8 PASSATE (make,
righe <800, canoniche n=3/5, autotest, riscontro n=8 con sha identico,
--brute n=3..6 coincidenti, nessuna riga rimossa in k3..k10); 7 saltata
per il vincolo di tempo (visto comunque l'UNSAT a 16 per n=8), 9 non
rieseguita (n=10 fermato alle 17:50 come concordato). Unico effetto
collaterale: il rilancio di prova ha riscritto un tempo in k8/README.md
(14,6→14,7 s), ripristinato dalla chat principale.

Deviazioni accettate: --brute esteso a n=8; opzioni aggiuntive --limite,
--parziale, --parti; moduli extra dauto.c e drisultati.c per restare
sotto le 800 righe. Il seguito proposto è nel brief
03-appunto-budget-segmenti.md (da controverificare prima dell'esecuzione).
