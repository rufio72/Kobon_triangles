# Appunto: il budget dei segmenti — tetti degeneri senza forza bruta

Stato: **appunto da controverificare**, non ancora un brief. Scritto il
2026-10-23 dalla chat principale, da idea emersa col censimento 3–10.
Prossimi passi: (1) l'analista controverifica le affermazioni A1–A4 e i
conti della tabella; (2) se reggono, si scrive il brief di esecuzione per
i bersagli B1–B4, in ordine.

## L'idea

Un arrangiamento di n rette ha al più n(n−2) segmenti finiti; T triangoli
non sovrapposti ne consumano 3T (nessun segmento borda due triangoli).
Chiamiamo **budget** la differenza n(n−2) − 3T. Ogni degenerazione ha un
prezzo fisso in segmenti distrutti, quindi il budget limita severamente
*quali* schemi di degenerazione sono compatibili con un dato T — e il SAT
va lanciato solo sugli schemi sopravvissuti, non sul modello degenere
generale.

## Affermazioni da controverificare (A1–A4)

- **A1 — prezzo di un punto multiplo.** In un punto di molteplicità m
  (m ≥ 3 rette concorrenti) ogni retta coinvolta perde m−2 punti di
  incrocio, quindi m−2 segmenti; totale **m(m−2)** segmenti distrutti
  (punto triplo: 3; quadruplo: 8). Verificare il conto e che punti
  multipli distinti non condividano rette senza correzioni.
- **A2 — prezzo di una classe di parallele.** Una classe di p rette
  parallele toglie a ciascuna p−1 incroci, totale **p(p−1)** segmenti
  (coppia: 2; terna: 6). Verificare interazione con A1 (una retta sia in
  una classe parallela sia in un punto triplo: i prezzi si sommano?).
- **A3 — il confronto è col massimo semplice.** n(n−2) segmenti si hanno
  solo nell'arrangiamento semplice; un arrangiamento con insieme di
  degenerazioni D ne ha n(n−2) − prezzo(D), e serve
  prezzo(D) ≤ n(n−2) − 3T. Verificare che non ci siano altri effetti
  (segmenti illimitati, triangoli che toccano i punti multipli — la
  convenzione "tocca conta" non cambia il conteggio dei segmenti?).
- **A4 — caso limite budget 0.** Se 3T = n(n−2) l'arrangiamento è
  necessariamente semplice e perfetto: nessuna degenerazione ammessa.
  (Già osservato empiricamente nel censimento: 0 tipi degeneri a
  n = 3, 5, 9.)

## Conseguenze, se A1–A4 reggono (tabella da ricontrollare)

| bersaglio | T | budget | schemi degeneri ammessi |
|---|---|---|---|
| n=10, tetto 26 | 26 | 80−78 = 2 | **solo** 1 coppia di parallele, resto perfetto |
| n=11, tetto 33 | 33 | 99−99 = 0 | nessuno: 33 degenere impossibile per aritmetica; il semplice a 33 è già UNSAT (Savchuk, arXiv:2507.07951) → **tetto degenere a 33 gratis** |
| n=12, il 39 | 39 | 120−117 = 3 | 1 punto triplo, oppure 1 coppia di parallele (budget residuo 1) |
| n=14, il 54 | 54 | 168−162 = 6 | combinazioni di prezzo ≤ 6: fino a 3 coppie parallele, fino a 2 punti tripli, misti 3+2; punti di molteplicità ≥ 4 esclusi (prezzo 8) |
| n=14, il 55 | 55 | 168−165 = 3 | come n=12: 1 triplo o 1 coppia |
| n=18, il 94 | 94 | 288−282 = 6 | come n=14 a 54: solo tripli e coppie, ≤ 2 tripli / ≤ 3 coppie / misto 3+2; **molteplicità ≥ 4 impossibile in un 94** |

## Bersagli di esecuzione, in ordine di costo (B1–B4)

- **B1 — n=10, tetto 26 ripartito.** Riprendere il tetto degenere a 26
  non col modello generale (il run monolitico del 2026-10-23 è stato
  interrotto senza verdetto) ma col solo schema "1 coppia di parallele":
  istanza molto più piccola. Chiude il censimento n=10 in modo completo.
- **B2 — n=12, esiste 39?** Due SAT mirati (triplo / coppia). Qualunque
  esito è pubblicabile: o record nuovo o N(12)=38.
- **B3 — n=14, esiste 54 (poi eventualmente 55)?** Una manciata di SAT
  mirati; esito negativo ⇒ N(14)=53 (il massimo dell'utente diventa
  dimostrato); positivo ⇒ record nuovo.
- **B4 — n=18, il 94 degenere per schemi.** Stessa lista corta di schemi
  di B3 ma a 18 rette: istanze dure, da affrontare con cube-and-conquer
  e/o macchina a nolo, dopo che B1–B3 hanno collaudato il metodo.

## Vincoli di contorno

- Tutto in C; kissat in ~/.local/bin; risultati nelle cartelle k{n} del
  repo kobon (append-only su ciò che è pubblicato).
- I tetti "gratis" (n=11 a 33) vanno comunque scritti nel README con il
  lemma esplicito e la citazione, non lasciati impliciti.
- Mai azioni su OEIS; eventuale uso pubblico dei risultati è dell'utente.
