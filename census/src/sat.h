/* Invocazione di kissat come processo esterno. */
#ifndef SAT_H
#define SAT_H

/* Risolve il CNF in cnf_path con il binario kissat, scrivendo l'uscita in
 * out_path. Restituisce 10 (SAT, val[1..nvar] = +1/-1), 20 (UNSAT) oppure
 * -1 in caso di errore. val deve avere nvar+1 elementi. */
int sat_risolvi(const char *kissat, const char *cnf_path, const char *out_path,
                int nvar, signed char *val);

#endif
