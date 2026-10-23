# census — censimento dei tipi ottimali di Kobon, n = 3..10

Programma in C che, per ogni n, enumera tutti gli arrangiamenti **semplici**
di n **pseudorette** con il massimo noto di triangoli (OEIS A006066) e ne
conta le classi di isomorfismo (rinumerazione, inversione del verso,
riflessione). Risultati in `../k3` … `../k10`.

## Uso

```
make
./census-kobon <n> [--out DIR] [--unsat] [--brute] [--kissat PATH] [--tmp DIR]
./esegui-tutto.sh        # n = 3..10, risultati in ../k<n> e riepilogo
```

- senza opzioni: censimento, tipi su stdout;
- `--out DIR`: anche verifica del tetto a T+1, tipi e README in DIR;
- `--unsat`: solo la verifica del tetto a T+1;
- `--brute`: DFS esaustiva sui wiring diagram (n ≤ 7) e confronto con il SAT.

Serve kissat nel PATH (o `--kissat`). I file di lavoro vanno in `tmp/`; un
censimento interrotto riparte da `tmp/stato-n<n>.txt`, che viene cancellato
a enumerazione completa.

## Moduli

- `src/tabella.c`: tabella degli ordini, gruppo di isomorfismo, forma
  canonica, triangoli;
- `src/cnf.c`: modello CNF di Savchuk (arXiv:2507.07951) riscritto in C, con
  segmenti mancanti liberi e cardinalità di Sinz;
- `src/sat.c`: invocazione di kissat;
- `src/wiring.c`: estrazione della tabella dal modello, validazione greedy;
- `src/bruto.c`: enumerazione esaustiva indipendente;
- `src/risultati.c`: file `tipoN.txt` / `soluzioni.json` e README (in inglese);
- `src/main.c`: riga di comando e ciclo di enumerazione.

## Arrangiamenti degeneri (compito 02): `census-degeneri`

Estende il censimento ai punti multipli (tre o più rette per un punto) e
alle classi di rette parallele (parallelismo transitivo, come per le rette
vere), con la convenzione di conteggio dei record pubblicati: una terna è un
triangolo se le tre coppie si incrociano in tre punti distinti e su ogni
retta gli eventi delle altre due sono consecutivi. Serve GMP (`-lgmp`).

```
./census-degeneri <n> [--out DIR] [--unsat] [--brute] [--con-semplici]
                      [--limite SECONDI] [--parti K] [--parziale]
                      [--kissat PATH] [--tmp DIR]
./census-degeneri --autotest | --controlla FILE | --riscontro JSON | --canone FILE
./esegui-degeneri.sh     # n = 8, 3..7, 9, 10: risultati in ../k<n> e riepilogo
```

- senza opzioni: tipi degeneri (almeno una degenerazione) a T = massimo noto;
- `--out DIR`: anche tetto a T+1 (in un processo parallelo al censimento),
  file `degenereN.txt` e sezione in coda al README di DIR (sostituita, mai
  duplicata, a ogni esecuzione);
- `--unsat`: solo il tetto (almeno T+1 triangoli), `--limite` in secondi
  per kissat (scaduto = tetto aperto);
- `--parti K`: censimento e tetto in K processi paralleli; la parte p < K-1
  ha esattamente p terne per un punto comune (variabili T vere), l'ultima
  almeno K-1. È un invariante del tipo, quindi le parti sono disgiunte; i
  risultati si riuniscono e un tipo trovato in due parti è un errore.
  Verificato identico al calcolo diretto su n = 6 e 8, ma misurato più lento
  su n = 9 (ogni parte con cardinalità esatta è più dura per kissat): non è
  usato da `esegui-degeneri.sh` salvo `PARTI=K`;
- `--parziale` (con `--out`): niente SAT, solo i tipi già nel file di stato
  di un censimento interrotto, che resta per la ripresa; risultati e README
  marcati come parziali, tetto dichiarato aperto;
- `--brute`: DFS esaustiva dei wiring degeneri (n ≤ 8) e confronto con il SAT
  (con `--con-semplici` anche il censimento SAT senza il vincolo di
  degenerazione);
- `--controlla FILE`: verifica che la tabella del file soddisfi ogni clausola
  del modello (difesa contro un modello troppo forte);
- `--riscontro JSON`: triangoli, punti multipli e canone-eventi da rette
  razionali esatte (formato k18, campo `lines_frac`);
- `--canone FILE`: tabella canonica di una tabella;
- `--autotest`: riscontro con la soluzione pubblicata n=8
  (`dati/k8-pubblicata.json`), `--controlla` su questa e sulle canoniche
  semplici, orbite semplici uguali a quelle di `census-kobon`, k18 sol1.

Formato delle tabelle (1-based): una riga per retta, gli eventi in ordine,
le rette di un punto multiplo fra parentesi, le parallele assenti; per
esempio `2 8 4 6 (3 7) 5`. Una tabella su una riga sola separa le righe con
`;`.

Moduli:

- `src/dtab.c`: tabella degli eventi (TabellaD), confronto, lettura e
  scrittura, conversione a `events_t`;
- `src/dorbita.c`: wiring generalizzato, presentazioni (2n estremi di
  partenza × 2 versi), tabella canonica;
- `src/dcnf.c`: modello CNF degenere (clausole D1-D12), `--controlla`,
  invocazione di kissat con tetto di tempo;
- `src/dbruto.c`: forza bruta indipendente;
- `src/dauto.c`: autotest e riscontro geometrico;
- `src/drisultati.c`: `degenereN.txt` e sezione dei README;
- `src/dmain.c`: riga di comando e ciclo di enumerazione;
- `src/{geom,canon,json_min,sha256}.{c,h}`: copiati senza modifiche da
  `k18/esperimenti/isomorfismo` (commenti in inglese).

`dati/k8-pubblicata.json`: copia dei soli dati numerici del certificato
ud1/kobon-solutions della soluzione pubblicata a 15 triangoli
(`source_url` nel file).
