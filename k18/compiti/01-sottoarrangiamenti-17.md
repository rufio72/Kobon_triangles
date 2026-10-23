# Compito 01: conteggio dei triangoli nei sotto-arrangiamenti a 17 rette

Data: 2026-10-23. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Per ciascuno dei 2357 arrangiamenti a 18 rette con 93 triangoli (2337 in
`all_solutions.json`, 20 in `sol1..sol20/lines_rational.json`) e per ciascuna
delle 18 rette, contare in aritmetica razionale esatta i triangoli del
sotto-arrangiamento a 17 rette ottenuto togliendo quella retta. Produrre un
file di risultati e un riepilogo stampato.

## Vincoli assoluti

- Non modificare nessun file esistente del repository (non è un repo git
  utilizzabile: `git status` non serve). Nuovi file solo in `esperimenti/`
  (script e risultati) e `compiti/` (solo questo brief).
- Solo libreria standard di Python 3.12 (`fractions`, `itertools`, `json`,
  `argparse`, `multiprocessing`). Niente numpy, niente float.
- Commenti, messaggi e documentazione in italiano.
- Nessun file di codice sopra le 800 righe (lo script atteso è ~150).
- Niente sudo, niente installazioni.

## Cosa esiste già (fatti verificati)

| File | Contenuto |
|---|---|
| `verify_direct_exact.py` (82 righe, letto per intero) | Conteggio diretto; da riusare. |
| `all_solutions.json` | dict con chiavi `k`(18), `triangles`(93), `note`, `count`(2337), `simple`(2070), `with_degeneracies`(267), `solutions`. |
| `sol1..sol20/lines_rational.json` | 20 arrangiamenti con degenerazioni, ognuno con cartella e figure. |

Formato di una voce di `all_solutions.json["solutions"]` (misurato): chiavi
`id` (stringa, "1".."2337"), `lines_frac` (18 terne di stringhe `"p/q"`
= a, b, c con a·x+b·y=c), `triple_points` (lista di terne di indici di rette),
`parallel_pairs` (lista di coppie di indici), `seed`, `arm`. Distribuzione
(len triple_points, len parallel_pairs) sui 2337: (0,0):2070, (1,0):125,
(0,1):70, (1,1):34, (0,2):24, (0,3):10, (2,0):4. Totale 267 con degenerazioni.

Formato di `solN/lines_rational.json` (misurato): chiavi `k`, `count`(93),
`lines_frac` (come sopra), `lines_float` (da ignorare), `declared_triple_points`,
`declared_parallel_pairs`, `structure`, `provenance`. Sui 20 file:
(len triple, len parallel): (2,0):15, (3,0):3, (2,1):1, (4,0):1. Il caso con
parallele è `sol*` con `[[2, 17]]`. Nei file `sol*` i nomi dei campi hanno il
prefisso `declared_`; in `all_solutions.json` no.

Gli indici delle rette sono 0-based, da 0 a 17 (misurato: min 0, max 17 su
tutti i triple_points di all_solutions.json). Le 2337 `lines_frac` sono tutte
distinte. I 20 `sol*` non sono tra le 2337 (README: 2357 non isomorfi).

Logica di conteggio di `verify_direct_exact.py` (da riga 26 a 78):
- righe 30-31: `lines = [tuple(Fraction(x) for x in ln) ...]`;
- righe 42-51: per ogni coppia (i<j) il determinante `det = a1*b2 - a2*b1`;
  se 0 la coppia è parallela e non ha punto, altrimenti
  `P[(i,j)] = ((c1*b2 - c2*b1)/det, (a1*c2 - a2*c1)/det)`;
- righe 57-75: per ogni terna i<j<l con le tre coppie presenti in `P`, il
  triangolo è scartato se l'area è nulla (terna concorrente, riga 62);
  altrimenti conta se nessuna altra retta m separa strettamente due vertici
  (valori `a*x+b*y-c` di segno opposto, con `<0<` stretto; una retta che passa
  per un vertice tocca soltanto e il triangolo conta).
Questa logica gestisce già i casi degeneri (parallele: terna saltata perché
manca un punto; terne concorrenti: area nulla; più rette per un vertice:
nessuna separazione stretta). Non va cambiata la semantica, solo ristretta a
un sottoinsieme di indici.

Tempi misurati: `verify_direct_exact.py` su sol1 = 0,09 s per 18 rette.
Prototipo equivalente (18 conteggi con 17 rette per arrangiamento, più il
conteggio a 18) su sol1..sol20: 21 s su un core; stima per 2357 arrangiamenti
circa 40 minuti su un core, circa 4-5 minuti con 12 processi (`nproc` = 12).

## Struttura da realizzare

Un solo file `esperimenti/sotto17.py` (eseguibile, shebang python3).

