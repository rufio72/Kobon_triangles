# Compito 01: Censimento dei tipi di arrangiamenti ottimali di Kobon, n=3..10

Data: 2026-10-23. Scritto dall'agente analista per un agente sviluppatore.
Questo file è tutto il contesto che serve: non c'è una chat da leggere.

## Obiettivo

Sistema in C che, per ogni n da 3 a 10, enumera TUTTI gli arrangiamenti
semplici di n pseudorette che realizzano il massimo noto di triangoli di Kobon
e conta le classi di isomorfismo (rinumerazione delle rette, inversione del
verso, riflessione). Per ogni n dimostra anche UNSAT a massimo+1 (tetto a
livello pseudorette). Risultati finali nelle cartelle `k3`..`k10` del repo.
Architettura già pronta per n=11 e 12 (fase successiva).

### Vincoli assoluti

- Tutto il codice in C (C11). Ammessi solo `Makefile` e al più uno script di
  lancio (shell). Niente Python nel prodotto.
- Codice e file di lavoro in `/home/rufio72/git/kobon/census/`. I risultati
  finali vanno in `/home/rufio72/git/kobon/k3` … `k10` (cartelle da creare).
  Non toccare nulla in `k14/`, `k16/`, `k18/` né altrove nel repo.
- SAT solver: kissat 4.0.4, binario `/home/rufio72/.local/bin/kissat` (è nel
  PATH, verificato con `which kissat`). Solo invocato come processo esterno:
  non ridistribuirlo, non copiarne sorgenti.
