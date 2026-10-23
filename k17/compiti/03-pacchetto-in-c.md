# Compito 03: tutti gli strumenti del pacchetto in C

Data: 2026-10-23. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Sostituire i quattro script Python del pacchetto (`verify_direct_exact.py`,
`classify.py`, `compare_bader.py`, `build/estrai.py`, commit 2e48049) con
equivalenti in C a parità esatta di risultati. I Python restano nell'albero
come oracolo durante lo sviluppo e si tolgono con `git rm` alla fine.
Aggiornare `README.md` e rigenerare `SHA256SUMS.txt`.

Vincoli assoluti:
- Nessuna modifica fuori da `/home/rufio72/git/kobon_triangles_k17`. Il
  repository `/home/rufio72/git/kobon_triangles_k18` si legge soltanto.
- Aritmetica esatta, nessun uso di virgola mobile nei predicati geometrici.
  Dipendenze massime: GMP e libc. SHA-256 va implementato nel pacchetto
  (né OpenSSL né altro). Parser JSON minimale ad hoc nel pacchetto.
- `Makefile` con `-Wall -Wextra`; la build non deve produrre warning.
- Nessun file di codice sopra le 800 righe (vedi moduli sotto).
- I testi stampati dagli eseguibili devono essere identici byte per byte a
  quelli dei Python (sono in Verifiche); i Python non si modificano.
- Commenti di `build/estrai.c` in italiano (come `estrai.py`); per i file
  pubblicati vedi domanda aperta 1. Nessun commit (lo fa la chat principale).
- Non modificare `k17_solutions.json`, `bader17_table.json`, `LICENSE`.

## Cosa esiste già (fatti verificati)

Toolchain (da comandi di sola lettura): gcc 13.3.0; `libgmp-dev`
2:6.3.0+dfsg-2ubuntu6.1 installato (`/usr/include/x86_64-linux-gnu/gmp.h`,
`-lgmp`).

| File | Righe | Ruolo |
|---|---|---|
| `verify_direct_exact.py` | 117 | verifica 1: definizione classica, 21/21 PASS |
| `classify.py` | 210 | verifica 2: eventi, adiacenza, forma canonica, 8 classi |
| `compare_bader.py` | 129 | tavola di Bader → classe 5 (importa `classify`) |
| `build/estrai.py` | 127 | estrae i 21 dal k18, scrive `k17_solutions.json` |
| `k17_solutions.json` | 94883 byte | sha256 `7f8742df58722353396b3aea606b8c60c49b382c3bb90e450fcba9cb5997da25` |
| `README.md` | 183 | da aggiornare (righe 44-46, 110-145, 128) |

Tutti e quattro i Python sono stati letti per intero: l'algoritmo da
riprodurre è il loro, riga per riga. Punti esatti:

- `verify_direct_exact.py::check` (r. 31-78): coincidenza di rette (tre
  prodotti incrociati nulli), coppie parallele vs dichiarate, per ogni terna
  non parallela: area nulla → scarta; vuota se nessuna altra retta separa
  strettamente due vertici (segni opposti di `a*x+b*y-c`). Conta anche i
  triangoli per retta (attesi 15). Opzioni: `[file]` (default
  `k17_solutions.json`), `--id`.
- `classify.py::events` (r. 39-70): per ogni retta i, incroci ordinati per
  `s = -b1*x + a1*y`; incroci con s uguale stanno nello stesso evento
  (frozenset); raccoglie coppie parallele (ordinate) e punti multipli
  (tuple ordinate di ≥3 rette, lista ordinata, dedotti dagli eventi con
  ≥2 elementi uniti a {i}).
- `classify.py::count_by_adjacency` (r. 73-91): terna conta se ogni coppia
  si incrocia, i tre incroci non sono lo stesso evento su x, e su ciascuna
  delle tre rette i due eventi rilevanti sono consecutivi (|Δpos| = 1).
