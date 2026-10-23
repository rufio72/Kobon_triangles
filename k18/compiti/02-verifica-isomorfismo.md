# Compito 02: verifica indipendente di non-isomorfismo dei 2427 arrangiamenti k18

Data: 2026-10-23. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Verificare in modo indipendente, con uno strumento in C, che gli arrangiamenti
a 18 rette del pacchetto `k18` siano a due a due non isomorfi (isomorfismo =
rietichettatura delle rette + inversione del verso di percorrenza di singole
rette), come dichiara il `README.md`. Lo strumento calcola la forma canonica
combinatoria completa di ogni arrangiamento e conta le forme distinte su:
(a) i semplici, (b) i degeneri, (c) tutti insieme compresi i 70 C3-simmetrici.
Ogni eventuale collisione va elencata esplicitamente (coppie di id con la
stessa forma).

### Vincoli assoluti

- Tutto il codice in **C** (C11), come il pacchetto k17.
- Nuovi file **solo** in `/home/rufio72/git/kobon/k18/esperimenti/isomorfismo/`
  (cartella da creare) e nessuna modifica a nessun altro file: i file del
  pacchetto pubblicato (`all_solutions.json`, `sol*/`, `c3_symmetric/`,
  `README.md`, `SHA256SUMS.txt`, gli script Python) e tutto `k17/` si leggono
  soltanto. In `/home/rufio72/git/kobon/census` lavora in parallelo un altro
  agente: **non toccare nulla fuori da `k18`**, nemmeno in lettura se non serve.
- Commenti, messaggi e documenti in italiano (i file copiati da k17 restano
  come sono, in inglese).
- Nessun file di codice sopra le 800 righe.
- Dipendenze: solo libc e GMP. GMP è installato (libgmp-dev 6.3.0, header in
  `/usr/include/x86_64-linux-gnu/gmp.h`, si linka con `-lgmp`); `cc` è GCC 13.3.

## Cosa esiste già (fatti verificati)

### Il motore k17 è già generale: non serve riprogettare la forma canonica

Punto delicato del compito, già risolto dai fatti. La forma canonica di
`k17/canon.c` **non** è specifica per n=17 né per arrangiamenti semplici:

- `GEOM_MAXK` è 64 (`k17/geom.h` riga 13, da grep); tutte le strutture usano
  `ev->k`, mai 17. L'unico codice legato a 17 è nei driver (`classify.c`,
  `compare_bader.c`), non in `canon.c`/`geom.c`.