Funzioni:
- `carica_tutti() -> list[dict]`: restituisce 2357 elementi
  `{"nome": str, "rette": list[tuple[Fraction,Fraction,Fraction]],
  "triple": list[list[int]], "parallele": list[list[int]]}`. Ordine: prima le
  2337 di `all_solutions.json` con nome `"all_<id>"`, poi `sol1..sol20` con
  nome `"sol<N>"` (ordine numerico). I percorsi si risolvono rispetto alla
  cartella dello script (`Path(__file__).resolve().parent.parent`).
- `conta_triangoli(rette, escluso=None) -> int`: logica di
  `verify_direct_exact.py` su tutti gli indici tranne `escluso` (gli indici
  restano quelli originali 0..17, così triple_points e parallel_pairs restano
  confrontabili). Con `escluso=None` conta le 18 rette.
- `lavora(elemento) -> dict`: calcola `tot18 = conta_triangoli(rette)` e i 18
  valori `sotto17[r] = conta_triangoli(rette, r)`. Controlli (assert con
  messaggio in italiano): `tot18 == 93`; ogni `sotto17[r]` in [61, 85]; le
  coppie parallele calcolate dal determinante coincidono con `parallele`
  dichiarate (come riga 52 del verificatore); ogni terna dichiarata è davvero
  concorrente (area nulla). Un assert fallito non salta l'arrangiamento:
  deve registrare l'errore nel risultato (campo `"errore"`) e il riepilogo
  lo elenca, con exit code 1 alla fine. Valori fuori [77, 85] ma dentro
  [61, 85] non sono errori: solo contati nel riepilogo come "sotto 77".
  Restituisce `{"nome", "sotto17": [18 int], "triple": ..., "parallele": ...}`.