- `classify.py::canonical_form` (r. 94-125): per ogni retta x e per le due
  direzioni di `ev[x]`, i gruppi sono gli eventi (liste ordinate) più il
  gruppo delle parallele a x; prodotto cartesiano delle permutazioni di ogni
  gruppo; etichette assegnate nell'ordine; per ogni retta (ordinata per
  etichetta) la sequenza rietichettata presa nella direzione minore
  (`min(s, s[::-1])`, confronto lessicografico di tuple: prefisso più corto
  è minore); minimo globale. Nei dati del pacchetto tutti i gruppi sono
  singoletti (nessun parallelismo, nessun punto triplo), ma va implementato
  il caso generale.
- `classify.py::canonical_sha256` (r. 128-131): SHA-256 esadecimale
  minuscolo del JSON compatto della forma: array annidati di interi senza
  spazi, es. `[[[3],[5],...],...]` (`json.dumps(form, separators=(",",":"))`).
- `classify.py::main` (r. 158-206): per ogni arrangiamento confronta
  conteggio (=85), `parallel_pairs`, `triple_points`, `canonical_sha256`
  con i dichiarati; raggruppa per sha, ordina i gruppi per primo membro
  (confronto di stringhe degli id), confronta con `classes` dichiarate
  (sha e membri) e con `class_count`. Con `--self-test` ricalcola la forma
  dopo 5 prove per arrangiamento: permutazione casuale delle etichette,
  cambio di segno casuale di ogni retta, riflessione `x -> -x`
  (`(-a, b, c)`), seme fisso.
- `compare_bader.py` (intero): valida la tavola (17 righe, ognuna
  permutazione di 1..17 senza i+1, errori → `ERROR: <msg>` ed exit 1),
  eventi a singoletti 0-based, conteggio per adiacenza, forma canonica,
  ricerca dello sha tra le `classes` di `k17_solutions.json`. Con
  `--self-test`: 5 prove con rietichettatura casuale e inversione casuale
  di righe. Opzioni posizionali: `[tavola] [soluzioni]`.
- `build/estrai.py` (intero): legge dal k18
  `esperimenti/risultati_sotto17.json` (552 KB: `arrangiamenti[]` con
  `nome`, `sotto17` di 18 interi), `all_solutions.json` (6,0 MB:
  `solutions[]` con `id`, `lines_frac`, `triple_points`, `parallel_pairs`)
  e `sol1..sol20/lines_rational.json` (`lines_frac`,
  `declared_triple_points`, `declared_parallel_pairs`); 2357 fonti attese.
  Prende le coppie (nome, retta) con `sotto17 == 85` nell'ordine del file
  (attese 21), toglie la retta, ricalcola eventi/conteggio/forma, esige
  zero degenerazioni, numera le classi per prima occorrenza e scrive il
  JSON con `indent=1` più newline finale. Le stringhe `lines_frac` si
  copiano identiche dalla fonte (il parser deve conservare il testo grezzo).
  Ordine esatto delle chiavi: quello del sorgente Python (righe 87-96 e
  104-117). Riga finale: `scritto <basename>: 21 arrangiamenti, 8 classi`.
  Opzioni: `--k18` (default `/home/rufio72/git/kobon_triangles_k18`),
  `--uscita` (default `k17_solutions.json` nella radice del pacchetto).
  Le assert diventano controlli con messaggio su stderr ed exit 1.

Formati (da ispezione dei file):
- `lines_frac`: stringhe `"p/q"` (eventuale segno su p); denominatori fino
  a 50 cifre decimali → servono mpz/mpq. Accettare anche `"p"` senza `/q`.
- `k17_solutions.json` attuale: nessun float con esponente, nessun float
  intero (niente `.0` secco da regola speciale nei dati odierni), nessun
  carattere non ASCII. 8 classi di dimensioni 5,1,2,3,5,3,1,1.