- La struttura eventi `events_t` (`k17/geom.h` righe 19-27) rappresenta ogni
  retta come sequenza di **eventi**, dove un evento è l'insieme (≥1 retta)
  incontrato in un punto: i **punti tripli sono già codificati come gruppi**
  nell'ordine di attraversamento, e le **coppie parallele come incroci
  mancanti** (la retta non compare nella sequenza dell'altra).
- `canonical_form` (`k17/canon.c` righe 159-198, letto per intero) enumera per
  ogni scelta di retta di partenza e verso i pareggi (rette per un punto
  comune) e il gruppo delle parallele in modo esaustivo; il commento in testa
  al file lo dichiara e il codice lo fa (righe 170-195). Forme uguali ⇔
  arrangiamenti isomorfi (dichiarazione in `k17/canon.h`).

Quindi: **binario unico** per semplici e degeneri, riusando i moduli k17 senza
modifiche. La riflessione del piano (x → −x) inverte tutti gli ordini di
attraversamento, cioè è un caso di inversioni per retta: già coperta (il
self-test di `k17/classify.c` righe 92-128 la esercita).

### File k17 da copiare tali e quali (righe da `wc -l`)

| file | righe | ruolo |
|---|---|---|
| `k17/geom.c` / `geom.h` | 259 / 66 | rette razionali esatte (GMP), `geom_events`, conteggio triangoli |
| `k17/canon.c` / `canon.h` | 236 / 24 | forma canonica completa, `canon_cmp`, `canon_json`, SHA-256 |
| `k17/json_min.c` / `json_min.h` | 421 / 34 | parser JSON minimale (DOM) |
| `k17/sha256.c` / `sha256.h` | 113 | SHA-256 (serve a `canon.c`) |

Firme utili (da `k17/geom.h` e `k17/canon.h`, letti):

```c
int  zline_from_strings(zline *l, const char *s[3]);   /* 0 = successo */
void geom_events(const zline *lines, int k, events_t *ev,
                 tuple_list *par, tuple_list *multi);   /* par/multi anche NULL */
int  geom_count_by_adjacency(const events_t *ev);
void canonical_form(const events_t *ev, events_t *form);
int  canon_cmp(const events_t *x, const events_t *y);
char *canon_json(const events_t *form);                 /* malloc'd */
void canonical_sha256(const events_t *form, char out[65]);
```

`k17/classify.c` (343 righe, letto per intero) è il modello del driver:
caricamento `lines_frac` (stringhe razionali "p/q"), self-test geometrico
(permutazione casuale + cambi di segno + x→−x, PRNG xorshift64 deterministico,
5 prove), raggruppamento per SHA della forma, confronto con le classi
dichiarate. Il Makefile k17 usa `CFLAGS = -O2 -Wall -Wextra -std=c11`,
`LDLIBS = -lgmp`.

### I dati k18 (tutti i fatti da scansione diretta dei JSON)

| sorgente | n | formato |
|---|---|---|
| `k18/all_solutions.json`, campo `solutions` | 2337 | chiavi per voce: `id` (numero intero 1..2337), `lines_frac` (18 triple di stringhe razionali), `triple_points`, `parallel_pairs` (liste di tuple di indici, es. `[[2,6,7],[2,10,13]]`), `arm`, `seed`. 2070 con entrambe le liste vuote (semplici), 267 degeneri (138 con coppie parallele) |
| `k18/sol1..sol20/lines_rational.json` | 20 | chiavi: `k`, `count`, `lines_frac` (18 triple), **`declared_triple_points`**, **`declared_parallel_pairs`** (nomi diversi da all_solutions!), `structure`, `provenance`, `lines_float`. Tutti e 20 degeneri |
| `k18/c3_symmetric/arrangements.json`, campo `solutions` | 69 | chiavi: `id` ("c3_00".."c3_68"), `events` (18 righe; ogni evento è una lista di indici, gruppi di 2 ai punti tripli, righe da 15-17 eventi), `triple_points` e `parallel_pairs` (**conteggi interi**, non liste), `straightened` (bool), `triangles`, `central_invariant_triangle`, `seed`, `source_campaign`. 48 con `straightened: true` hanno anche `lines_frac`; 21 hanno solo `events`. 13 degeneri |
| `k18/c3_symmetric_93_lines.json` | 1 | chiavi: `lines_frac` (18 triple), `lines_float`, `count_esatto`, `motore`, `restart`. Il 70° C3 (trovato dal SAT), semplice |

Totali: semplici 2070, degeneri 267+20 = 287, C3 70, complessivi **2427**.
Nessun id duplicato in `all_solutions.json`.

### Riferimento k17 per la coerenza

`k17/k17_solutions.json`: campo `arrangements` (21 voci: `id` stringa
"k17_01".."k17_21", `lines_frac`, `triple_points`, `parallel_pairs`,
`canonical_sha256`), campi `triangles` (85), `class_count` (8), `classes`
(membri dichiarati). Classi dichiarate (dal file):

```
1: k17_01 k17_13 k17_16 k17_17 k17_19
2: k17_02
3: k17_03 k17_07
4: k17_04 k17_12 k17_20
5: k17_05 k17_09 k17_10 k17_14 k17_15
6: k17_06 k17_11 k17_21
7: k17_08
8: k17_18
```

I binari k17 non sono compilati nel repo: queste attese vengono dai dati
dichiarati (verificati in-house dal pacchetto k17), non da un'esecuzione.

Git: radice del repo è `/home/rufio72/git/kobon`, un solo commit (`09b8c03`).

## Struttura da realizzare

Tutto in `/home/rufio72/git/kobon/k18/esperimenti/isomorfismo/`:

1. **Copie identiche** da `k17/`: `geom.c`, `geom.h`, `canon.c`, `canon.h`,
   `json_min.c`, `json_min.h`, `sha256.c`, `sha256.h` (con `cp`, nessuna
   modifica).
2. **`isocheck.c`** (nuovo, stimato 400-600 righe, commenti in italiano), con:
   - caricamento delle quattro sorgenti k18 (percorsi relativi `../../...`
     di default, radice k18 passabile come argomento). Id esterni:
     `a<id>` per all_solutions (es. `a1`), `sol1`..`sol20`, `c3_00`..`c3_68`,
     `c3_sat` per il file alla radice;
   - per le voci con `lines_frac`: `zline_from_strings` → `geom_events` →
     controllo triangoli `geom_count_by_adjacency == 93` e controllo che
     punti multipli/coppie parallele calcolati coincidano con i dichiarati
     (attenti ai due nomi di chiave; per i c3 il dichiarato è un conteggio:
     confrontare le lunghezze delle liste calcolate, dove i punti tripli
     calcolati da `geom_events` sono tuple e le parallele coppie);
   - per le voci solo-eventi (21 c3): costruzione diretta di `events_t`
     dal campo `events` (gruppi ordinati in modo crescente); triangoli con
     `geom_count_by_adjacency == 93`; coerenza degenerazioni: numero di
     eventi con gruppo ≥2 uguale a 3 × `triple_points` dichiarato, numero di
     coppie (i,j), i<j, con j assente dalla riga i (e viceversa, simmetrico)
     uguale a `parallel_pairs` dichiarato;
   - per le 48 voci c3 con entrambe le rappresentazioni: forma canonica da
     `lines_frac` uguale a forma canonica da `events` (confronto con
     `canon_cmp`);
   - forma canonica di ogni arrangiamento, raggruppamento per
     `canonical_sha256` (o per stringa `canon_json`, a scelta), conteggi
     distinti su (a) semplici, (b) degeneri, (c) tutti; per ogni gruppo con
     ≥2 membri una riga `DUP: <id1> <id2> ...`;
   - **`--self-test`**: (i) test combinatorio su **tutti** i 2427: permutazione
     casuale delle etichette + inversione di un sottoinsieme casuale di righe
     applicate alla `events_t` (rietichetta i membri dei gruppi, riordina
     ogni gruppo, inverte l'ordine degli eventi delle righe scelte), forma
     canonica invariata (`canon_cmp == 0`); (ii) test geometrico come
     `self_test` di `classify.c` (permutazione + segni + x→−x) su un
     campione: i 20 `sol*`, 40 voci di all_solutions di cui almeno 20
     degeneri, 5 c3 straightened di cui 2 degeneri, `c3_sat`. PRNG
     deterministico (xorshift64 come in classify.c);
   - **`--k17 <file>`**: modalità di coerenza su `k17_solutions.json`
     (campo `arrangements`, 85 triangoli attesi dal campo `triangles`):
     stesse pipeline, in più controllo che `canonical_sha256` calcolato
     coincida col dichiarato per tutte le 21 voci, e stampa delle classi
     nel formato atteso sotto.
3. **`Makefile`**: come quello k17 (`-O2 -Wall -Wextra -std=c11`, `-lgmp`),
   target `isocheck` e `check` (esegue le tre verifiche 2-4 qui sotto).

Formato di uscita prescritto (una riga per voce solo se FAIL, poi il
riepilogo):

```
SEMPLICI: 2070 arrangiamenti, 2070 forme distinte
DEGENERI: 287 arrangiamenti, 287 forme distinte
C3: 70 arrangiamenti, 70 forme distinte
TOTALE: 2427 arrangiamenti, 2427 forme distinte — PASS
```

(in modalità `--k17`: `classe N: <membri>` una per classe, poi
`CLASSI: 8 distinte su 21 arrangiamenti — PASS`; in `--self-test`:
`SELF-TEST eventi: invariante su 2427/2427 — PASS` e
`SELF-TEST geometria: invariante su 66/66 — PASS`). `PASS` solo se tutti i
controlli (triangoli, degenerazioni dichiarate, coerenza linee/eventi,
assenza di duplicati) passano; exit status 0 solo su PASS.

## Verifiche obbligatorie

Dalla cartella `/home/rufio72/git/kobon/k18/esperimenti/isomorfismo/`:

1. `make` — compila senza errori né warning.
2. `./isocheck --k17 /home/rufio72/git/kobon/k17/k17_solutions.json` — le 8
   classi esattamente come la tabella sopra (stessi membri, stesso ordine) e
   riga finale `CLASSI: 8 distinte su 21 arrangiamenti — PASS`; include il
   confronto degli SHA dichiarati. [attesa dai dati dichiarati k17, non
   misurata con un binario]
3. `./isocheck --self-test` — entrambe le righe SELF-TEST in PASS, incluso
   almeno un caso degenere in ciascun test (garantito dal campione
   prescritto). [attesa, non misurata]
4. `./isocheck` — le quattro righe di riepilogo esattamente come il formato
   prescritto, nessuna riga `DUP`, exit 0. [attesa, non misurata: è proprio
   l'oggetto della verifica; se compaiono `DUP` o conteggi diversi, NON è un
   bug da nascondere: riportare le coppie nel rapporto]
5. `find /home/rufio72/git/kobon/k18/esperimenti/isomorfismo -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale"'`
   — uscita vuota.
6. `git -C /home/rufio72/git/kobon status --short k18` — solo righe `??`
   o `A` sotto `k18/esperimenti/isomorfismo/` e il brief `k18/compiti/02-*`;
   nessun file del pacchetto modificato.
7. `diff` di ogni file copiato con l'originale k17 (es.
   `for f in geom.c geom.h canon.c canon.h json_min.c json_min.h sha256.c sha256.h; do diff -q $f /home/rufio72/git/kobon/k17/$f; done`)
   — nessuna uscita.

Tempo atteso della corsa completa: minuti, non ore (2427 × forma canonica con
~36 valutazioni l'una su k=18; la parte GMP domina). Se supera i 15 minuti
qualcosa non va.

## Consegna

- File nuovi in `esperimenti/isomorfismo/` (copie k17 + `isocheck.c` +
  `Makefile`). Nessun commit: lo fa la chat principale (un altro agente lavora
  nello stesso repo in `census/`).
- Rapporto finale: massimo trenta righe con l'uscita delle verifiche 1-7,
  i tre conteggi (a)(b)(c), le eventuali righe `DUP` integrali, e le
  deviazioni dal brief.

## Domande aperte

1. «preferenza» — Binario unico per semplici e degeneri riusando `canon.c`
   k17 senza modifiche (proposta), invece del doppio binario: il motore k17
   codifica già i punti tripli come gruppi negli eventi e le parallele come
   incroci mancanti, in modo invariante (fatti verificati sopra). Il doppio
   binario non ha ragione d'essere.
2. «preferenza» — I 21 C3 non raddrizzati si trattano dal campo `events`
   (pseudolinee): la forma canonica è puramente combinatoria, quindi il
   confronto con gli arrangiamenti da `lines_frac` è ben definito. Le 48 voci
   con entrambe le rappresentazioni fanno da ponte di controllo
   (verifica di coerenza linee/eventi prescritta sopra).
3. «preferenza» — Raggruppamento per stringa `canon_json` o per
   `canonical_sha256`: indifferente per la correttezza (lo SHA serve comunque
   per la modalità `--k17`); proposta: SHA, come in `classify.c`.

## Decisioni della chat principale (2026-10-23)

Domande 1-3: preferenze accettate come da proposta (binario unico riusando canon.c del k17; i 21 C3 non raddrizzati dal campo events con le 48 voci doppie come ponte di controllo; raggruppamento per SHA-256 della forma canonica).

## Esito (2026-10-23)

Ciclo passato: sviluppatore e verificatore concordano. Risultato: 2426 forme canoniche distinte su 2427 (2070 semplici tutte distinte, 287 degeneri tutte distinte, C3 69 su 70: c3_sat isomorfo a c3_03). La riga FAIL del riepilogo è l'esito scientifico, non un difetto dello strumento: entrambi i self-test di invarianza passano (eventi 2427/2427, geometria 66/66). README di kobon/k18 corretto (69 nuovi tipi C3, nota sul duplicato) e SHA256SUMS rigenerato; il repo congelato kobon_triangles_k18 resta intonso. Token: analista (Fable) 61k, sviluppatore (Opus) 53k, verificatore (Sonnet) 19k.