- Il modello CNF è quello di Savchuk (arXiv:2507.07951, repo
  https://github.com/zegalur/kobon-cnf). Il repo si può clonare in sola
  lettura in una cartella temporanea per consultazione, ma il C è scritto da
  zero: niente traduzione meccanica né copia del Python nel prodotto.
- Nessun file di codice sopra le 800 righe. `gcc -Wall -Wextra` senza warning.
- Commenti, messaggi e documenti in italiano (eccezione possibile: README
  delle cartelle k{n}, vedi Domande aperte).
- Ambito fase 1: solo arrangiamenti SEMPLICI (niente punti tripli, niente
  parallele), a livello pseudorette. La realizzazione in coordinate razionali
  (come in k14-k18) e le degenerazioni sono estensioni future: i README dei
  risultati devono dichiararlo.

## Cosa esiste già (fatti verificati)

- `census/` è vuota (solo questo brief). Nessun CLAUDE.md di progetto.
  `k14/ k16/ k18/` esistono: schema `kNN/README.md` + `solN/` con
  `lines_rational.json` e figure (da `ls`; quelle cartelle contengono
  arrangiamenti CON punti tripli, non c'entrano con questa fase).
- gcc 13.3.0 (da `gcc --version`). kissat 4.0.4: esce con codice 10 su SAT
  (stampa `s SATISFIABLE` e righe `v <interi> ... 0` con il modello), 20 su
  UNSAT (`s UNSATISFIABLE`); l'opzione `-q` riduce l'output alle sole righe
  `s`/`v` (misurato eseguendo kissat su CNF generati dal repo di riferimento).
- Massimi noti (OEIS A006066) e segmenti "mancanti" m = n(n-2) − 3T
  (ogni retta ha n-2 segmenti finiti; un triangolo vuoto usa 3 segmenti,
  mai condivisi):

  | n  | T max | m  | m' per T+1 (verifica UNSAT)      |
  |----|-------|----|-----------------------------------|
  | 3  | 1     | 0  | <0 → impossibile aritmeticamente |
  | 4  | 2     | 2  | <0 → aritmetico                  |
  | 5  | 5     | 0  | <0 → aritmetico                  |
  | 6  | 7     | 3  | 0 → serve SAT (misurato UNSAT)   |
  | 7  | 11    | 2  | <0 → aritmetico                  |
  | 8  | 15    | 3  | 0 → serve SAT (misurato UNSAT)   |
  | 9  | 21    | 0  | <0 → aritmetico                  |
  | 10 | 25    | 5  | 2 → serve SAT (attesa UNSAT)     |
  | 11 | 32    | 3  | 0 (fase 2)                       |
  | 12 | 38    | 6  | 3 (fase 2)                       |

- Misure fatte con la pipeline di riferimento (gen.py del repo clonato +
  kissat), che fa esattamente ciò che il nostro C deve riprodurre per gli n
  senza segmenti mancanti (m=0):
  - `python3 gen.py 3` → 1 classe in 0,03 s; `gen.py 5` → 1 classe in 0,03 s;
    `gen.py 9` → 1 classe in 0,19 s (ogni esecuzione: cicli SAT con clausole
    bloccanti sull'intera orbita fino a UNSAT).
  - CNF "perfetto" (m=0) per n=6 → `s UNSATISFIABLE` in 0,00 s; per n=8 →
    `s UNSATISFIABLE` in 0,03 s (conferma 8 e 16 triangoli impossibili).
- Misure fatte con enumerazione esaustiva indipendente dei wiring diagram
  (DFS su tutte le parole ridotte, script Python usa-e-getta; ground truth
  per pseudorette, da riprodurre in C):

  | n | parole (foglie DFS) | tabelle distinte | tabelle con T max | classi |
  |---|--------------------:|-----------------:|------------------:|-------:|
  | 3 | 2                   | 2                | 2                 | 1      |
  | 4 | 16                  | 8                | 8                 | 1      |
  | 5 | 768                 | 62               | 2                 | 1      |
  | 6 | 292864              | 908              | 32                | 2      |

- Semantica verificata sulle tabelle trovate: conteggio triangoli per
  "adiacenza su tre righe" (sotto) dà 1, 5, 21 per n=3, 5, 9; la
  ricostruzione greedy del wiring (sotto) riesce con permutazione iniziale
  identità su tutte; orbite canoniche di taglia 2, 2, 6 (arrangiamenti molto
  simmetrici; il massimo teorico è ~4n).
- Forme canoniche misurate (fixture per le verifiche):
  - n=3: `[[2,3],[1,3],[1,2]]`
  - n=5: `[[2,4,3,5],[1,4,5,3],[4,1,5,2],[3,1,2,5],[1,3,2,4]]`

### Rappresentazione: la tabella degli ordini

Un arrangiamento semplice di n pseudorette è rappresentato da una tabella di
n righe: la riga r elenca, nell'ordine di attraversamento lungo la retta r,
le n-1 rette incontrate (numerazione 1..n; le rette sono numerate secondo
l'ordine circolare delle direzioni, come in un wiring diagram ordinato per
pendenza). Fatti verificati sulle tabelle del riferimento:

- Triangoli: la terna {a,b,c} forma un triangolo vuoto ⇔ su ciascuna delle
  tre righe a, b, c le altre due compaiono in posizioni ADIACENTI.
  #triangoli = numero di tali terne.
- Validità (è davvero un arrangiamento di pseudorette): ricostruzione greedy
  del wiring diagram. Permutazione corrente = [1..n] (alto→basso); finché
  restano incroci: cerca una coppia adiacente (u,v) nella permutazione tale
  che il prossimo elemento non consumato della riga u sia v E il prossimo
  della riga v sia u; scambiala e avanza i due cursori. Se dopo n(n-1)/2
  scambi la permutazione è invertita → valida; se nessuna coppia è
  applicabile prima → tabella non realizzabile. Una tabella è valida se la
  ricostruzione riesce su almeno un elemento della sua orbita.

### Gruppo di isomorfismo e forma canonica

Trasformazioni (semantica esatta, trascritta dal riferimento
`koboncnf/koboncnf.py`, funzioni `reindex_table` e `reverse_order`;
indici 1-based, `t[l]` è la riga della retta l):

```python
def reindex(t, f):            # la retta f diventa la retta 1
    n = len(t)
    res = []
    for l, row in enumerate(t):            # l: 0-based
        new = [(n + x - f) % n + 1 for x in row]
        if (l + 1) < f:
            new.reverse()
        res.append(new)
    return res[-(n - f + 1):] + res[:(f - 1)]

def inverti(t):               # inversione del verso
    n = len(t)
    idx = lambda l: 1 if l == 1 else (n - l + 2)
    return [[idx(l) for l in (t[0] if r == 0 else t[idx(r+1)-1][::-1])]
            for r in range(n)]
```

Orbita = chiusura BFS della tabella sotto i generatori
{`reindex(·,f)` per f=1..n, `inverti`, `t2 = reindex(reindex(·,n),2)`}
(t2 è la riflessione usata dal riferimento; nelle misure era già nella
chiusura di reindex+inverti, ma va tenuta come generatore per sicurezza:
costa nulla). Forma canonica = minimo lessicografico dell'orbita
(confronto riga per riga, elemento per elemento). Taglia orbita ≤ ~4n.

### Modello CNF (da riprodurre in C, clausole verificate sul riferimento)

Indici 0-based: r, i, j rette distinte in [0,n); k colonna in [0,n-1).
Variabili (numerazione intera deterministica qualsiasi, purché stabile):
- `A(r,i,k)`: la retta i è la k-esima incontrata lungo r.
- `G(r,i,j)`: j viene immediatamente dopo i lungo r (segmento finito (i,j)).
- `X(r,i,j)`: j viene dopo i (non necessariamente subito) lungo r.
- `Miss(r,i,j)` (solo se m>0): il segmento (i,j) di r non è lato di triangolo.

Clausole (∀ sui range validi, letterali negativi con `-`):
1. ∀r,i≠r: ⋁ₖ A(r,i,k).
2. ∀r,i≠r, k≠k': ¬A(r,i,k) ∨ ¬A(r,i,k').
3. ∀r,i,j distinti, k∈[0,n-2): le tre clausole
   ¬A(r,i,k) ∨ ¬A(r,j,k+1) ∨ G(r,i,j);
   ¬A(r,i,k) ∨ ¬G(r,i,j) ∨ A(r,j,k+1);
   ¬G(r,i,j) ∨ ¬A(r,j,k+1) ∨ A(r,i,k).
4. ∀r,i,j≠k distinti: ¬G(r,i,j) ∨ ¬G(r,i,k)  (successore unico).
5. primo: ∀j: ¬A(r,i,0) ∨ ¬G(r,j,i);  e  A(r,i,0) ∨ ⋁ⱼ G(r,j,i).
6. ultimo: ∀j: ¬A(r,i,n-2) ∨ ¬G(r,i,j);  e  A(r,i,n-2) ∨ ⋁ⱼ G(r,i,j).
7. ∀r, i<j (≠r): X(r,i,j) ∨ X(r,j,i)  e  ¬X(r,i,j) ∨ ¬X(r,j,i).
8. transitività: ¬X(r,i,j) ∨ ¬X(r,j,k) ∨ X(r,i,k).
9. ∀j: ¬A(r,i,0) ∨ X(r,i,j);  ¬A(r,i,n-2) ∨ X(r,j,i);
   (⋁ⱼ ¬X(r,i,j)) ∨ A(r,i,0);  (⋁ⱼ ¬X(r,j,i)) ∨ A(r,i,n-2).
10. ¬G(r,i,j) ∨ X(r,i,j).
11. Chiusura dei triangoli. Per ogni terna ordinata (r,i,j), i "compagni"
    C1, C2 di G(r,i,j) dipendono dall'ordine degli indici (tabella esatta,
    trascritta dal riferimento; vale sia per G sia per X):

    | caso    | G(r,i,j) ⇒ C1 ∧ C2  | G(r,j,i) ⇒ C1' ∧ C2' |
    |---------|----------------------|-----------------------|
    | r<i<j   | G(i,r,j), G(j,r,i)   | G(i,j,r), G(j,i,r)    |
    | r<j<i   | G(j,i,r), G(i,j,r)   | G(j,r,i), G(i,r,j)    |
    | i<r<j   | G(i,r,j), G(j,i,r)   | G(i,j,r), G(j,r,i)    |
    | j<r<i   | G(j,i,r), G(i,r,j)   | G(j,r,i), G(i,j,r)    |
    | i<j<r   | G(i,j,r), G(j,i,r)   | G(i,r,j), G(j,r,i)    |
    | j<i<r   | G(j,r,i), G(i,r,j)   | G(j,i,r), G(i,j,r)    |

    Per G (condizione di Kobon, con via d'uscita Miss; se m=0 ometti i
    letterali Miss): per ogni direzione (a,b) della coppia:
    ¬G(r,a,b) ∨ C1 ∨ Miss(r,a,b);  ¬G(r,a,b) ∨ C2 ∨ Miss(r,a,b);
    G(r,a,b) ∨ ¬C1 ∨ ¬C2 ∨ Miss(r,a,b).
    Per X (coerenza delle terne, SENZA Miss): stesse tre clausole con X al
    posto di G e senza i letterali Miss.
12. Se m>0: ∀: ¬Miss(r,i,j) ∨ G(r,i,j); più cardinalità "al più m veri" su
    tutte le Miss con il contatore sequenziale di Sinz (variabili ausiliarie
    s(v,c), v=1..V, c=1..m; clausole standard: ¬x₁∨s(1,1); ¬s(v-1,c)∨s(v,c);
    ¬xᵥ∨¬s(v-1,c-1)∨s(v,c); ¬xᵥ∨¬s(v-1,m)).

Nota: il riferimento usa variabili M per "triangoli mancanti" legate a rette
fissate da riga di comando; qui la generalizzazione con Miss libere su tutti
i segmenti + cardinalità ≤ m è necessaria per il censimento completo (le
mancanze possono stare su qualunque retta, anche più d'una sulla stessa).
È una relaxation corretta: blocchi e conteggi avvengono sulla tabella A.

### Ciclo di enumerazione (come il riferimento, bloccando per classe)

1. Genera il CNF (con m segmenti mancanti ammessi) + le clausole bloccanti
   accumulate; scrivi DIMACS (`p cnf V C` prima delle clausole; i commenti
   `c` sono ammessi) in `census/tmp/`.
2. Esegui kissat (`system()` + `WEXITSTATUS`, o fork/exec: scelta libera);
   output su file; exit 10 → leggi le righe `v` nel vettore dei valori.
3. Estrai la tabella: primo della riga r = l'unico i con A(r,i,0) vero; poi
   segui G(r,·,·) fino a fine riga (come il riferimento).
4. Valida con la ricostruzione greedy (su almeno un elemento dell'orbita);
   se valida, verifica #triangoli == T (abort con errore se diverso); calcola
   la forma canonica e registrala come nuovo tipo. Se non valida, segnalala
   e contala a parte (non è un arrangiamento di pseudorette).
5. In entrambi i casi aggiungi una clausola bloccante per OGNI elemento
   dell'orbita: ⋁_{r,k} ¬A(r, t[r][k]−1, k). Torna al passo 1.
6. Exit 20 → enumerazione completa: i tipi registrati sono il censimento.

Ripartenza (serve per n=11/12): all'avvio, se il file dei tipi esiste già,
rileggilo e ricostruisci i blocchi da lì prima di riprendere il ciclo.

## Struttura da realizzare

In `/home/rufio72/git/kobon/census/` (nomi funzione indicativi, liberi):

| file                | contenuto                                            | ~righe |
|---------------------|------------------------------------------------------|--------|
| `src/costanti.h`    | `N_MAX 12`, tabella massimi `MAXT[13]`, m per n      | 40     |
| `src/tabella.c/.h`  | tipo Tabella, reindex, inverti, orbita BFS, canonica, confronto, triangoli, lettura/scrittura testo | 300 |
| `src/cnf.c/.h`      | numerazione variabili, emissione clausole DIMACS, cardinalità di Sinz, clausole bloccanti | 350 |
| `src/sat.c/.h`      | invocazione kissat, parsing `s`/`v`, valori modello  | 120    |
| `src/wiring.c/.h`   | ricostruzione greedy, estrazione tabella dal modello | 150    |
| `src/bruto.c/.h`    | DFS esaustiva dei wiring per n≤7, dedup per canonica, conteggio classi con T max | 200 |
| `src/main.c`        | CLI, ciclo di enumerazione, scrittura risultati e README k{n}, riepilogo | 250 |
| `Makefile`          | `make` → `census-kobon`; `-std=c11 -O2 -Wall -Wextra`| —      |
| `esegui-tutto.sh`   | ciclo n=3..10: censimento, UNSAT, brute per n≤6, riepilogo finale | —  |

CLI proposta (scelte di dettaglio libere, ma queste funzioni devono esserci):
```
./census-kobon <n> [--out DIR] [--unsat] [--brute] [--kissat PATH] [--tmp DIR]
```
- default: censimento di n, tipi su stdout e, con `--out`, scrittura in DIR;
- `--unsat`: sola dimostrazione del tetto a T+1 (se m'<0 stampa
  "impossibile per conteggio dei segmenti" senza invocare kissat; altrimenti
  genera il CNF con ≤ m' mancanti e pretende exit 20);
- `--brute`: enumerazione esaustiva indipendente (rifiuta n>7) e confronto
  con il risultato SAT;
- `--kissat`: default `kissat` (è nel PATH); `--tmp`: default `census/tmp/`.

Risultati in `/home/rufio72/git/kobon/k{n}/` (creata dal programma o dallo
script): un file `tipo1.txt`, `tipo2.txt`, … per classe, contenente la
tabella canonica (n righe di n-1 interi separati da spazio, commenti `#`
in testa con n, T, metodo), più un `README.md` con: n, massimo di triangoli,
numero di tipi, metodo (modello CNF di Savchuk + kissat + canonicalizzazione,
livello pseudorette, arrangiamenti semplici), esito della verifica UNSAT a
T+1, e la dichiarazione che coordinate razionali e degenerazioni (punti
tripli, parallele) sono un'estensione futura. Se i tipi per un n superano 20,
in alternativa un unico `soluzioni.json` (array di tabelle) più il README.
`census/tmp/` e i binari vanno in un `.gitignore` dentro `census/`.

## Verifiche obbligatorie

1. `cd /home/rufio72/git/kobon/census && make` → nessun warning con
   `-Wall -Wextra`.
2. `find /home/rufio72/git/kobon/census/src -name '*.c' -o -name '*.h' | xargs wc -l | awk '$1>800 && $2!="totale"'`
   → uscita vuota.
3. `./census-kobon 3` → 1 tipo; la tabella canonica è esattamente
   `[[2,3],[1,3],[1,2]]` (misurata). `./census-kobon 5` → 1 tipo, tabella
   `[[2,4,3,5],[1,4,5,3],[4,1,5,2],[3,1,2,5],[1,3,2,4]]` (misurata).
4. `./census-kobon 9` → 1 tipo (misurato con la pipeline di riferimento,
   0,19 s; tempo atteso simile).
5. `./census-kobon <n> --brute` per n=3,4,5,6 → classi 1, 1, 1, 2 e
   coincidenza con il conteggio SAT; statistiche DFS stampate uguali alle
   misure: parole 2, 16, 768, 292864; tabelle distinte 2, 8, 62, 908;
   tabelle con T max 2, 8, 2, 32.
6. `./census-kobon <n> --unsat` per n=3,4,5,7,9 → "impossibile per conteggio
   dei segmenti" (aritmetico, senza kissat); per n=6 e 8 → UNSAT (misurato,
   0,00 s e 0,03 s); per n=10 → UNSAT (attesa, non misurata: il massimo 25
   è dimostrato in letteratura; se risultasse SAT fermarsi e segnalarlo nel
   rapporto, sarebbe un errore del modello).
7. `./esegui-tutto.sh` → cartelle `k3`..`k10` popolate (tipoN.txt + README),
   riepilogo finale `n | T | tipi | UNSAT T+1 | scartate non realizzabili`.
   I conteggi per n=7, 8, 10 non sono noti in anticipo: sono il risultato
   del censimento; vanno riportati nel rapporto. Tabelle SAT scartate dalla
   validazione: attese 0 per n≤6 (la brute dà gli stessi conteggi).
8. `git -C /home/rufio72/git/kobon status --short` → solo `census/` e le
   nuove `k3/`..`k10/`; nulla in `k14/ k16/ k18/`.

## Consegna

- Codice, Makefile, script e risultati come sopra. Commit nel repo
  `/home/rufio72/git/kobon` con messaggio in italiano che descrive il
  censimento e riga finale `Co-Authored-By: Claude <modello usato>
  <noreply@anthropic.com>`. Niente push.
- Rapporto finale: massimo trenta righe, con la tabella dei conteggi per
  n=3..10, l'uscita delle verifiche e le deviazioni dal brief.

## Domande aperte

1. (preferenza) Blocco a livello di classe canonica (una clausola per ogni
   elemento dell'orbita, come il riferimento) anziché a modello grezzo:
   proposta già adottata sopra, è ciò che rende 1 chiamata SAT ≈ 1 classe.
2. (preferenza) Formato risultati: `tipoN.txt` per classe + README per
   cartella, JSON unico solo oltre 20 tipi. Proposta adottata sopra.
3. (preferenza) Cross-check esaustivo obbligatorio per n≤6; per n=7
   (~1,1·10⁹ foglie DFS) facoltativo dietro `--brute`, da eseguire solo se
   resta sotto qualche minuto in C, altrimenti dichiararlo saltato.
4. (scelta di progetto) Lingua dei README nelle cartelle `k3`..`k10`:
   le regole del progetto dicono documenti in italiano, ma i README esistenti
   di k14/k16/k18 sono in inglese (repo con vocazione pubblica). Proposta:
   inglese per i README dei risultati k{n}, italiano per tutto il resto
   (codice, commenti, README di census).

## Decisioni della chat principale (2026-10-23)

1-3: preferenze accettate come da proposta (blocco a livello di classe canonica; formato tipoN.txt + README per cartella; brute n=7 facoltativa). 4 (scelta di progetto, lingua dei README in k3..k10): inglese, come i README pubblicati di k14-k18 — coerenza del repo pubblico; l'utente può chiedere di cambiarla.

## Esito (2026-10-23)

Ciclo passato. Sviluppatore: censimento completo n=3..10, tutte le verifiche del brief passate, più n=7 in forza bruta (facoltativo). Risultato: tipi semplici al massimo noto = 1,1,1,2,1,0,1,36 per n=3..10. Lo 0 di n=8 è reale e triplamente confermato (SAT su tutti i multiset -L; forza bruta esaustiva su 1.232.944 arrangiamenti semplici di 8 pseudorette: massimo semplice 14 con 17 classi; coerenza con BBL Teorema 1.4, tetto non raggiunto per 8). Verificatore indipendente: tutte le riesecuzioni coincidono; il suo dubbio su n=8 («probabile errore del modello») nasce dal fatto che il brief dava 15 come massimo senza distinguere semplice/degenere — chiarito dall'analisi dello sviluppatore. Il 15 a n=8 richiede degenerazioni: prima prova nel progetto che le degenerazioni battono strettamente i semplici. Token: analista (Fable) 97k, sviluppatore (Opus) 111k, verificatore (Sonnet) 26k.