- `bader17_table.json`: chiavi `n`, `triangles`, `author`, `source`,
  `format`, `table` (17 righe da 16 interi 1-based).

Matematica esatta senza virgola mobile (scelta prescritta):
- Ogni retta `(a,b,c)` razionale si moltiplica per il mcm positivo dei tre
  denominatori → terna di `mpz_t`. Fattore positivo: conserva coincidenze,
  parallelismi, segni dei lati e ordine lungo la retta.
- Parallelismo: `D = a1*b2 - a2*b1 == 0`. Intersezione omogenea intera:
  `X = c1*b2 - c2*b1`, `Y = a1*c2 - a2*c1`, `W = D`.
- Area nulla (terna concorrente): determinante 3×3 nullo delle tre righe
  `(X,Y,W)`.
- Segno del lato della retta m nel punto `(X,Y,W)`:
  `sgn(a_m*X + b_m*Y - c_m*W) * sgn(W)`.
- Parametro d'evento lungo la retta i: `s = (-b1*X + a1*Y) / W` come
  `mpq_t` (`mpq_canonicalize`), ordinamento con `mpq_cmp`, uguaglianza
  esatta per i bucket.

Trappole note (verificate sul codice e sui dati):
- `mpq_get_d` TRONCA verso zero; Python `float(Fraction)` arrotonda al più
  vicino. Per `lines_float` serve l'arrotondamento corretto: `d =
  mpq_get_d(v)`; candidato `d2 = nextafter(d, ±HUGE_VAL)` nel verso del
  segno di v; punto medio `(d+d2)/2` ricostruito esatto con `mpq_set_d`
  (esatta sui double) e media in mpq; confronto esatto con v: oltre il
  medio → `d2`, sotto → `d`, pari → mantissa pari.
- Stampa dei double identica a `repr` di Python: ciclo `%.1g`…`%.17g`,
  prima precisione che fa il giro con `strtod` == valore; se il risultato
  non contiene `.` né `e`, aggiungere `.0` (es. `2` → `2.0`, `-0` → `-0.0`).
- `json.dump(..., indent=1)`: a ogni livello newline + un numero di spazi
  pari alla profondità; `"chiave": valore`; separatore `,` prima del
  newline; array vuoto → `[]` sulla stessa riga; dopo la radice
  `f.write("\n")`. Escape stringhe: `\"`, `\\`, controlli; i dati sono
  ASCII puro.
- `--self-test`: non riprodurre il Mersenne Twister di Python. Basta un
  PRNG deterministico proprio (es. xorshift64 con seme fisso, shuffle di
  Fisher-Yates), 5 prove: il testo stampato è identico comunque, perché
  riporta solo l'esito.
- Il parser JSON deve essere lineare (carica il file in un buffer unico):
  `all_solutions.json` è 6,0 MB.

## Struttura da realizzare

Moduli condivisi nella radice (nomi liberi solo se indicato):
- `sha256.c` / `sha256.h` (~170 righe): SHA-256 (FIPS 180-4) scritto per il
  pacchetto, API tipo `void sha256_hex(const unsigned char*, size_t, char out[65])`.
- `json_min.c` / `json_min.h` (~300 righe): parser JSON minimale a DOM per
  questo formato fisso: oggetti, array, stringhe, numeri (conservare il
  testo grezzo del numero e il valore `long` se intero), true/false/null.
  Funzioni di accesso per chiave e indice; caricamento file intero.
- `geom.c` / `geom.h` (~350 righe): parse di `"p/q"` in `mpq_t`, riduzione
  della retta a terna `mpz_t`; costruzione eventi (struct con, per retta,
  array di eventi = array ordinati di int), coppie parallele, punti
  multipli; `count_by_adjacency`.
- `canon.c` / `canon.h` (~250 righe): forma canonica (array annidati di
  int), confronto lessicografico, serializzazione compatta +
  `canonical_sha256` esadecimale.