- `main()`: argparse con
  - `--da N --a M`: sottointervallo dell'elenco di `carica_tutti()` in
    indici 0-based, `[N, M)` (default tutto);
  - `--pilota`: equivale a elementi `sol1..sol20` (gli ultimi 20 dell'elenco)
    e uscita di default `esperimenti/risultati_pilota.json`;
  - `--processi P` (default `os.cpu_count()`), `multiprocessing.Pool`,
    `imap` con `chunksize` 4, ordine preservato;
  - `--uscita FILE` (default `esperimenti/risultati_sotto17.json`).
  Scrive JSON: `{"k":18,"conteggio_base":93,"intervallo":[N,M],
  "arrangiamenti":[{"nome","sotto17","triple","parallele"} ...]}`
  (indentazione 1). Poi stampa il riepilogo.

Riepilogo stampato (italiano), sugli arrangiamenti elaborati:
1. numero di arrangiamenti e di conteggi (18 per arrangiamento);
2. distribuzione complessiva dei 18·N conteggi (valore: occorrenze, ordinata);
3. distribuzione dei massimi sotto-17 per arrangiamento;
4. elenco dei "17 perfetti": arrangiamenti con almeno un sotto-17 = 85; per
   ciascuno nome, rette tolte con 85, e per ogni retta tolta `r`: se `r`
   compare in qualche terna di `triple` («in triple_points: sì/no») e se
   compare in qualche coppia di `parallele` («in parallel_pairs: sì/no»),
   con le terne/coppie in cui compare;
5. eventuali errori (sezione vuota se non ce ne sono); exit code 0 se nessuno.

Scelta libera: nomi di variabili e formato esatto delle righe del riepilogo.

## Verifiche obbligatorie

Eseguire dalla cartella `/home/rufio72/git/kobon_triangles_k18`.

1. Sintassi: `python3 -m py_compile esperimenti/sotto17.py && echo ok`.
   Atteso: `ok`.
2. Pilota: `python3 esperimenti/sotto17.py --pilota`. Atteso: exit code 0,
   file `esperimenti/risultati_pilota.json` con 20 arrangiamenti, nessun
   errore. Valori misurati con un prototipo equivalente (sotto17 per sol
   1..20, indici retta 0..17):
   - sol1: 77 77 79 77 78 78 77 82 77 78 79 78 77 78 77 78 79 77
   - sol4: 78 77 77 78 77 77 77 77 78 78 78 77 77 77 78 78 77 85
   - sol9: 77 85 77 77 78 77 78 77 77 78 77 77 77 77 78 77 77 78
   - sol11: 77 77 78 77 78 77 77 77 78 77 77 78 77 85 78 77 77 77
   - sol20: 83 77 77 77 77 77 78 77 77 77 77 77 79 78 78 77 77 79
   Massimi per sol1..sol20: 82 82 84 85 81 84 82 81 85 81 85 84 80 81 82 82
   80 81 82 83.
   Distribuzione complessiva sui 360 conteggi del pilota: 77:216 78:88
   79:33 80:5 81:5 82:6 83:1 84:3 85:3.
   Elenco "17 perfetti" nel pilota: sol4 (retta 17; triple dichiarate
   [[3,8,17],[9,15,17],[10,14,17]]: in triple_points sì, in parallel_pairs
   no), sol9 (retta 1; triple [[1,4,14],[1,9,17]]: sì, no), sol11 (retta 13;
   triple [[2,8,13],[11,13,14]]: sì, no).
3. Coerenza con il verificatore esistente: per `sol4` e `sol1`,
   `python3 verify_direct_exact.py sol4/lines_rational.json` stampa come
   ultima riga `DIRECT EXACT COUNT: 93 — PASS` (attesa, uscita misurata solo
   per sol1: `DIRECT EXACT COUNT: 93 — PASS`); inoltre `tot18` nel risultato
   deve essere 93 per tutti.
4. Prova di intervallo: `python3 esperimenti/sotto17.py --da 0 --a 10
   --uscita /tmp/p10.json` produce 10 arrangiamenti; `--da 0 --a 5` seguito
   da `--da 5 --a 10` (uscite diverse) dà, concatenati, gli stessi `sotto17`
   (confrontare con `python3 -I -c` e json). Attesa: uguali. Uscite
   temporanee fuori dal repository.
5. Test della sanità: una chiamata diretta a `conta_triangoli` con le rette
   di sol1 e `escluso=None` restituisce 93. Attesa: 93.
6. Lancio completo: `python3 esperimenti/sotto17.py`. Atteso: exit code 0,
   `esperimenti/risultati_sotto17.json` con 2357 arrangiamenti, nessun
   errore, tempo orientativo 4-10 minuti (stima, non misurata sull'insieme
   completo). Nel riepilogo, gli ultimi 20 arrangiamenti devono dare
   gli stessi valori del pilota (confronto con json).
   Nota: esito atteso su all_solutions.json non misurato; i contatori del
   riepilogo non vanno confrontati con valori a priori, solo con i vincoli
   [61, 85] (nessun errore) e 3 «17 perfetti» presenti almeno tra sol*.
7. Ambito: `find . -newer verify_direct_exact.py -type f -not -path
   './esperimenti/*' -not -path './compiti/*' -not -path './.git/*'`
   deve essere vuoto, e `SHA256SUMS.txt` non modificato
   (`sha256sum -c SHA256SUMS.txt 2>&1 | grep -vc ': OK$'` non deve
   crescere rispetto a prima del lavoro: eseguirlo anche all'inizio e
   confrontare).
8. Dimensione: `wc -l esperimenti/*.py | awk '$1>800 && $2!="total"'`.
   Atteso: vuoto.

## Consegna

- Script `esperimenti/sotto17.py`; risultati `esperimenti/risultati_pilota.json`
  e `esperimenti/risultati_sotto17.json`; salvare anche l'uscita completa del
  riepilogo del lancio completo in `esperimenti/riepilogo_sotto17.txt`
  (`... | tee`).
- Nessun commit richiesto (la cartella è un repository pubblicato: non
  committare né fare push). Se la chat principale chiede un commit, la riga è
  `Co-Authored-By: Claude Opus <noreply@anthropic.com>`.
- Rapporto finale in massimo trenta righe: uscita delle verifiche 1-8,
  distribuzione complessiva, elenco dei «17 perfetti» con l'esito
  triple/parallele, deviazioni.

## Domande aperte

1. Perimetro dei «17 perfetti»: considerare solo valori esattamente 85
   (massimo teorico per 17 rette: ⌊17·15/3⌋ = 85). Proposta: sì, solo 85.
   Marca: preferenza.
2. Dedup tra i 2357: i 20 `sol*` non sono tra le 2337 secondo il README;
   lo script non lo verifica (lo fa già il pacchetto). Proposta: nessun
   controllo di non-isomorfismo, solo che le `lines_frac` siano distinte
   (assert in `carica_tutti`). Marca: preferenza.
3. Se qualche conteggio fuori [61, 85] compare davvero, è un bug o un
   risultato interessante? Proposta: l'agente non lo "corregge", lo riporta
   nel rapporto con nome e retta e non cambia l'intervallo. Marca: preferenza.
4. Una retta tolta che sia in una terna: il sotto-17 la tratta come
   incrocio semplice delle altre due (conseguenza automatica della logica
   riusata). Proposta: nessuna gestione speciale. Marca: preferenza.

## Esito (2026-10-23)

Ciclo passato al primo colpo. Sviluppatore: 8/8 verifiche passate, lancio completo 8m05s, 0 errori. Verificatore indipendente: 7/8 passate, la 7 ambigua per un disallineamento preesistente di README.md in SHA256SUMS.txt (file non toccato dal lavoro, differenza presente anche prima). Risultato: 21 arrangiamenti su 2357 con un sotto-17 perfetto (85 triangoli); 5 tramite retta di un punto triplo, 16 in arrangiamenti semplici. Token: analista (Sonnet) 30k, sviluppatore (Opus) 31k, verificatore (Sonnet) 18k.
