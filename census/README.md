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