Eseguibili (ognuno un `main` più l'eventuale logica propria):
- `verify_direct_exact.c` (~250 righe): usa json_min + GMP, non geom
  (la definizione diretta è indipendente, come in Python).
- `classify.c` (~250 righe): json_min + geom + canon + sha256.
- `compare_bader.c` (~200 righe): json_min + geom (solo adiacenza) + canon.
- `build/estrai.c` (~350 righe): json_min + geom + canon + scrittore JSON
  `indent=1` e formattazione dei double (le due routine possono stare qui,
  o in un modulo `json_out.c` se si superano le 800 righe).

`Makefile` nella radice: `CC ?= cc`, `CFLAGS = -O2 -Wall -Wextra -std=c11`,
`LDLIBS = -lgmp`; target `all` (i quattro eseguibili, `estrai` in
`build/`), `check` (esegue verify, classify --self-test,
compare_bader --self-test), `clean`. Aggiungere `.gitignore` con `*.o` e i
nomi dei quattro eseguibili (i binari non si committano).

`README.md`: righe 44-46 (`build/estrai.py` → `build/estrai.c`), sezione
Layout (file `.c`/`.h`, `Makefile`), sezione «Verify it yourself»: al posto
di «Python 3, standard library only», requisiti «a C compiler and GMP
(Debian/Ubuntu: `apt install libgmp-dev`)» e comandi `make` seguito dai tre
eseguibili (o `make check`), più `sha256sum -c SHA256SUMS.txt`; aggiornare i
nomi degli script citati nelle righe 113-117 e 147-159. Il resto non cambia.

`SHA256SUMS.txt`: rigenerare con tutti i file pubblicati tranne sé stesso,
in ordine alfabetico, ad esempio:
`sha256sum LICENSE Makefile README.md bader17_table.json build/estrai.c canon.c canon.h classify.c compare_bader.c geom.c geom.h json_min.c json_min.h k17_solutions.json sha256.c sha256.h verify_direct_exact.c > SHA256SUMS.txt`

Ordine di lavoro: prima i C con i Python ancora presenti, confrontando le
uscite con `diff <(./prog ...) <(python3 prog.py ...)`; poi `git rm
verify_direct_exact.py classify.py compare_bader.py build/estrai.py`,
README, SHA256SUMS, verifiche finali.

## Verifiche obbligatorie

Le uscite 2, 4, 5, 6 sono state misurate oggi con i Python (oracolo).

1. `make` — compila senza errori né warning.
2. `./verify_direct_exact` — 21 righe `k17_NN: 85 triangles, 15 per line — PASS`
   (NN da 01 a 21 in ordine) e ultima riga `DIRECT EXACT: 21/21 PASS`;
   exit 0. Identica all'oracolo (`diff` con l'uscita di
   `python3 verify_direct_exact.py` prima della rimozione).
3. `./verify_direct_exact --id k17_05` — attesa, non misurata:
   `k17_05: 85 triangles, 15 per line — PASS` poi `DIRECT EXACT: 1/1 PASS`.
4. `./classify --self-test` — uscita misurata, identica a questa:
   ```
   SELF-TEST: canonical form invariant on 21/21 — PASS
   class 1: 5 arrangements (k17_01 k17_13 k17_16 k17_17 k17_19)
   class 2: 1 arrangement (k17_02)
   class 3: 2 arrangements (k17_03 k17_07)
   class 4: 3 arrangements (k17_04 k17_12 k17_20)
   class 5: 5 arrangements (k17_05 k17_09 k17_10 k17_14 k17_15)
   class 6: 3 arrangements (k17_06 k17_11 k17_21)
   class 7: 1 arrangement (k17_08)
   class 8: 1 arrangement (k17_18)
   CLASSES: 8 distinct, 21 arrangements — PASS
   ```
5. `./compare_bader --self-test` — uscita misurata, identica a questa:
   ```
   table: 17 lines, all pairs cross
   triangles (adjacency): 85
   canonical_sha256: 5834f74309210141eb194a733258c665399527d0ce5c65f6d091f459d43d5a3f
   SELF-TEST: canonical form invariant — PASS
   BADER: class 5 (k17_05 k17_09 k17_10 k17_14 k17_15) — MATCH
   ```
6. `./build/estrai --uscita /tmp/k17_c.json && cmp k17_solutions.json /tmp/k17_c.json && echo IDENTICO`
   — stampa `scritto k17_c.json: 21 arrangiamenti, 8 classi` poi `IDENTICO`
   (misurato: l'oracolo Python rigenera il file byte per byte, sha256
   `7f8742df…`). Se il C differisse solo per la resa dei float, fermarsi e
   riportarlo come deviazione, non dichiarare comunque chiuso.
7. `make check` — esegue 2, 4, 5 e termina con exit 0.
8. `sha256sum -c SHA256SUMS.txt` — tutti `OK` dopo la rigenerazione.
9. `ls *.py build/*.py 2>/dev/null` — vuoto (Python rimossi).
10. `find . -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale"'`
   — vuoto.
11. `git status --short` — solo: `.py` cancellati, `.c`/`.h`/`Makefile`/
   `.gitignore` nuovi, `README.md` e `SHA256SUMS.txt` modificati, nessun
   binario né `.o`.

## Consegna

- File: i moduli e gli eseguibili C, `Makefile`, `.gitignore`, `README.md`
  e `SHA256SUMS.txt` aggiornati; Python rimossi con `git rm`. Nessun
  commit (lo fa la chat principale).
- Rapporto finale: massimo trenta righe, con l'uscita delle verifiche
  1-2, 4-8 e le deviazioni dal brief. Se la verifica 6 non desse
  `IDENTICO`, riportare il `diff` delle prime differenze e fermarsi lì.

## Domande aperte

1. Lingua dei commenti nei file C pubblicati (tutti tranne
   `build/estrai.c`): i Python pubblicati hanno docstring in inglese e il
   pacchetto è in inglese. Proposta: commenti e usage in inglese per i file
   pubblicati, italiano per `build/estrai.c`, come oggi. «Preferenza».
2. Nomi dei moduli condivisi (`geom`, `canon`, `json_min`, `sha256`) e del
   target `check`. Proposta: quelli indicati sopra. «Preferenza».
3. Interfaccia a riga di comando: riprodurre solo le opzioni esistenti
   (file posizionali, `--id`, `--self-test`, `--k18`, `--uscita`), con un
   parsing a mano senza getopt lungo. Proposta: sì, messaggi d'errore
   liberi purché exit ≠ 0. «Preferenza».

## Decisioni della chat principale (2026-10-23)

Domande 1-3: preferenze accettate come da proposta (commenti in inglese nei file pubblicati e italiano in estrai.c; moduli geom/canon/json_min/sha256 e target check; CLI identica con parsing a mano). Nota di contesto: a valle di questo compito l'intero pacchetto verrà spostato in /home/rufio72/git/kobon/k17 (cartella unica decisa dall'utente); nessuna azione richiesta qui, i percorsi relativi interni non devono assumere la posizione assoluta attuale.

## Esito (2026-10-23)

Ciclo passato al primo colpo. Sviluppatore: 11/11 verifiche passate, uscite C identiche byte per byte agli oracoli Python (diff fatto prima della rimozione dei .py), 3 deviazioni documentate (stampa dei decimali con shortest round-trip, campi interi stretti in compare_bader, default di estrai relativo all'eseguibile). Verificatore indipendente: 10/11 passate, l'11 ambigua solo per build/ non tracciata come cartella intera (i binari sono coperti da .gitignore). Token: analista (Fable) 60k, sviluppatore (Opus) 104k, verificatore (Sonnet) 15k.
